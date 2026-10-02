/*
 * File: hash.h
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "hasht.h"

#include <types.h>

status_t hash_map_crate(hash_map_t **mp, int max_capacity);
void hash_map_destroy(hash_map_t* mp);

status_t insert(hash_map_t *mp, char *key, void *value);
status_t delete(hash_map_t *mp, char *key);
status_t search(hash_map_t *mp, char *key, hash_node_t **out);
