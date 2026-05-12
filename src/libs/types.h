/*
 * File: types.h
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __x86_64__
typedef uint64_t uint32_64;
typedef uint64_t reg_t;
#else
typedef uint32_t uint32_64;
typedef uint32_t reg_t;
#endif

typedef void *phys_addr;
typedef void *virt_addr;

typedef int32_t off_t;
typedef uint64_t time_t;
typedef uint64_t pid_t;
typedef uint64_t cpu_id;

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} time_struct;

typedef int fd_t;