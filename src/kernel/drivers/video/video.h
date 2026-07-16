/*
 * File: video.h
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "video_ioctl.h"

#include <boot/bootparams.h>

#include "kernel.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern framebuffer_t *video_current_mode_data;

void video_set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
void video_init(boot_params_t *bp);
