/*
 * File: tsc.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <defs.h>

ALWAYS_INLINE uint64_t rdtsc()
{
    uint32_t low;
    uint32_t high;
    inline_asm("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}