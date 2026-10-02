/*
 * File: hasht.h
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct hash_node
{
    // key is string
    char *key;

    // value is also string
    void *value;
    struct hash_node *next;
} hash_node_t;

typedef struct hash_map
{
    int elements_count;
    int capacity;

    hash_node_t **arr;
} hash_map_t;
