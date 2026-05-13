/*
 * File: inode.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internal.h"

// call fs->alloc_inode, insert icache
inode_t *inode_alloc(volume_t *vol);

// call fs->free_inode, remove from icache
void inode_free(inode_t *ino);

// increment refcount
inode_t *inode_get(inode_t *ino);

// decrement refcount, free if zero
void inode_put(inode_t *ino);


// hash lookup by (volume ptr + inode number)
inode_t *icache_lookup(volume_t *vol, uint32_t ino_num);

// insert into hash table
void icache_insert(inode_t *ino);

// remove from hash table
void icache_remove(inode_t *ino);

// zero the hash table
void icache_init();