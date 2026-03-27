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
#include "kernel.h"
#include "debug/debug.h"

#include "libs/memory.h"
#include <util/binary.h>

extern char __frame_bitmap;
extern char __frame_bitmap_phys;
extern char __frame_bitmap_phys_end;

uint32_t *frame_bitmap;

inline int frame_get_frame(frame frame_index)
{
    return BIT_GET(frame_bitmap[frame_index / 32], frame_index % 32);
}

inline void frame_mark_frame_used(frame frame_index)
{
    BIT_SET(frame_bitmap[frame_index / 32], (frame_index % 32));
}

inline void frame_mark_frame_free(frame frame_index)
{
    BIT_UNSET(frame_bitmap[frame_index / 32], (frame_index % 32));
}

void frame_alloc_region(phys_addr start_phys, phys_addr end_phys)
{
    frame start_frame = (frame)start_phys / PAGE_SIZE;
    frame end_frame = (frame)end_phys / PAGE_SIZE;
    log_info("FRAME", "marking frames in [0x%x;0x%x]", start_phys, end_phys);
    log_info("FRAME", "marking frames in index [%d;%d]", start_frame, end_frame);
    log_info("FRAME", "size %x", (end_frame - start_frame) * PAGE_SIZE);
    for (int f = start_frame; f <= end_frame; f++)
    {
        frame_mark_frame_used(f);
    }
}

void frame_alloc_at(phys_addr addr)
{
    frame frame_index = (frame)addr / PAGE_SIZE;
    if (frame_get_frame(frame_index))
    {
        log_warn("FRAME", "frame %u (0x%08X) already marked used", frame_index, addr);
        return;
    }
    frame_mark_frame_used(frame_index);
}

phys_addr frame_alloc_frame()
{
    for (int i = 0; i < FRAME_BITMAP_WORDS; i++)
    {
        if (frame_bitmap[i] == 0xFFFFFFFF)
            continue; // all 32 frames in this word are used

        // find first free bit
        for (int bit = 0; bit < 32; bit++)
        {
            int frame_index = i * 32 + bit;
            if (!frame_get_frame(frame_index) && frame_index != 0)
            {
                frame_mark_frame_used(frame_index);
                return (phys_addr)((frame)frame_index*PAGE_SIZE); // return PHYSICAL address
            }
        }
    }
    log_crit("FRAME", "out of physical frames");
    panic("FRAME", __FILE__, __LINE__, "out of physical frames");
    return 0;
}

void frame_free_frame(phys_addr addr)
{
    frame frame_index = (frame)addr / PAGE_SIZE;
    frame_mark_frame_free(frame_index);
}

void frame_dump_bitmap()
{
    uint32_t free_count = 0;
    uint32_t used_count = 0;
    uint32_t total_frames = FRAME_BITMAP_WORDS;

    for (uint32_t i = 0; i < total_frames; i++)
    {
        for (int bit = 0; bit < 32; bit++)
        {
            int frame_index = i * 32 + bit;
            if (frame_get_frame(frame_index))
            {
                used_count++;
                log_debug("FRAME", "frame %u at 0x%08X: Taken", frame_index, frame_index * PAGE_SIZE);
            }
            else
            {
                free_count++;
            }
        }
    }

    log_debug("FRAME", "Number of free frames:      %u (%u KB)", free_count, free_count * 4);
    log_debug("FRAME", "Number of allocated frames: %u (%u KB)", used_count, used_count * 4);
}

void frame_init()
{
    phys_addr frame_bitmap_phys = (phys_addr)&__frame_bitmap_phys;
    virt_addr frame_bitmap_virt = (virt_addr)&__frame_bitmap;

    frame_bitmap = frame_bitmap_virt;

    log_debug("FRAME", "frame p%p v%p size is %x", frame_bitmap_phys, frame_bitmap_virt, FRAME_BITMAP_SIZE);
    memset(frame_bitmap, 0, FRAME_BITMAP_SIZE);

    log_debug("FRAME", "mapping frame bitmap");
    paging_map_region(kernel_page, frame_bitmap_virt, frame_bitmap_phys, FRAME_BITMAP_SIZE, -1);

    log_debug("FRAME", "=====BEFORE=====");
    log_debug("FRAME", "word 13195 = 0x%08x", frame_bitmap[13195]);
    log_debug("FRAME", "word 13196 = 0x%08x", frame_bitmap[13196]);
    log_debug("FRAME", "marking all frames from 0x0 to 0x%x used", (phys_addr)(frame_bitmap_phys + FRAME_BITMAP_SIZE));
    frame_alloc_region((phys_addr)0x0, (phys_addr)(frame_bitmap_phys + FRAME_BITMAP_SIZE));
    log_debug("FRAME", "=====AFTER=====");
    log_debug("FRAME", "word 13195 = 0x%08x", frame_bitmap[13195]);
    log_debug("FRAME", "word 13196 = 0x%08x", frame_bitmap[13196]);
    frame_dump_bitmap();
}