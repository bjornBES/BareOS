/*
 * File: arch_types.h
 * File Created: 05 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <bitops.h>

#include "bitsperlong.h"

#define __BYTE_ORDER __BYTE_ORDER__

#ifdef __x86_64__
typedef long long int32_64_t;
typedef unsigned long long uint32_64_t;
typedef unsigned long long reg_t;
#define _NSIG_BPW 64
#else
typedef int int32_64_t;
typedef unsigned int uint32_64_t;
typedef unsigned int reg_t;
#define _NSIG_BPW 32
#endif
typedef uint8_t interrupt_vector_t;

#define _NSIG       64
#define _NSIG_WORDS (_NSIG / _NSIG_BPW)
#define IRQ_MAX     222

typedef struct irq_mask
{
    long bits[BITS_TO_LONGS(IRQ_MAX)];
} irq_mask_t;


typedef uint32_64_t paddr_t;
typedef uint32_64_t vaddr_t;
