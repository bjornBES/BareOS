/*
 * File: bitmap.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <base_align.h>
#include <bitsperlong.h>
#include <bitops.h>

#define DECLARE_BITMAP(name, bits) unsigned long name[BITS_TO_LONGS(bits)]

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define BITMAP_SIZE(nbits)         (ALIGN_DOWN(nbits, BITS_PER_LONG) / BITS_PER_BYTE)

#define BITMAP_ZERO(addr, nbits)   memset(addr, 0, BITMAP_SIZE(nbits));
#define BITMAP_FILL(addr, nbits)   memset(addr, 0xFF, BITMAP_SIZE(nbits));