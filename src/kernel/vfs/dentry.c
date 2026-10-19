/*
 * File: dentry.c
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/dentry.h"

#include "lists/hash.h"

#include "memory.h"
#include "string.h"

#include "init.h"

#include "kerrno.h"

#include "config.h"

#define MODULE "vfs-dentry"

spinlock_t dcache_lock;

dentry_t *dcache[CONFIG_DCACHE_SIZE];

status_t vfs_dentry_init()
{
    memset(dcache, 0, sizeof(dcache));
    return KERRNO_SUCCESSES;
}

POSTCORE_INITCALL(vfs_dentry_init);

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
    return hash % CONFIG_DCACHE_SIZE;
}

// alloc, set name/inode/parent, refcount=1
status_t dentry_alloc(const char *name, inode_t *inode, dentry_t *parent, dentry_t **out)
{
    dentry_t *d = kzmalloc(sizeof(dentry_t));
    if (d == NULL)
    {
        return KERRNO_OOM;
    }

    strncpy(d->name, name, CONFIG_MAX_PATH_LENGTH - 1);
    d->inode = inode;
    d->parent = parent;
    d->refcount = 1;
    d->hash_next = NULL; // not in dcache yet

    if (parent != NULL)
    {
        d->next = parent->children;
        parent->children = d;
    }

    *out = d;
    return KERRNO_SUCCESSES;
}

// remove from cache, free struct
status_t dentry_free(dentry_t *d)
{
    if (d == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

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
                {
                    d->parent->children = d->next;
                }
                else
                {
                    prev->next = d->next;
                }
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    return kfree(d);
}

// increment refcount, return d
status_t dentry_get(dentry_t *d, dentry_t **out)
{
    if (d == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    d->refcount++;
    if (out != NULL)
    {
        *out = d;
    }
    return KERRNO_SUCCESSES;
}

// decrement refcount, free if zero
status_t dentry_put(dentry_t *d)
{
    if (d == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    d->refcount--;
    if (d->refcount == 0)
    {
        status_t ret = dcache_invalidate(d);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
        ret = dentry_free(d);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
    }
    return KERRNO_SUCCESSES;
}

// hash lookup by (parent ino + name)
status_t dcache_lookup(dentry_t *parent, const char *name, dentry_t **out)
{
    uint32_t slot = dcache_hash(parent, name);

    spinlock_acquire(&dcache_lock);
    dentry_t *d = dcache[slot];
    while (d != NULL)
    {
        if (d->parent == parent && strcmp(d->name, name) == 0)
        {
            dentry_get(d, NULL);
            // log_debug(MODULE, "dcache_lookup is done -> %p", d);
            spinlock_release(&dcache_lock);
            *out = d;
            return KERRNO_SUCCESSES;
        }
        d = d->hash_next; // walk hash chain, not sibling chain
    }
    spinlock_release(&dcache_lock);
    // log_debug(MODULE, "dcache_lookup is done -> NULL");
    return KERRNO_UNSUCCESS;
}

// insert into hash table
status_t dcache_insert(dentry_t *d)
{
    uint32_t slot = dcache_hash(d->parent, d->name);

    spinlock_acquire(&dcache_lock);
    d->hash_next = dcache[slot]; // prepend to bucket
    dcache[slot] = d;
    spinlock_release(&dcache_lock);
    return KERRNO_SUCCESSES;
}

// mark negative or remove on unlink
status_t dcache_invalidate(dentry_t *d)
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
            {
                dcache[slot] = d->hash_next;
            }
            else
            {
                prev->hash_next = d->hash_next;
            }
            d->hash_next = NULL;
            break;
        }
        prev = curr;
        curr = curr->hash_next;
    }
    spinlock_release(&dcache_lock);
    return KERRNO_SUCCESSES;
}
