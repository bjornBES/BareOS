/*
 * File: video_common.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "video_types.h"
#include "video_color.h"
#include "device/device_types.h"
#include "kernel/memory.h"
#include <types.h>
#include <defs.h>

static inline void video_pack_to_buffer(fb_info_t *fb, uint8_t *buffer, uint8_t r, uint8_t g, uint8_t b)
{
    switch (fb->fmt)
    {
        case FB_PIXEL_RGB :
            {
                buffer[0] = r;
                buffer[1] = g;
                buffer[2] = b;
            }
        case FB_PIXEL_BGR :
            {
                buffer[0] = b;
                buffer[1] = g;
                buffer[2] = r;
            }
        case FB_PIXEL_BITMASK :
            {
                uint32_t color = video_pack_color(fb, r, g, b);
                memcpy(buffer, &color, fb->bpp / 8);
            }
    }
}

static inline void fb_unpack_index(fb_info_t *fb, uint32_t index, uint32_t *x, uint32_t *y)
{
    *y = index / fb->width;
    *x = index % fb->width;
}
