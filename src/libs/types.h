/*
 * File: types.h
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

#ifdef __x86_64__
typedef uint64_t uint32_64;
typedef uint64_t reg_t;
#else
typedef uint32_t uint32_64;
typedef uint32_t reg_t;
#endif

typedef void *phys_addr;
typedef void *virt_addr;

typedef int64_t off_t;

typedef int64_t time_t;
typedef int64_t suseconds_t;

typedef int64_t tid_t;

typedef int64_t ino_t;
typedef int64_t dev_t;

typedef int64_t ssize_t;

typedef int32_t pid_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t id_t;
typedef uint32_t useconds_t;

typedef uint64_t cpu_id;
typedef uint32_64 paddr_t;
typedef uint32_64 vaddr_t;
typedef uint64_t sigset_t;

typedef struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
} timeval_t;

typedef struct timespec
{
    time_t tv_sec;
    int64_t tv_nsec;
} timespec_t;

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
