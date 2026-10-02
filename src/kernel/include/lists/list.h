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

#include <types.h>

#include "list_node.h"

#define ADD_THIS_TO_LIST(list, n) list_push_tail(&list, &n->LIST_ENTRY_NAME);

void list_initialize(list_t *list);
status_t list_push_head(list_t *list, list_node_t *node);
status_t list_push_tail(list_t *list, list_node_t *node);
list_node_t *list_pop_head(list_t *list);
list_node_t *list_pop_tail(list_t *list);
status_t list_remove(list_t *list, list_node_t *node);
bool list_is_empty(list_t *list);
