/*
 * File: video_color.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "video_types.h"
#include "device/device_types.h"

#include <types.h>
#include <defs.h>


static inline void video_unpack_color(fb_info_t *fb, uint32_t packed_color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    switch (fb->fmt)
    {
        case FB_PIXEL_RGB :
            {
                *r = (uint8_t)(packed_color >> 16) & 0xFF;
                *g = (uint8_t)(packed_color >> 8) & 0xFF;
                *b = (uint8_t)(packed_color) & 0xFF;
            }
        case FB_PIXEL_BGR :
            {
                *b = (uint8_t)(packed_color >> 16) & 0xFF;
                *g = (uint8_t)(packed_color >> 8) & 0xFF;
                *r = (uint8_t)(packed_color) & 0xFF;
            }
        case FB_PIXEL_BITMASK :
            {
                *r = (uint8_t)(packed_color >> fb->red_shift) & 0xFF;
                *g = (uint8_t)(packed_color >> fb->green_shift) & 0xFF;
                *b = (uint8_t)(packed_color >> fb->blue_shift) & 0xFF;
            }
    }
}

static inline uint32_t video_pack_color(fb_info_t *fb, uint8_t r, uint8_t g, uint8_t b)
{
    switch (fb->fmt)
    {
        case FB_PIXEL_RGB :
            {
                return (r << 16) | (g << 8) | b;
            }
        case FB_PIXEL_BGR :
            {
                return (b << 16) | (g << 8) | r;
            }
        case FB_PIXEL_BITMASK :
            {
                return (r << fb->red_shift) | (g << fb->green_shift) | (b << fb->blue_shift);
            }
    }
    return 0;
}