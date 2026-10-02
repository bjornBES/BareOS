/*
 * File: volume.h
 * File Created: 02 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "lists/list_node.h"
#include "vfs/fs_registry.h"
#include "vfs_internals.h"
#include "dev/devicet.h"

#include <config.h>
#include <types.h>

#define VOLUME_READONLY 0x0001
#define VOLUME_VIRTUAL 0x0002
#define VOLUME_REMOVABLE 0x0004


typedef struct volume
{
    LIST_ENTRY_THIS();
    char volume_id[CONFIG_MAX_VOLUME_LENGTH];
    filesystem_t *fs;
    device_t *device;
    void *sb;
    uint32_t flags;
    uint32_t refcount;
    mountpoint_t *mountpoint;
} volume_t;

THIS_TO_TYPE_DEF(volume_t);

// probe fs drivers, alloc + insert volume
status_t volume_register(const char *volume_id, device_t *dev, volume_t **out);

status_t volume_create_synthetic(const char *volume_id, const char *fs_name, device_t *dev, volume_t **out);

// remove from registry, free
status_t volume_unregister(const char *volume_id);

// lookup by id string
status_t volume_find(const char *volume_id, volume_t **out);

// parse "vol_id/" prefix, return volume
status_t volume_from_prefix(const char *path, volume_t **out);

// return all registered volumes
status_t volume_list(volume_t **buf, size_t buf_size, size_t *out_size);