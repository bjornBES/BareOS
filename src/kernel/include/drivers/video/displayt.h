/*
 * File: displayt.h
 * File Created: 20 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "video_framebuffer.h"

#include "lists/list_node.h"

#include "dev/devicet.h"

#include <types.h>

typedef uint32_t color_t;

typedef struct display_mode
{
    uint32_t w;
    uint32_t h;
    uint8_t bpp;
} display_mode_t;

typedef struct display_driver display_driver_t;

typedef struct display_device
{
    device_t dev;
    LIST_ENTRY_THIS();

    const display_driver_t *driver;

    video_framebuffer_t frame_buffer;
    int priority; // firmware = 0, native driver = higher


    void *priv;
} display_device_t;

typedef struct display_driver
{
    const char *name;                                                                         // "bochs", "firmware-fb"
    status_t (*set_mode)(display_device_t *display_dev, const display_mode_t *mode);                  // optional
    status_t (*flush)(display_device_t *display_dev, uint32_t x, uint32_t y, uint32_t w, uint32_t h); // optional
    status_t (*release)(display_device_t *display_dev);                                               // handoff / unregister

} display_driver_t;

