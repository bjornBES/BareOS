/*
 * File: block_queue.h
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "sync/spinlock.h"
#include "lists/list_node.h"

typedef struct block_queue
{
    list_t queue;
    spinlock_t lock;
} block_queue_t;
