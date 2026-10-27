/*
 * File: ringbuft.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct ringbuf
{
    void **buffer;
    size_t object_size;

    uint32_t read_ptr;
    uint32_t write_ptr;
} ringbuf_t;