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

#define __BYTE_ORDER 1234


#ifdef __x86_64__
typedef long long int32_64;
typedef unsigned long long uint32_64;
typedef unsigned long long reg_t;
#else
typedef int int32_64;
typedef unsigned int uint32_64;
typedef unsigned int reg_t;
#endif
typedef uint32_t interrupt_vector;

#define _SIGSET_NWORDS (int)(128 / sizeof(long))
typedef struct __kernel_sigset_t
{
    uint32_64 __bits[_SIGSET_NWORDS];
} kernel_sigset_t;

typedef uint32_64 sigset_t;

typedef int32_64 suseconds_t;
typedef uint32_64 time_t;

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

typedef uint32_64 paddr_t;
typedef uint32_64 vaddr_t;
