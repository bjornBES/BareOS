/*
 * File: bits.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define BIT_MASK(nr)                (1ul << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)                ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)            (1ull << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)            ((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE               8
#define BITS_PER_TYPE(type)         (sizeof(type) * BITS_PER_BYTE)