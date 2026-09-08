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

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))

uint32_64 ceil_log2(uint32_64 x);
size_t floor_log2(size_t x);
size_t trailing_zero_count(size_t x);