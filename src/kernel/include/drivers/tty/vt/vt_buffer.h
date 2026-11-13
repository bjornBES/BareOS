/*
 * File: vt_buffer.h
 * File Created: 13 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/tty/vt/vtt.h"

#include "memory.h"

#include <types.h>

#define scr_writew(val, addr) (*(addr) = (val))
#define scr_readw(addr) (*(addr))

static inline void scr_memsetw(vt_cell_t *s, vt_cell_t c, size_t count)
{
    // void *p_c = &c;
    memset64(s, *((long long*)&c), count);
    // for (size_t i = 0; i < count; i++)
    // {
    //     s[i] = c;
    // }
    // memset(s, c, count / 2);
}

static inline void scr_memmovew(vt_cell_t *d, const vt_cell_t *s, size_t count)
{
	memmove(d, s, count * sizeof(vt_cell_t));
}