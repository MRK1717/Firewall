/* SPDX-License-Identifier: BSD-3-Clause */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"
#include "producer.h"

void publish_data(so_ring_buffer_t *rb, const char *filename)
{
    char buffer[PKT_SZ];
    ssize_t sz;
    int fd;
    size_t seq_no = 0;
    packet_wrapper_t wrapper;

    fd = open(filename, O_RDONLY);
    DIE(fd < 0, "open");

    while ((sz = read(fd, buffer, PKT_SZ)) != 0) {
        DIE(sz != PKT_SZ, "packet truncated");

        wrapper.seq_no = seq_no++;
        memcpy(&wrapper.pkt, buffer, PKT_SZ);

        /* enqueue packet into ring buffer */
        ring_buffer_enqueue(rb, &wrapper, sizeof(wrapper));
    }

    close(fd);
    ring_buffer_stop(rb);
}
