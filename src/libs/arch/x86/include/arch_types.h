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

#include <binary.h>

#define __BYTE_ORDER 1234


#ifdef __x86_64__
typedef long long int32_64_t;
typedef unsigned long long uint32_64_t;
typedef unsigned long long reg_t;
#else
typedef int int32_64_t;
typedef unsigned int uint32_64_t;
typedef unsigned int reg_t;
#endif
typedef uint8_t interrupt_vector_t;

#define _SIGSET_NWORDS (int)(128 / sizeof(long))
typedef struct __kernel_sigset_t
{
    uint32_64_t __bits[_SIGSET_NWORDS];
} kernel_sigset_t;

#define IRQ_MAX 222
typedef struct irq_mask
{
    long bits[BITS_TO_LONGS(IRQ_MAX)];
} irq_mask_t;

typedef uint32_64_t sigset_t;

typedef int32_64_t suseconds_t;
typedef uint32_64_t time_t;

typedef struct time_struct
{
    uint32_t second;
    uint32_t minute;
    uint32_t hour;
    uint32_t day;
    uint32_t month;
    uint32_t year;
} time_struct_t;

typedef int32_t fd_t;

typedef struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
} timeval_t;

typedef struct timespec
{
    time_t tv_sec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER == 4321);
    int64_t tv_nsec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER != 4321);
} timespec_t;

typedef uint32_64_t paddr_t;
typedef uint32_64_t vaddr_t;
