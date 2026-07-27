/*
 * File: list.h
 * File Created: 27 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"

typedef struct list_node
{
    struct list_node *prev;
    struct list_node *next;
} list_node_t;

typedef struct
{
    list_node_t *head;
    list_node_t *tail;
    uint32_t count;
} list_t;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void list_init(list_t *list);
int list_push_head(list_t *list, list_node_t *node);
int list_push_tail(list_t *list, list_node_t *node);
list_node_t *list_pop_head(list_t *list);
list_node_t *list_pop_tail(list_t *list);
int list_remove(list_t *list, list_node_t *node);
bool list_is_empty(list_t *list);
