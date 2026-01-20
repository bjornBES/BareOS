#pragma once
#include <stdint.h>
#include <core/arch/i686/VGATextDevice.h>

#define NULL ((void *)0)

void putc(char c);
void puts(const char* str);
int printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);