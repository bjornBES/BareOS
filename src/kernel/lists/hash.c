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

uint64_t hash_do_hash_key(const char *key, int capacity)
{
    uint64_t hash = FNV_OFFSET;
    while (*key)
    {
        trace_debug(MODULE, "hash = %x p = %c", hash, *key);
        hash ^= (uint64_t)(unsigned char)(*key);
        hash *= FNV_PRIME;
        key++;
    }
    uint64_t index = hash % capacity;
    return index;
}

uint64_t hash_key(hash_map_t *mp, const char *key)
{
    return hash_do_hash_key(key, mp->capacity);
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
    hash_node_t *new_node = (hash_node_t *)kzmalloc(sizeof(hash_node_t));
    trace_info(MODULE, "bucket_index = 0x%x, new_node = %p", bucket_index, new_node);
    
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    new_node->index = hash_do_hash_key(key, UINT16_MAX);
    // hexdump(new_node->key, 16, 16);
    trace_info(MODULE, "new_node->key = %s, new_node->value = %p", new_node->key, new_node->value);
    
    if (mp->arr[bucket_index] == NULL)
    {
        mp->arr[bucket_index] = new_node;
    }
    else
    {
        // KERRNO_RETURN(KERRNO_NAME_IN_USE, "key '%s' is already in use", key);
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
    uint64_t index = hash_do_hash_key(key, UINT16_MAX);

    while (curr_node != NULL)
    {
        if (curr_node->index == index)
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

status_t hash_search(hash_map_t *mp, char *key, hash_node_t **out)
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
    trace_info(MODULE, "bucket_index = 0x%x", bucket_index);

    uint64_t index = hash_do_hash_key(key, UINT16_MAX);
    hash_node_t *bucket_head = mp->arr[bucket_index];
    while (bucket_head != NULL)
    {
        if (bucket_head->index == index)
        {
            trace_debug(MODULE, "bucket_head = %p, bucket_head->key = %s, key = %s, out = %p", bucket_head, bucket_head->key, key, out);
            if (out != NULL)
            {
                *out = bucket_head;
            }
            return KERRNO_SUCCESSES;
        }
        bucket_head = bucket_head->next;
    }

    KERRNO_RETURN(ENOENT, "entry not found from key \"%s\"", key);
}
