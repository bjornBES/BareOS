/*
 * File: base_types.h
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef long int32_64;
typedef unsigned long uint32_64;

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
