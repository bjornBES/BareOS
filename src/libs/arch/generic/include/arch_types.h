/*
 * File: arch_types.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <binary.h>

#define __BYTE_ORDER __BYTE_ORDER__

#ifdef CONFIG_64BIT
typedef long long int32_64_t;
typedef unsigned long long uint32_64_t;
typedef unsigned long long reg_t;
#define BITS_PER_LONG 64
#define _NSIG_BPW     64
#else
typedef int int32_64_t;
typedef unsigned int uint32_64_t;
typedef unsigned int reg_t;
#define BITS_PER_LONG 32
#define _NSIG_BPW     32
#endif
typedef uint8_t interrupt_vector_t;

#define _NSIG       64
#define _NSIG_WORDS (_NSIG / _NSIG_BPW)
#define IRQ_MAX     222

typedef struct irq_mask
{
    long bits[BITS_TO_LONGS(IRQ_MAX)];
} irq_mask_t;

typedef struct timespec
{
    time_t tv_sec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER == 4321);
    int64_t tv_nsec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER != 4321);
} timespec_t;

typedef uint32_64_t paddr_t;
typedef uint32_64_t vaddr_t;
