/*
 * File: fbcon.h
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/video/displayt.h"

#include <types.h>

typedef struct fbcon_cell
{
    uint32_t cp; // glyph index
    color_t fg; // palette indexes
    color_t bg; // palette indexes
} fbcon_cell_t;

typedef struct fbcon_size
{
    uint32_t cols;
    uint32_t rows;
} fbcon_size_t;

status_t fbcon_putc(uint32_t cp);                   // handles \n \r \t \b and wrapping
status_t fbcon_write(const char *s, size_t len);
status_t fbcon_clear();                             // clears grid and screen, cursor to 0,0

status_t fbcon_set_color(color_t fg, color_t bg); // applies to following output
status_t fbcon_get_size(fbcon_size_t *out);
status_t fbcon_set_cursor(uint32_t col, uint32_t row);
status_t fbcon_get_cursor(uint32_t *col, uint32_t *row);
status_t fbcon_show_cursor(int visible);

status_t fbcon_redraw(); // whole grid onto the current surface
status_t fbcon_redraw_region(uint32_t col, uint32_t row, uint32_t cols, uint32_t rows);
