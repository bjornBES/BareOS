/*
 * File: display.h
 * File Created: 20 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "displayt.h"

#include "pci/pci.h"

#include <config.h>

#include <types.h>

#define MODULE_VIDEO_DRIVER(__video_driver)                                           \
    MODULE_DRIVER(__video_driver, display_driver_register, display_driver_unregister)

typedef status_t (*display_listener_t)(display_device_t *new_primary, void *ctx);

display_device_t *display_create();

status_t display_register(display_device_t *dev);
status_t display_unregister(display_device_t *dev);
display_device_t *display_get_primary();

status_t display_driver_register(display_driver_t *drv);
status_t display_driver_unregister(display_driver_t *drv);

status_t display_set_mode(display_device_t *dev, const display_mode_t *m);
status_t display_set_default_mode(display_device_t *dev);
status_t display_flush(display_device_t *dev, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
const video_framebuffer_t *display_get_fb(display_device_t *dev);

status_t display_add_listener(display_listener_t cb, void *ctx);

status_t display_notify_listeners(display_device_t *dev);

status_t display_draw_putpixel(const video_framebuffer_t *fb, uint32_t x, uint32_t y, color_t color);
status_t display_draw_fillrect(const video_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
status_t display_draw_clear(const video_framebuffer_t *fb, color_t color);
status_t display_draw_rect(const video_framebuffer_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
status_t display_draw_blit(const video_framebuffer_t *dst, uint32_t dx, uint32_t dy, const void *src, uint32_t src_pitch, uint32_t w, uint32_t h);
status_t display_draw_blit_mono(const video_framebuffer_t *dst, uint32_t x, uint32_t y, const uint8_t *bitmap, uint32_t w, uint32_t h, color_t fg, color_t bg);
status_t display_draw_copyrect(const video_framebuffer_t *fb, uint32_t dx, uint32_t dy, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h);
status_t display_draw_scroll_up(const video_framebuffer_t *fb, uint32_t lines, color_t fill_color);
