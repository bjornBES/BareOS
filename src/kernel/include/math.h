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

#include <uapi_math.h>
#include <types.h>


uint32_64_t ceil_log2(uint32_64_t x);
size_t floor_log2(size_t x);
size_t trailing_zero_count(size_t x);
