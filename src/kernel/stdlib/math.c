/*
 * File: math.c
 * File Created: 02 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "math.h"

uint32_64 ceil_log2(uint32_64 x)
{
    static const uint32_64 t[6] = {0xFFFFFFFF00000000ull, 0x00000000FFFF0000ull,
                                   0x000000000000FF00ull, 0x00000000000000F0ull,
                                   0x000000000000000Cull, 0x0000000000000002ull};

    uint32_64 y = (((x & (x - 1)) == 0) ? 0 : 1);
    uint32_64 j = 32;
    uint32_64 i;

    for (i = 0; i < 6; i++)
    {
        uint32_64 k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}

// ty https://stackoverflow.com/questions/3272424/compute-fast-log-base-2-ceiling

size_t floor_log2(size_t x)
{
    // x must be > 0
    return (sizeof(size_t) * 8 - 1) - __builtin_clzl(x);
}

size_t trailing_zero_count(size_t x)
{
    if (x == 0)
    {
        return SIZE_MAX; // fully aligned — let max_fit_order be the real cap
    }
    return __builtin_ctzl(x);
}
