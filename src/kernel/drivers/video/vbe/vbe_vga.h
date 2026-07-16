/*
 * File: vbe_vga.h
 * File Created: 11 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/video/video_types.h"

void vbe_put_pixel(fb_info_t *fb, uint32_t x, uint32_t y, color_t color);