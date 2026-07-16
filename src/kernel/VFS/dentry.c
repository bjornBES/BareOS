/*
 * File: dentry.c
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "dentry.h"
#include "vfs_config.h"
#include "debug/debug.h"
#include "kernel/memory.h"
#include "kernel/string.h"

#define MODULE "VFS-DENTRY"

spinlock_t dcache_lock;
dentry_t *dcache[DCACHE_SIZE];

// hash by parent inode number + name
uint32_t dcache_hash(dentry_t *parent, const char *name)
{
    const char *cname = name;
    uint32_t hash = parent ? parent->inode->ino : 0;
    while (*name)
    {
        hash = hash * 31 + (uint8_t)*name++;
    }
    name = cname;
    log_debug(MODULE, "%u = dcache_hash(%p, %s(%p))", hash % DCACHE_SIZE, parent, name, name);
    return hash % DCACHE_SIZE;
}

// alloc, set name/inode/parent, refcount=1
dentry_t *dentry_alloc(const char *name, inode_t *inode, dentry_t *parent)
{
    log_debug(MODULE, "dentry_alloc(%s(%p), %p, %p)", name, name, inode, parent);
    dentry_t *d = malloc(sizeof(dentry_t));
    if (d == NULL)
        return NULL;

    memset(d, 0, sizeof(dentry_t));
    strncpy(d->name, name, MAX_FILE_NAME - 1);
    d->inode = inode;
    d->parent = parent;
    d->refcount = 1;
    d->hash_next = NULL; // not in dcache yet

    if (parent != NULL)
    {
        d->next = parent->children;
        parent->children = d;
    }

    return d;
}

// remove from cache, free struct
void dentry_free(dentry_t *d)
{
    log_debug(MODULE, "dentry_free(%p)", d);
    if (d == NULL)
        return;

    // unlink from parent's children list
    if (d->parent != NULL)
    {
        dentry_t *prev = NULL;
        dentry_t *curr = d->parent->children;
        while (curr != NULL)
        {
            if (curr == d)
            {
                if (prev == NULL)
                    d->parent->children = d->next;
                else
                    prev->next = d->next;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    free(d);
}

// increment refcount, return d
dentry_t *dentry_get(dentry_t *d)
{
    log_debug(MODULE, "dentry_get(%p)", d);
    if (d == NULL)
        return NULL;
    d->refcount++;
    return d;
}

// decrement refcount, free if zero
void dentry_put(dentry_t *d)
{
    log_debug(MODULE, "dentry_put(%p)", d);
    if (d == NULL)
        return;
    d->refcount--;
    if (d->refcount == 0)
    {
        dcache_invalidate(d);
        dentry_free(d);
    }
}

// hash lookup by (parent ino + name)
dentry_t *dcache_lookup(dentry_t *parent, const char *name)
{
    log_debug(MODULE, "dcache_lookup(%p, %s(%p))", parent, name, name);
    uint32_t slot = dcache_hash(parent, name);

    spinlock_acquire(&dcache_lock);
    dentry_t *d = dcache[slot];
    while (d != NULL)
    {
        if (d->parent == parent && strcmp(d->name, name) == 0)
        {
            dentry_get(d);
            spinlock_release(&dcache_lock);
            return d;
        }
        d = d->hash_next; // walk hash chain, not sibling chain
    }
    spinlock_release(&dcache_lock);
    return NULL;
}

// insert into hash table
void dcache_insert(dentry_t *d)
{
    log_debug(MODULE, "dcache_insert(%p)", d);
    uint32_t slot = dcache_hash(d->parent, d->name);

    spinlock_acquire(&dcache_lock);
    d->hash_next = dcache[slot]; // prepend to bucket
    dcache[slot] = d;
    spinlock_release(&dcache_lock);
}

// mark negative or remove on unlink
void dcache_invalidate(dentry_t *d)
{
    uint32_t slot = dcache_hash(d->parent, d->name);

    spinlock_acquire(&dcache_lock);
    dentry_t *prev = NULL;
    dentry_t *curr = dcache[slot];
    while (curr != NULL)
    {
        if (curr == d)
        {
            if (prev == NULL)
                dcache[slot] = d->hash_next;
            else
                prev->hash_next = d->hash_next;
            d->hash_next = NULL;
            break;
        }
        prev = curr;
        curr = curr->hash_next;
    }
    spinlock_release(&dcache_lock);
}

// zero the hash table
void dcache_init()
{
    memset(dcache, 0, sizeof(dcache));
}
