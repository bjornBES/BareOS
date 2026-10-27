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

#include <config.h>

#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif

#define BIT_MASK(nr)                (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)                ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)            (1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)            ((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE               8
#define BITS_PER_TYPE(type)         (sizeof(type) * BITS_PER_BYTE)

#define BIT(x)                      (1ull << (x))
#define BIT_RANGE(high, low)        ((1ull << ((high) - (low) + 1)) - 1)

#define BIT_SET(x, bit)             (x) |= (1 << (bit))
#define BIT_UNSET(x, bit)           (x) &= ~(1 << (bit))
#define BIT_GET(x, bit)             (((x) >> (bit)) & 1)
#define BIT_GET_RANGE(x, low, high) (((x) >> (low)) & (BIT((high) - (low) + 1) - 1))
#define BIT_IS_SET(x, bit)          ((((x) >> (bit)) & 1) == 1)

#define BIT_ASSIGN(x, bit, value)   x = (((value == 1) ? (x) | BIT(bit) : (x) & (~BIT(bit))))
#define BIT_TEST_AND_SET(x, bit)    ((x) = BIT_IS_SET(x, bit) ? (x) | (bit) : (x) & ~(bit))

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

#define KERNEL_DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))

#define BITS_TO_LONGS(nr)          KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(long))
#define BITS_TO_U64(nr)            KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(u64))
#define BITS_TO_U32(nr)            KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(u32))
#define BITS_TO_BYTES(nr)          KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(char))

#define BYTES_TO_BITS(nb)          ((nb) * BITS_PER_BYTE)

#define SET_BIT(nr, addr)          *((addr) + BIT_WORD((nr))) |= BIT_MASK((nr))
#define CLEAR_BIT(nr, addr)        *((addr) + BIT_WORD((nr))) &= ~BIT_MASK((nr))
#define CHANGE_BIT(nr, addr)       *((addr) + BIT_WORD((nr))) ^= BIT_MASK((nr))

#define TEST_BIT(nr, addr)         (1ul & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1))))
// #define TEST_BIT_ACQUIRE(nr, addr) bitop(_test_bit_acquire, nr, addr)
