/*
 * File: types.h
 * File Created: 05 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "uapi_types.h"

typedef int64_t off_t;

typedef uint32_t useconds_t;

typedef int32_64_t tid_t;

typedef int32_64_t ino_t;
typedef int32_64_t dev_t;
typedef uint64_t kernel_dev_t;

typedef int32_64_t ssize_t;
typedef int32_64_t clock_t;

typedef int32_64_t blksize_t;
typedef uint64_t sector_t;
typedef uint64_t blkcnt_t;

/* generic data direction definitions */
#define READ  0
#define WRITE 1

/*
 * The type of an index into the pagecache.
 */
typedef uint32_64_t pgoff_t;

typedef int32_t pid_t;

typedef uint32_t mode_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t id_t;

typedef int32_64_t nlink_t;
typedef uint32_t cpu_logical_id_t;

/// @brief the default return type of functions
typedef int32_t status_t;

typedef uint32_t gsi_t;

#define KERNEL_IRQ_NONE (kernel_irq_t)(~0)
typedef uint32_t kernel_irq_t;

typedef uint32_t color_t;

typedef uint32_t refcount_t;
