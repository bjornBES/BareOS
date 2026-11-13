/*
 * File: ffs.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>

static ALWAYS_INLINE ATTRIBUTE_CONST unsigned int generic_ffs(unsigned long word)
{
    unsigned int num = 0;

#if BITS_PER_LONG == 64
    if ((word & 0xFFFFFFFF) == 0)
    {
        num += 32;
        uint64_t u64_word = word;
        u64_word >>= 32;
        word = (unsigned long)u64_word;
    }
#endif
    if ((word & 0xFFFF) == 0)
    {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xFF) == 0)
    {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xF) == 0)
    {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0)
    {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
    {
        num += 1;
    }
    return num;
}

#ifndef HAVE_ARCH_FFS
#define FFS(word) generic_ffs(word)
#endif
