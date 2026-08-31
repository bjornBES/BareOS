/*
 * File: memory.c
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "memory.h"
#include "debug/debug.h"

void *mem_segoffset_to_linear(uint16_t seg, uint16_t off)
{
    return (void *)(seg * 16 + off);
}

seg_off_t mem_linear_to_segoff(void *linear)
{
    return (seg_off_t) {
        .segment = (uint16_t)((uint32_t)linear >> 4),
        .offset = (uint16_t)((uint32_t)linear & 0x0F),
    };
}
