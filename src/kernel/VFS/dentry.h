/*
 * File: dentry.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vfs_internal.h"

// alloc, set name/inode/parent, refcount=1
dentry_t *dentry_alloc(const char *name, inode_t *inode, dentry_t *parent);

// remove from cache, free struct
void dentry_free(dentry_t *d);

//increment refcount, return d
dentry_t *dentry_get(dentry_t *d); 

//decrement refcount, free if zero
void dentry_put(dentry_t *d);


// hash lookup by (parent ino + name)
dentry_t *dcache_lookup(dentry_t *parent, const char *name);

// insert into hash table
void dcache_insert(dentry_t *d);

// mark negative or remove on unlink
void dcache_invalidate(dentry_t *d);

// zero the hash table
void dcache_init();
