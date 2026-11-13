/*
 * File: vfs_table.h
 * File Created: 05 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internals.h"

#include "dev/devicet.h"

#include <types.h>

typedef struct fd_entry
{
    vfs_node_t *node;
    refcount_t refcount;
    off_t offset;
} fd_entry_t;

typedef struct fd_table
{
    fd_entry_t *entries[CONFIG_PROCESS_MAX_OPEN_FD];
} fd_table_t;

status_t vfs_clone_node(fd_table_t *table, fd_table_t *dest);
status_t vfs_open_file(fd_table_t *table, const char *path, fd_t *out_fd);
status_t vfs_open_dev(fd_table_t *table, device_t *dev, fd_t *out_fd);
