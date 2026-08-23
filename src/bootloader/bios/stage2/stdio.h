/*
 * File: stdio.h
 * File Created: 18 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define NULL ((void *)0)

void vga_set_cell(int x, int y, char c, uint8_t color);

void putc(char c);
void puts(const char* str);
int printf(const char* fmt, ...);
void vga_clear();
void vga_set_cursor(int x, int y);
