// SPDX-License-Identifier: BSD-3-Clause

#ifndef __SO_CONSUMER_H__
#define __SO_CONSUMER_H__

#include "ring_buffer.h"
#include "packet.h"
#include <pthread.h>

typedef struct so_consumer_ctx_t {
    struct so_ring_buffer_t *producer_rb;

    pthread_mutex_t *log_mutex;
    pthread_cond_t *log_cond;
    size_t *next_seq_no;
    int out_fd;
} so_consumer_ctx_t;

int create_consumers(pthread_t *tids,
                     int num_consumers,
                     so_ring_buffer_t *rb,
                     const char *out_filename);

void *consumer_thread(void *arg);

#endif /* __SO_CONSUMER_H__ */
