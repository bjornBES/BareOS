/*
 * File: math.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define min3(a, b, c) min(a, min(b, c))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))

#define swap(a, b)             \
    {                          \
        typeof(a) __tmp = (a); \
        (a) = (b);             \
        (b) = __tmp;           \
    }
