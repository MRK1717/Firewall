/* SPDX-License-Identifier: BSD-3-Clause */

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"

//am folosit ring_buffer_dequeue pentru a preia pachete,process_packet pentru procesarea datelor si mutex pentru restrictonare 
void* consumer_thread(void *arg)
{
    so_consumer_ctx_t *ctx = (so_consumer_ctx_t*)arg;
    packet_wrapper_t wrapper;
    ssize_t sz;

    while (1) {
        sz = ring_buffer_dequeue(ctx->producer_rb, &wrapper, sizeof(wrapper));
        if (sz == 0) {
            break;
        }

        struct so_packet_t *pkt = &wrapper.pkt;
        int action = process_packet(pkt);
        unsigned long hash = packet_hash(pkt);
        unsigned long packet_timestamp = pkt->hdr.timestamp;
        size_t seq_no = wrapper.seq_no;

        pthread_mutex_lock(ctx->log_mutex);
        while (seq_no != *ctx->next_seq_no) {
            pthread_cond_wait(ctx->log_cond, ctx->log_mutex);
        }

        char out_buf[256];
        int len = snprintf(out_buf, sizeof(out_buf), "%s %016lx %lu\n",
                           RES_TO_STR(action), hash, packet_timestamp);
        write(ctx->out_fd, out_buf, len);

        (*ctx->next_seq_no)++;

        pthread_cond_broadcast(ctx->log_cond);
        pthread_mutex_unlock(ctx->log_mutex);
    }

    free(ctx);
    return NULL;
}

//Am folosit log_cond pentru a asigura ordinea screrii
int create_consumers(pthread_t *tids,
                     int num_consumers,
                     so_ring_buffer_t *rb,
                     const char *out_filename)
{
    int rc;
    pthread_mutex_t *log_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *log_cond = malloc(sizeof(pthread_cond_t));
    size_t *next_seq_no = malloc(sizeof(size_t));
    *next_seq_no = 0;

    if (!log_mutex || !log_cond || !next_seq_no) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(log_mutex, NULL);
    pthread_cond_init(log_cond, NULL);

    int out_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    DIE(out_fd < 0, "open");

    for (int i = 0; i < num_consumers; i++) {
        so_consumer_ctx_t *ctx = malloc(sizeof(so_consumer_ctx_t));
        if (!ctx) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        ctx->producer_rb = rb;
        ctx->log_mutex = log_mutex;
        ctx->log_cond = log_cond;
        ctx->next_seq_no = next_seq_no;
        ctx->out_fd = out_fd;

        rc = pthread_create(&tids[i], NULL, consumer_thread, ctx);
        DIE(rc != 0, "pthread_create");
    }

    return num_consumers;
}
