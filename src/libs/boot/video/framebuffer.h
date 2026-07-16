/*
 * File: framebuffer.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"

typedef enum
{
    FB_PIXEL_RGB,
    FB_PIXEL_BGR,     // GOP commonly returns this
    FB_PIXEL_BITMASK, // use shift fields
} fb_pixel_fmt_t;

typedef struct framebuffer
{
    uint32_t mode;      //0
    uint64_t addr;      //4
    uint32_t width;     //12
    uint32_t height;    //
    uint32_t pitch; // always bytes per row
    uint8_t bpp;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    fb_pixel_fmt_t fmt;
} __attribute__((packed)) framebuffer_t;
