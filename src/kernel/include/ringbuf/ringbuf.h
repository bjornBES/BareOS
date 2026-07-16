/*
 * File: ringbuf.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <types.h>

typedef uint32_t ringbuf_index;

ringbuf_index ringbuf_init(size_t size, uint32_t count);
size_t ringbuf_read(ringbuf_index buf_index, void *buf, int count);
void ringbuf_write(ringbuf_index buf_index, void *buf);
