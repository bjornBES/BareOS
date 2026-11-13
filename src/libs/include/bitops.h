/*
 * File: bitops.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <bits.h>
#include <stdint.h>

#define is_signed_type(type) (((type)(-1)) < (type)1)
#define type_half_max(type) ((type)1 << (8*sizeof(type) - 1 - is_signed_type(type)))
#define type_max(t)	((typeof(t))((type_half_max(typeof(t)) - 1) + type_half_max(typeof(t))))

#define KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define BITS_TO_LONGS(nr)         KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(long))
#define BITS_TO_U64(nr)           KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(uint64_t))
#define BITS_TO_U32(nr)           KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(uint32_t))
#define BITS_TO_BYTES(nr)         KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(char))

#define BYTES_TO_BITS(nb)         ((nb) * BITS_PER_BYTE)

#define SET_BIT(nr, addr)         *((addr) + BIT_WORD((nr))) |= BIT_MASK((nr))
#define CLEAR_BIT(nr, addr)       *((addr) + BIT_WORD((nr))) &= ~BIT_MASK((nr))
#define CHANGE_BIT(nr, addr)      *((addr) + BIT_WORD((nr))) ^= BIT_MASK((nr))

#define TEST_BIT(nr, addr)        (1ul & ((addr)[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1))))
#define TEST_AND_SET_BIT(nr, addr)                                                                   \
    ((((addr)[BIT_WORD(nr)] & BIT_MASK(nr)) != 0) ? 1 : (((addr)[BIT_WORD(nr)] |= BIT_MASK(nr)), 0))

#define GENMASK_TYPE(t, h, l)                            \
    ((t)((type_max(t) << (l) &                           \
          type_max(t) >> (BITS_PER_TYPE(t) - 1 - (h)))))

#define GENMASK(h, l) GENMASK_TYPE(unsigned long, h, l)
