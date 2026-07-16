/*
 * File: bp_video.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "asm/boot/bp_config.h"
#include "framebuffer.h"

typedef struct bp_video
{
    uint32_t count;
    uint32_t max_count;
    framebuffer_t entries[MAX_VIDEO_MODES]; // MAX_VIDEO_MODES = 8
} __attribute__((packed)) bp_video_t;
