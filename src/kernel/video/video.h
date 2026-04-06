/*
 * File: video.h
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

/*
 * File: video.h
 * File Created: 27 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <boot/bootparams.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    uint16_t mode_attributes;
    uint8_t memory_model;

    // ---- Resolution & format ----
    uint16_t width;
    uint16_t height;
    uint8_t bpp;

    // ---- Linear frame_buffer ----
    uint32_t frame_buffer;
    uint32_t pitch;

    // ---- Color layout ----
    uint8_t red_mask_size;
    uint8_t red_field_position;

    uint8_t greenMaskSize;
    uint8_t greenFieldPosition;

    uint8_t blueMaskSize;
    uint8_t blueFieldPosition;

    uint8_t alphaMaskSize;
    uint8_t alphaFieldPosition;

    uint16_t mode;
} video_mode;

extern video_mode *video_current_mode_data;

void video_set_pixel(uint32_t cursor_x, uint32_t cursor_y, uint32_t color_packed);
void video_set_mode(int mode);
void video_init(boot_params *bp, void *mode_addr);
