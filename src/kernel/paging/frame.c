/*
 * File: fream.c
 * File Created: 18 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "frame.h"
#include "paging.h"
#include "kernel.h"
#include "debug/debug.h"

#include <memory.h>
#include <util/binary.h>

#define FRAME_BITMAP_SIZE 1024 // in future (1024 * 1024) / 32 // each bit is a 4KB frame

static uint32_t frame_bitmap[FRAME_BITMAP_SIZE];

inline int frame_get_frame(uint32_t frame_index)
{
    return BIT_GET(frame_bitmap[frame_index / 32], frame_index % 32);
}

inline void frame_mark_frame_used(uint32_t frame_index)
{
    BIT_SET(frame_bitmap[frame_index / 32], (frame_index % 32));
}

inline void frame_mark_frame_free(uint32_t frame_index)
{
    BIT_UNSET(frame_bitmap[frame_index / 32], (frame_index % 32));
}

void frame_alloc_region(uint32_t start_phys, uint32_t end_phys)
{
    uint32_t start_index = (start_phys / PAGE_SIZE) / 32;
    uint32_t end_index = ((end_phys) / PAGE_SIZE) / 32 + 1;
    log_info("FRAME", "start_index = %u, end_index = %u", start_index, end_index);
    for (int i = start_index; i < end_index; i++)
    {
        if (frame_bitmap[i] == 0xFFFFFFFF)
            continue; // all 32 frames in this word are used

        // find first free bit
        for (int bit = 0; bit < 32; bit++)
        {
            int frame_index = i * 32 + bit;
            if (!frame_get_frame(frame_index))
            {
                frame_mark_frame_used(frame_index);
            }
        }
    }
}

void frame_alloc_at(uint32_t phys_addr)
{
    uint32_t frame_index = phys_addr / PAGE_SIZE;
    if (frame_get_frame(frame_index))
    {
        log_warn("FRAME", "frame %u (0x%08X) already marked used", frame_index, phys_addr);
        return;
    }
    frame_mark_frame_used(frame_index);
}

uint32_t frame_alloc_frame()
{
    for (int i = 0; i < FRAME_BITMAP_SIZE; i++)
    {
        if (frame_bitmap[i] == 0xFFFFFFFF)
            continue; // all 32 frames in this word are used

        // find first free bit
        for (int bit = 0; bit < 32; bit++)
        {
            int frame_index = i * 32 + bit;
            if (!frame_get_frame(frame_index))
            {
                frame_mark_frame_used(frame_index);
                return (frame_index)*PAGE_SIZE; // return PHYSICAL address
            }
        }
    }
    panic("FRAME", __FILE__, __LINE__, "out of physical frames");
    return 0;
}

void frame_free_frame(uint32_t phys_addr)
{
    uint32_t frame_index = phys_addr / PAGE_SIZE;
    frame_mark_frame_free(frame_index);
}

void frame_dump_bitmap()
{
    uint32_t free_count = 0;
    uint32_t used_count = 0;
    uint32_t total_frames = FRAME_BITMAP_SIZE * 32;

    for (uint32_t i = 0; i < total_frames; i++)
    {
        if (frame_get_frame(i))
        {
            used_count++;
            log_debug("FRAME", "frame %u at 0x%08X: Taken", i, i * PAGE_SIZE);
        }
        else
        {
            free_count++;
        }
    }

    log_debug("FRAME", "Number of free frames:      %u (%u KB)", free_count, free_count * 4);
    log_debug("FRAME", "Number of allocated frames: %u (%u KB)", used_count, used_count * 4);
}

void frame_init()
{
    memset32(frame_bitmap, 0, sizeof(frame_bitmap) / 32);
}