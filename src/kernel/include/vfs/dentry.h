/*
 * File: dentry.h
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

#define DENTRY_FLAG_NEGATIVE  (1 << 0)  // known not to exist
#define DENTRY_FLAG_MOUNTED   (1 << 1)  // something mounted here
#define DENTRY_FLAG_ROOT      (1 << 2)  // root of a mountpoint

// alloc, set name/inode/parent, refcount=1
status_t dentry_alloc(const char *name, inode_t *inode, dentry_t *parent, dentry_t **out);

// remove from cache, free struct
status_t dentry_free(dentry_t *d);

//increment refcount, return d
status_t dentry_get(dentry_t *d, dentry_t **out); 

//decrement refcount, free if zero
status_t dentry_put(dentry_t *d);


// hash lookup by (parent ino + name)
status_t dcache_lookup(dentry_t *parent, const char *name, dentry_t **out);

// insert into hash table
status_t dcache_insert(dentry_t *d);

// mark negative or remove on unlink
status_t dcache_invalidate(dentry_t *d);
