/*
 * File: hash.c
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "lists/hash.h"

#include "assert/assert.h"

#include "kerrno.h"

#include "string.h"
#include "memory.h"

#include <defs.h>

#define MODULE "hash"

status_t hash_map_create(hash_map_t **out_mp, int max_capacity)
{
    hash_map_t *mp = kmalloc(sizeof(hash_map_t));
    mp->capacity = max_capacity;
    mp->elements_count = 0;

    mp->arr = (hash_node_t **)kcalloc(max_capacity, sizeof(hash_node_t *));
    *out_mp = mp;
    return KERRNO_SUCCESSES;
}

// from https://github.com/benhoyt/ht/blob/master/ht.c
#define FNV_OFFSET 14695981039346656037ul
#define FNV_PRIME  1099511628211ul

uint64_t hash_key(hash_map_t *mp, const char *key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char *p = key; *p; p++)
    {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    uint64_t index = hash % mp->capacity;
    return index;
}

status_t hash_insert(hash_map_t *mp, char *key, void *value)
{
    ENTER_FUNC("%p, %s, %p", mp, key, value);
    if (mp == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "hash map has not been initialized");
    }
    if (mp->arr == NULL)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "hash map has not been initialized");
    }
    uint64_t bucket_index = hash_key(mp, key);
    hash_node_t *new_node = (hash_node_t *)kmalloc(sizeof(hash_node_t));

    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;

    if (mp->arr[bucket_index] == NULL)
    {
        mp->arr[bucket_index] = new_node;
    }
    else
    {
        // KERRNO_RETURN(KERRNO_NAME_IN_USE, "key is already in use");
        new_node->next = mp->arr[bucket_index];
        mp->arr[bucket_index] = new_node;
    }
    return KERRNO_SUCCESSES;
}

status_t hash_delete(hash_map_t *mp, char *key)
{
    if (mp == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "hash map has not been initialized");
    }
    if (mp->arr == NULL)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "hash map has not been initialized");
    }
    uint64_t bucket_index = hash_key(mp, key);

    hash_node_t *prev_node = NULL;

    hash_node_t *curr_node = mp->arr[bucket_index];

    while (curr_node != NULL)
    {
        if (strcmp(key, curr_node->key) == 0)
        {
            if (curr_node == mp->arr[bucket_index])
            {
                mp->arr[bucket_index] = curr_node->next;
            }
            else
            {
                prev_node->next = curr_node->next;
            }
            kfree(curr_node);
            break;
        }
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    return KERRNO_SUCCESSES;
}

status_t hash_search(hash_map_t *mp, char *key, hash_node_t *out)
{
    if (mp == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "hash map has not been initialized");
    }
    if (mp->arr == NULL)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "hash map has not been initialized");
    }
    uint64_t bucket_index = hash_key(mp, key);

    hash_node_t *bucket_head = mp->arr[bucket_index];
    while (bucket_head != NULL)
    {
        if (bucket_head->key == key)
        {
            if (out != NULL)
            {
                out = bucket_head->value;
            }
            return KERRNO_SUCCESSES;
        }
        bucket_head = bucket_head->next;
    }

    KERRNO_RETURN(ENOENT, "entry not found");
}
