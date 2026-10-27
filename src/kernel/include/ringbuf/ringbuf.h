/*
 * File: ringbuf.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ringbuft.h"

#include <types.h>

status_t ringbuf_init(size_t size, uint32_t count, ringbuf_t **buf_out);
status_t ringbuf_read(ringbuf_t *ring_buf, void *buf, size_t *size_out);
status_t ringbuf_write(ringbuf_t *ring_buf, void *buf);