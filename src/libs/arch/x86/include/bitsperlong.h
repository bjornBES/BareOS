/*
 * File: bitsperlong.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#if defined(__x86_64__) && !defined(__ILP32__)
#define __BITS_PER_LONG 64
#else
#define __BITS_PER_LONG 32
#endif

#include <generic/bitsperlong.h>

#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif

#if 0 && BITS_PER_LONG != __BITS_PER_LONG
#error Inconsistent word size.
#endif

#if __CHAR_BIT__ * __SIZEOF_LONG__ != __BITS_PER_LONG
#error Inconsistent word size.
#endif

#ifndef BITS_PER_LONG_LONG
#define BITS_PER_LONG_LONG 64
#endif

#define small_const_nbits(nbits) (__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG && (nbits) > 0)
