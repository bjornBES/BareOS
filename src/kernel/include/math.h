/*
 * File: math.h
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define abs(a)        (((a) < 0) ? -(a) : (a))
#define max(a, b)     (((a) > (b)) ? (a) : (b))
#define min(a, b)     (((a) < (b)) ? (a) : (b))
#define min3(a, b, c) min(a, min(b, c))
#define sign(x)       ((x < 0) ? -1 : ((x > 0) ? 1 : 0))

#define swap(a, b)             \
    {                          \
        typeof(a) __tmp = (a); \
        (a) = (b);             \
        (b) = __tmp;           \
    }

uint32_64_t ceil_log2(uint32_64_t x);
size_t floor_log2(size_t x);
size_t trailing_zero_count(size_t x);
