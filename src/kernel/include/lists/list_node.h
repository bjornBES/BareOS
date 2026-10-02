/*
 * File: list_node.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#include "sync/spinlock.h"

#define LIST_ENTRY_NAME   this_node

#define LIST_ENTRY_THIS() list_node_t LIST_ENTRY_NAME

#define container_of(ptr, type, member)                \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define NODE_TO_TYPE_DEF(type, member)                   \
    ALWAYS_INLINE type *to_##type(void *node)            \
    {                                                    \
        return (type *)container_of(node, type, member); \
    }
#define THIS_TO_TYPE_DEF(type)                                                      \
    ALWAYS_INLINE type *to_##type(type *node)                                       \
    {                                                                               \
        return (type *)container_of(&node->LIST_ENTRY_NAME, type, LIST_ENTRY_NAME); \
    }

typedef struct list_node
{
    int curr_index;
    struct list_node *prev;
    struct list_node *next;
} list_node_t;

typedef struct
{
    bool has_initialize;
    spinlock_t list_lock;
    list_node_t *head;
    list_node_t *tail;
    uint32_t count;
} list_t;
