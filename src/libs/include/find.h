/*
 * File: find.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <bitmap.h>
#include <ffs.h>

unsigned long _find_next_bit(unsigned long *addr, unsigned long nbits, unsigned long start);

static ALWAYS_INLINE unsigned long find_next_bit(unsigned long *addr, unsigned long size, unsigned long offset)
{
    if (small_const_nbits(size))
    {
        unsigned long val;

        if (UNLIKELY(offset >= size))
        {
            return size;
        }

        val = *addr & GENMASK(size - 1, offset);
        return val ? FFS(val) : size;
    }

    return _find_next_bit(addr, size, offset);
}

static ALWAYS_INLINE unsigned long find_first_bit(unsigned long *addr, unsigned long size)
{
	if (small_const_nbits(size)) {
		unsigned long val = *addr & GENMASK(size - 1, 0);

		return val ? FFS(val) : size;
	}

	return (~0); // _find_first_bit(addr, size);
}