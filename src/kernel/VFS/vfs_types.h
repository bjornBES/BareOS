/*
 * File: vfs_types.h
 * File Created: 20 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "vfs_config.h"

typedef struct vfs_node vfs_node_t;

typedef struct
{
    vfs_node_t *node;
    uint32_t refcount;
} fd_entry_t;

typedef struct
{
    fd_entry_t *entries[FD_SIZE];
} fd_table_t;
