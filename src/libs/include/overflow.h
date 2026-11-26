/*
 * File: overflow.h
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>

static ALWAYS_INLINE MUST_CHECK bool __must_check_overflow(bool overflow)
{
    return UNLIKELY(overflow);
}

#define check_mul_overflow(a, b, d)                        \
    __must_check_overflow(__builtin_mul_overflow(a, b, d))

#define check_add_overflow(a, b, d)                        \
    __must_check_overflow(__builtin_add_overflow(a, b, d))
