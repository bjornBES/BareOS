/*
 * File: arch_time_types.h
 * File Created: 11 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "arch_types.h"

typedef uint32_t useconds_t;
typedef int32_64_t suseconds_t;
typedef uint32_64_t time_t;
typedef struct timespec
{
    time_t tv_sec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER == 4321);
    int64_t tv_nsec;
    int : 8 * (sizeof(time_t) - sizeof(long)) * (__BYTE_ORDER != 4321);
} timespec_t;

typedef struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
} timeval_t;
