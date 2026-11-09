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

#include <types.h>

typedef struct fd_entry
{
    vfs_node_t *node;
    refcount_t refcount;
} fd_entry_t;

typedef struct fd_table
{
    fd_entry_t *entries[CONFIG_PROCESS_MAX_OPEN_FD];
} fd_table_t;

status_t vfs_open_node(fd_table_t *table, int index);
