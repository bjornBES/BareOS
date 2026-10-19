/*
 * File: inode.h
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs/vfs_internals.h"
#include "vfs/volume.h"

#include <types.h>

// call fs->alloc_inode, insert icache
status_t inode_alloc(volume_t *vol, inode_t **out);

// call fs->free_inode, remove from icache
status_t inode_free(inode_t *ino);

// increment refcount
status_t inode_get(inode_t *ino, inode_t **out);

// decrement refcount, free if zero
status_t inode_put(inode_t *ino);


// hash lookup by (volume ptr + inode number)
status_t icache_lookup(volume_t *vol, uint32_t ino_num, inode_t **out);

// insert into hash table
status_t icache_insert(inode_t *ino);

// remove from hash table
status_t icache_remove(inode_t *ino);
