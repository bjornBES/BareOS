/*
 * File: semaphore.h
 * File Created: 08 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "spinlock.h"
#include "lists/list_node.h"

#include <types.h>

typedef struct
{
    spinlock_t lock;
    int count;
    // block_queue_t waiters;
    list_t queue_list;
} semaphore_t;