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
    rbuf->buffer = kcalloc(count, size);
    rbuf->count = count;
    rbuf->object_size = size;
    *buf_out = rbuf;
    return KERRNO_SUCCESSES;
}

status_t ringbuf_read_one(ringbuf_t *ring_buffer, void *buf, size_t *size_out)
{
    off_t read_offset = ring_buffer->read_ptr * ring_buffer->object_size;
    memcpy(buf, &ring_buffer->buffer[read_offset], ring_buffer->object_size);
    if (size_out)
    {
        *size_out = ring_buffer->object_size;
    }
    return KERRNO_SUCCESSES;
}

status_t ringbuf_read(ringbuf_t *ring_buffer, void *buf, size_t *size_out)
{
    if (ring_buffer == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer is null");
    }

    if (ring_buffer->read_ptr == ring_buffer->count)
    {
        ring_buffer->read_ptr = 0;
    }

    // if (ring_buffer->write_ptr == (ring_buffer->read_ptr + 1))
    // {
    //     KERRNO_RETURN(KERRNO_BUFFER_OVERFLOW, "buffer full, drop");
    // }

    // trace_debug(MODULE, "ring_buffer->write_ptr = %u", ring_buffer->write_ptr);
    // trace_debug(MODULE, "ring_buffer->read_ptr = %u", ring_buffer->read_ptr);
    if (ring_buffer->read_ptr == ring_buffer->write_ptr)
    {
        if (size_out)
        {
            *size_out = 0;
        }
        KERRNO_RETURN(ENOBUFS, "buffer is, empty");
    }

    ringbuf_read_one(ring_buffer, buf, size_out);
    ring_buffer->read_ptr++;
    return KERRNO_SUCCESSES;
}

status_t ringbuf_write(ringbuf_t *ring_buffer, void *buf)
{
    // ENTER_FUNC("%p, %p", ring_buffer, buf);
    if (ring_buffer == NULL || buf == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer or buf is null");
    }

    if (ring_buffer->write_ptr == ring_buffer->count)
    {
        ring_buffer->write_ptr = 0;
    }

    if (ring_buffer->read_ptr == (ring_buffer->write_ptr + 1))
    {
        KERRNO_RETURN(ENOBUFS, "buffer full, drop");
    }

    off_t write_offset = ring_buffer->write_ptr * ring_buffer->object_size;
    ring_buffer->write_ptr++;
    memcpy(&ring_buffer->buffer[write_offset], buf, ring_buffer->object_size);
    return KERRNO_SUCCESSES;
}

status_t ringbuf_erase(ringbuf_t *ring_buffer)
{
    // ENTER_FUNC(MODULE, "%p", ring_buffer,);
    if (ring_buffer == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer is null");
    }

    if (ring_buffer->read_ptr == ring_buffer->write_ptr)
    {
        KERRNO_RETURN(ENOBUFS, "buffer full, drop");
    }
    ring_buffer->write_ptr--;

    if (ring_buffer->write_ptr < ring_buffer->count)
    {
        ring_buffer->write_ptr = ring_buffer->count - 1;
    }
    return KERRNO_SUCCESSES;
}

status_t ringbuf_read_buffer(ringbuf_t *ring_buffer, void *buf, size_t *size_out)
{
    if (ring_buffer == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "ring_buffer is null");
    }

    return ringbuf_read_one(ring_buffer, buf, size_out);
}
