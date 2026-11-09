/*
 * File: pidt.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "sync/spinlock.h"
#include <types.h>

#define MAX_BLOCKS    32
#define PID_MAX       4096

#define RESERVED_PIDS 300

typedef struct process process_t;

typedef struct pid_block
{
    uint32_t base;                       // block_index * PID_MAX
    uint32_t allocated;
    uint32_t hint;                       // cyclic search start
    long bitmap[BITS_TO_LONGS(PID_MAX)]; // PID_MAX bits
    process_t **procs;                   // slot -> process
    spinlock_t block_lock;
} pid_block_t;
