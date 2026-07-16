/*
 * File: video_types.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "task/threading/spinlock/spinlock_types.h"

#include <boot/video/framebuffer.h>

typedef uint32_t color_t;
struct video_ops;

typedef struct
{
    void *addr; // mapped virtual address
    uint32_t width;
    uint32_t height;
    uint32_t pitch; // bytes per row
    uint8_t bpp;    // bits per pixel (always 32)
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    fb_pixel_fmt_t fmt;
} fb_info_t;

typedef struct video_backend
{
    struct video_ops *ops;
    fb_info_t fb;
    void *priv; // backend specific data
} video_backend_t;

typedef struct video_ops
{
    int (*init)(video_backend_t *backend);
    void (*set_pixel)(video_backend_t *backend, uint32_t x, uint32_t y, color_t color);
    void (*fill_rect)(video_backend_t *backend, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
    void (*flip)(video_backend_t *backend); // double buffering
    void (*shutdown)(video_backend_t *backend);
} video_ops_t;


typedef struct
{
    fb_info_t fb;    // framebuffer info
    spinlock_t lock; // for concurrent access
} video_priv_t;
