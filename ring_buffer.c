/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdlib.h>
#include <string.h>
#include "ring_buffer.h"
#include "utils.h"

//Am folosit pthread pentru a controla accesul la ring buffer
int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
    int rc;

    ring->data = malloc(sizeof(packet_wrapper_t) * cap);
    if (ring->data == NULL)
        return -1;

    ring->read_pos = 0;
    ring->write_pos = 0;
    ring->len = 0;
    ring->cap = cap;
    ring->is_open = true;

    rc = pthread_mutex_init(&ring->mutex, NULL);
    if (rc != 0) {
        free(ring->data);
        return -1;
    }

    rc = pthread_cond_init(&ring->not_empty, NULL);
    if (rc != 0) {
        pthread_mutex_destroy(&ring->mutex);
        free(ring->data);
        return -1;
    }

    rc = pthread_cond_init(&ring->not_full, NULL);
    if (rc != 0) {
        pthread_cond_destroy(&ring->not_empty);
        pthread_mutex_destroy(&ring->mutex);
        free(ring->data);
        return -1;
    }

    return 0;
}

//Am utlilizat conditii in cazul in care bufferul este plin si trebuie sa blocam adaugarea
ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
    if (size != sizeof(packet_wrapper_t)) {
        fprintf(stderr, "Invalid size for enqueue\n");
        return -1;
    }

    packet_wrapper_t *wrapper = (packet_wrapper_t *)data;

    pthread_mutex_lock(&ring->mutex);

    while (ring->len == ring->cap) {
        pthread_cond_wait(&ring->not_full, &ring->mutex);
    }

    ring->data[ring->write_pos] = *wrapper;
    ring->write_pos = (ring->write_pos + 1) % ring->cap;
    ring->len++;

    pthread_cond_signal(&ring->not_empty);
    pthread_mutex_unlock(&ring->mutex);

    return 1;
}

//Am folosit conditii pentru a bloca extragerea din buffer cand acesta este gol
ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
    if (size != sizeof(packet_wrapper_t)) {
        fprintf(stderr, "Invalid size for dequeue\n");
        return -1;
    }

    packet_wrapper_t *wrapper = (packet_wrapper_t *)data;

    pthread_mutex_lock(&ring->mutex);

    while (ring->len == 0) {
        if (!ring->is_open) {
            pthread_mutex_unlock(&ring->mutex);
            return 0;
        }
        pthread_cond_wait(&ring->not_empty, &ring->mutex);
    }

    *wrapper = ring->data[ring->read_pos];
    ring->read_pos = (ring->read_pos + 1) % ring->cap;
    ring->len--;

    pthread_cond_signal(&ring->not_full);
    pthread_mutex_unlock(&ring->mutex);

    return 1;
}

//Am oprit accesul la buffer si am eliberat memoria
void ring_buffer_destroy(so_ring_buffer_t *ring)
{
    pthread_mutex_destroy(&ring->mutex);
    pthread_cond_destroy(&ring->not_empty);
    pthread_cond_destroy(&ring->not_full);
    free(ring->data);
}

//Am oprit bufferul si am anuntat sfarsitul datelor
void ring_buffer_stop(so_ring_buffer_t *ring)
{
    pthread_mutex_lock(&ring->mutex);
    ring->is_open = false;
    pthread_cond_broadcast(&ring->not_empty);
    pthread_mutex_unlock(&ring->mutex);
}
