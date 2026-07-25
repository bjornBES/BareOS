/*
 * File: fd_table.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internal.h"
#include "vfs_types.h"

// find lowest free slot, store node ptr
fd_t fd_alloc(vfs_node_t *node);

fd_t fd_alloc_for_process(vfs_node_t *node, fd_table_t *table);

fd_entry_t *fd_get_for_process(fd_t fd, fd_table_t *table);

// validate fd, return node ptr
vfs_node_t *fd_get(fd_t fd);

// null the slot, free node
void fd_release(fd_t fd);

// zero table, reserve stdin/stdout/stderr/debug
void fd_table_init();
