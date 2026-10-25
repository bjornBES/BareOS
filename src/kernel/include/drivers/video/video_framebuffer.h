/*
 * File: video_framebuffer.h
 * File Created: 20 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef enum
{
    FB_FMT_XRGB8888,
    FB_FMT_BGRX8888,
    FB_FMT_RGB888,
    FB_FMT_UNKNOWN,
} framebuffer_format_t;

typedef struct video_framebuffer
{
    vaddr_t addr;
    paddr_t phys_addr;
    size_t size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    framebuffer_format_t format;
    uint8_t r_pos, g_pos, b_pos; // bit positions
    uint8_t r_size, g_size, b_size;
} video_framebuffer_t;
