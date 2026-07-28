/*
 * File: sched_types.h
 * File Created: 28 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "lists/list.h"
#include "task/threading/spinlock/spinlock_types.h"

typedef struct block_queue
{
    list_t queue;
    spinlock_t lock;
} block_queue_t;
