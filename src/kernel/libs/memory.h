/*
 * File: memory.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <core/Defs.h>
#include <stdint.h>
#include <stddef.h>

ASMCALL void* memcpy(void* dst, const void* src, size_t num);
ASMCALL void* memset(void* ptr, int value, size_t num);
ASMCALL uint16_t* memset16(uint16_t* ptr, uint16_t value, size_t num);
ASMCALL uint32_t* memset32(uint32_t* ptr, uint32_t value, size_t num);
ASMCALL void* memmove(void* dst, const void* src, size_t num);
ASMCALL int memcmp(const void* ptr1, const void* ptr2, size_t num);
ASMCALL void* memchr(const void* ptr, int value, size_t num);