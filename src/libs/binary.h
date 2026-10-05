/*
 * File: binary.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define BIT(x)                      (1ull << (x))
#define BIT_RANGE(high, low)        ((1ull << ((high) - (low) + 1)) - 1)

#define BIT_SET(x, bit)             (x) |= (1 << (bit))
#define BIT_UNSET(x, bit)           (x) &= ~(1 << (bit))
#define BIT_GET(x, bit)             (((x) >> (bit)) & 1)
#define BIT_GET_RANGE(x, low, high) (((x) >> (low)) & (BIT((high) - (low) + 1) - 1))
#define BIT_IS_SET(x, bit)          ((((x) >> (bit)) & 1) == 1)

#define BIT_ASSIGN(x, bit, value)   ((value == 1) ? BIT_SET((x), (bit)) : BIT_UNSET((x), (bit)))

#define BITS_PER_BYTE               8
#define BITS_PER_TYPE(type)         (sizeof(type) * BITS_PER_BYTE)

#define FLAG_SET(x, flag)           ((x) |= (flag))
#define FLAG_UNSET(x, flag)         ((x) &= ~(flag))
#define FLAG_IS_SET(x, flag)        (((x) & (flag)) == (flag))
#define FLAG_GET(x, flag)           ((x) & (flag))
#define FLAG_TEST_AND_SET(x, flag)  ((x) = FLAG_IS_SET(x, flag) ? (x) | (flag) : (x) & ~(flag))

#define FLAG_ASSIGN(x, flag, value) (x) = (((value) == 1) ? (x) | (flag) : (x) & ~(flag))

#define BIT_FIRST_ZERO(x, type, res)            \
    {                                           \
        for (type i = 0; i < sizeof(type); i++) \
        {                                       \
            if (BIT_GET(x, i) == 0)             \
            {                                   \
                res = i;                        \
                break;                          \
            }                                   \
        }                                       \
    }
