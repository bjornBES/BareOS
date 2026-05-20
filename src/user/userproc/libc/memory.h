/*
 * File: memory.h
 * File Created: 17 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <types.h>

void* memcpy(void* dst, const void* src, size_t num);

void* memmove(void* dst, const void* src, size_t num);

void* memset(void* ptr, int value, size_t num);

int memcmp(const void* ptr1, const void* ptr2, size_t num);

void* memchr(const void* ptr, int value, size_t num);