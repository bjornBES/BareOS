/*
 * File: stdio.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define NULL ((void *)0)

extern bool disableOutput;

void vga_set_cell(int x, int y, char c, uint8_t color);

void putc(char c);
void puts(const char* str);
int printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);
void vga_clear();
void vga_set_cursor(int x, int y);