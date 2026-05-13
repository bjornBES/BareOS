/*
 * File: volume.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vfs_internal.h"

// probe fs drivers, alloc + insert volume
volume_t *volume_register(const char *volume_id, device_t *dev);

// remove from registry, free
void volume_unregister(const char *volume_id);

// lookup by id string
volume_t *volume_find(const char *volume_id);

// parse "vol_id:/" prefix, return volume
volume_t *volume_from_prefix(const char *path);

// return all registered volumes
int volume_list(volume_t **buf, int buf_size);

void volume_init();
