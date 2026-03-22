/*
 * File: VGATextDevice.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

typedef struct {
    uint32_t fg_color;  // full 32bpp ready
    uint32_t bg_color;  // full 32bpp ready
    char ch;
} vga_cell_t;

void vga_clear();

void vga_set_cell(int x, int y, char c, uint32_t fg_color, uint32_t bg_color);

void vga_set_cursor(int x, int y);
void vga_get_cursor(int* x, int* y);
void vga_scrollback(int lines);

void vga_put_char(char c);

void vga_load_font(void *font);

void vga_set_mode();

void vga_init();

void vga_check();

extern int screen_x, screen_y;
extern uint8_t *g_ScreenBuffer;