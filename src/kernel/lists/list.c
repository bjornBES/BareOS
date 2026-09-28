/*
 * File: list.c
 * File Created: 27 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "lists/list.h"

#include "kerrno.h"

#include <defs.h>

#define MODULE "list"

void list_initialize(list_t *list)
{
    list->has_initialize = true;
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

status_t list_push_head(list_t *list, list_node_t *node)
{
    if (!list)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "list is null");
    }
    if (!node)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "node is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }
    node->next = list->head;
    node->prev = NULL;
    node->curr_index = 0;

    if (list->head)
    {
        list->head->prev = node;
    }
    else
    {
        list->tail = node;
    }

    list->head = node;
    list->count++;

    // reindex everyone after the new head, tail included
    list_node_t *cur = node->next;
    while (cur)
    {
        cur->curr_index++;
        cur = cur->next;
    }
    spinlock_release(&list->list_lock);
    return KERRNO_SUCCESSES;
}

status_t list_push_tail(list_t *list, list_node_t *node)
{
    if (!list)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "list is null");
    }
    if (!node)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "node is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }

    node->prev = list->tail;
    node->next = NULL;

    if (list->tail)
    {
        node->curr_index = list->tail->curr_index + 1;
        list->tail->next = node;
    }
    else
    {
        node->curr_index = 0;
        list->head = node;
    }

    list->tail = node;
    list->count++;
    spinlock_release(&list->list_lock);
    return KERRNO_SUCCESSES;
}

INTERNAL status_t list_internal_reindex(list_t *list)
{
    if (!list)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "list is null");
    }
    if (!list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }
    if (list->count == 0)
    {
        return KERRNO_SUCCESSES;
    }
    // reindex everyone after the new head, tail included
    list->head->curr_index = 0;
    list_node_t *priv = list->head;
    if (!priv || list->count == 0)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "empty list");
    }
    list_node_t *cur = priv->next;
    while (cur)
    {
        cur->curr_index = priv->curr_index + 1;
        priv = cur;
        cur = cur->next;
    }
    return KERRNO_SUCCESSES;
}

status_t list_reindex(list_t *list)
{
    if (!list)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "list is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }
    status_t ret = list_internal_reindex(list);
    spinlock_release(&list->list_lock);
    return ret;
}

list_node_t *list_pop_head(list_t *list)
{
    if (!list)
    {
        KERRNO_RETURN_TYPE(KERRNO_BAD_VALUE, list_node_t *, "list is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN_TYPE(KERRNO_NOT_INITIALIZED, list_node_t *, "list has not been initialized");
    }
    if (list->count == 0)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN_TYPE(KERRNO_NOT_ALLOWED, list_node_t *, "empty list");
    }
    list_node_t *curr_node = list->head;
    if (!curr_node)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN_TYPE(KERRNO_NOT_ALLOWED, list_node_t *, "empty list");
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

    list_internal_reindex(list);
    spinlock_release(&list->list_lock);
    return curr_node;
}

list_node_t *list_pop_tail(list_t *list)
{
    if (!list)
    {
        KERRNO_RETURN_TYPE(KERRNO_BAD_VALUE, list_node_t *, "list is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN_TYPE(KERRNO_NOT_INITIALIZED, list_node_t *, "list has not been initialized");
    }
    if (list->count == 0)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN_TYPE(KERRNO_NOT_ALLOWED, list_node_t *, "empty list");
    }
    list_node_t *curr_node = list->tail;
    if (!curr_node)
    {
        spinlock_release(&list->list_lock);
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
    spinlock_release(&list->list_lock);
    return curr_node;
}

status_t list_remove(list_t *list, list_node_t *node)
{
    if (!node)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "node is null");
    }

    if (!list)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "list is null");
    }
    spinlock_acquire(&list->list_lock);
    if (!list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }
    if (list->count == 0)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "empty list");
    }

    list_node_t *curr = list->head;
    while (curr != NULL && curr != node)
    {
        curr = curr->next;
    }

    if (curr == NULL)
    {
        spinlock_release(&list->list_lock);
        KERRNO_RETURN(KERRNO_BAD_DATA, "couldn't find the node");
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

    list_internal_reindex(list);

    spinlock_release(&list->list_lock);
    return 0;
}

bool list_is_empty(list_t *list)
{
    return list->count == 0;
}
