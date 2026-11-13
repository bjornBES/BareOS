/*
 * File: generic_memory.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "defs.h"

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memset32(void *s, int c, size_t n);
#ifdef CONFIG_64BIT
static INLINE void *memset64(void *s, long long c, size_t n);
#endif
int memcmp(const char *cs, const char *ct, size_t num);
void *memmove(void *dest, const void *src, size_t n);
int memchr(const char *s, int c, size_t count);
void *memscan(void *s, int c, size_t size);
