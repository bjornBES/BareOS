/*
 * File: ringbuf.c
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ringbuf/ringbuf.h"

#include "kernel/memory.h"

#define MODULE "ringbuf"

typedef struct ringbuf
{
    void **buffer;
    size_t object_size;

    uint32_t read_ptr;
    uint32_t write_ptr;
} ringbuf_t;

size_t buffer_count;
size_t buffer_max_count;
ringbuf_t **buffers = NULL;

ringbuf_index ringbuf_init(size_t size, uint32_t count)
{
    if (buffers == NULL)
    {
        buffers = calloc(8, sizeof(ringbuf_t));
        buffer_max_count = 8;
    }
    ringbuf_t *rbuf = malloc(sizeof(ringbuf_t));
    memset(rbuf, 0, sizeof(ringbuf_t));
    rbuf->buffer = calloc(count, size);
    rbuf->object_size = size;
    buffers[buffer_count] = rbuf;
    ringbuf_index ret = buffer_count;
    buffer_count++;
    return ret;
}

size_t ringbuf_read(ringbuf_index buf_index, void *buf, int count)
{
    // ENTER_FUNC(MODULE, "%u, %p, %u", buf_index, buf, count);
    if (buf_index >= buffer_count || buffers == NULL)
    {
        return 0;
    }

    ringbuf_t *ring_buffer = buffers[buf_index];
    if (buf == NULL)
    {
        ring_buffer->read_ptr--;
        return ring_buffer->object_size;
    }

    if (ring_buffer->read_ptr == ring_buffer->write_ptr)
    {
        return 0;
    }

    off_t read_offset = ring_buffer->read_ptr * ring_buffer->object_size;
    ring_buffer->read_ptr++;
    memcpy(buf, &ring_buffer->buffer[read_offset], ring_buffer->object_size);
    return ring_buffer->object_size;
}

void ringbuf_write(ringbuf_index buf_index, void *buf)
{
    // ENTER_FUNC(MODULE, "%u, %p", buf_index, buf);
    if (buf_index >= buffer_count || buffers == NULL)
    {
        return;
    }
    if (buf == NULL)
    {
        return;
    }
    ringbuf_t *ring_buffer = buffers[buf_index];
    
    off_t write_offset = ring_buffer->write_ptr * ring_buffer->object_size;
    // log_debug(MODULE, "ring_buffer->buffer = %p", ring_buffer->buffer);
    // log_debug(MODULE, "ring_buffer->buffer[%u] = %p", write_offset, &ring_buffer->buffer[write_offset]);
    ring_buffer->write_ptr++;
    memcpy(&ring_buffer->buffer[write_offset], buf, ring_buffer->object_size);
}
