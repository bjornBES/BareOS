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

#include <stdint.h>

#ifndef __BITS_PER_LONG
#if defined(__CHAR_BIT__) && defined(__SIZEOF_LONG__)
#define __BITS_PER_LONG (__CHAR_BIT__ * __SIZEOF_LONG__)
#else
#define __BITS_PER_LONG 32
#endif
#endif

#ifndef __BITS_PER_LONG_LONG
#define __BITS_PER_LONG_LONG 64
#endif