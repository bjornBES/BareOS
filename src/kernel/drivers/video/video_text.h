/*
 * File: video_text.h
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "device/device_types.h"

void video_text_load_font_from_data(void *font);
void video_text_set_cell(device_t *video_driver, int x, int y, char c, uint32_t fg_color, uint32_t bg_color);