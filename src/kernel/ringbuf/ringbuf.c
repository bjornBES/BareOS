/*
 * File: ringbuf.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ringbuf/ringbuf.h"

#include "memory.h"

#define MODULE "ringbuf"

status_t ringbuf_init(size_t size, uint32_t count, ringbuf_t **buf_out)
{
    ringbuf_t *rbuf = kzmalloc(sizeof(ringbuf_t));
    rbuf->buffer = calloc(count, size);
    rbuf->object_size = size;
    *buf_out = rbuf;
    return KERRNO_SUCCESSES;
}

status_t ringbuf_read(ringbuf_t *ring_buffer, void *buf, size_t *size_out)
{
    if (ring_buffer == NULL || buf == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer or buf is null");
    }

    if (ring_buffer->read_ptr == ring_buffer->write_ptr)
    {
        *size_out = 0;
        return KERRNO_SUCCESSES;
    }

    if (buf == NULL)
    {
        ring_buffer->read_ptr--;
        *size_out = ring_buffer->object_size;
        return KERRNO_SUCCESSES;
    }

    off_t read_offset = ring_buffer->read_ptr * ring_buffer->object_size;
    ring_buffer->read_ptr++;
    memcpy(buf, &ring_buffer->buffer[read_offset], ring_buffer->object_size);
    *size_out = ring_buffer->object_size;
    return KERRNO_SUCCESSES;
}

status_t ringbuf_write(ringbuf_t *ring_buffer, void *buf)
{
    // ENTER_FUNC(MODULE, "%u, %p", buf_index, buf);
    if (ring_buffer == NULL || buf == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer or buf is null");
    }

    off_t write_offset = ring_buffer->write_ptr * ring_buffer->object_size;
    ring_buffer->write_ptr++;
    memcpy(&ring_buffer->buffer[write_offset], buf, ring_buffer->object_size);
}
