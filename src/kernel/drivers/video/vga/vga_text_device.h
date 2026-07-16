/*
 * File: vga_text_device.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include "drivers/video/video_types.h"


void vga_set_cursor(int x, int y);
void vga_get_cursor(int* x, int* y);
void vga_scrollback(int lines);

void vga_put_char(char c);

video_ops_t *vga_get_ops();
