/*
 * File: list.c
 * File Created: 27 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "list.h"
#include <base_errno.h>
#include "debug/debug.h"

void list_init(list_t *list)
{
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

int list_push_head(list_t *list, list_node_t *node)
{
    node->next = list->head;
    node->prev = NULL;

    if (list->head)
    {
        list->head->prev = node;
    }
    else
    {
        list->tail = node; // was empty — this node is now also the tail
    }

    list->head = node;
    list->count++;
    return 0;
}

int list_push_tail(list_t *list, list_node_t *node)
{
    node->prev = list->tail;
    node->next = NULL;

    if (list->tail)
    {
        list->tail->next = node;
    }
    else
    {
        list->head = node; // was empty — this node is now also the head
    }

    list->tail = node;
    list->count++;
    return 0;
}

list_node_t *list_pop_head(list_t *list)
{
    list_node_t *curr_node = list->head;
    if (!curr_node)
    {
        return (void *)-EPERM; // empty list
    }

    list->head = curr_node->next;
    if (list->head)
    {
        list->head->prev = NULL;
    }
    else
    {
        list->tail = NULL; // list is now empty
    }

    curr_node->next = NULL;
    curr_node->prev = NULL;
    list->count--;
    return curr_node;
}

list_node_t *list_pop_tail(list_t *list)
{
    list_node_t *curr_node = list->tail;
    if (!curr_node)
    {
        return NULL; // empty list
    }

    list->tail = curr_node->prev;
    if (list->tail)
    {
        list->tail->next = NULL;
    }
    else
    {
        list->head = NULL; // list is now empty
    }

    curr_node->next = NULL;
    curr_node->prev = NULL;
    list->count--;
    return curr_node;
}

int list_remove(list_t *list, list_node_t *node)
{
    if (!node)
    {
        return 1;
    }

    list_node_t *curr = list->head;
    while (curr != NULL && curr != node)
    {
        // log_debug(NO_MODULE, "curr = %p", curr);
        curr = curr->next;
    }

    if (curr == NULL)
    {
        return 1; // not found
    }

    if (curr->prev)
    {
        curr->prev->next = curr->next;
    }
    else
    {
        list->head = curr->next; // was the head
    }

    if (curr->next)
    {
        curr->next->prev = curr->prev;
    }
    else
    {
        list->tail = curr->prev; // was the tail
    }

    curr->next = NULL;
    curr->prev = NULL;
    list->count--;
    return 0;
}

bool list_is_empty(list_t *list)
{
    return list->count == 0;
}
