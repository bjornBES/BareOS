/*
 * File: inode.c
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "inode.h"
#include "kernel/string.h"
#include "kernel/memory.h"
#include "debug/debug.h"

#define MODULE "VFS-INODE"

static inode_t *icache[ICACHE_SIZE];
static spinlock_t icache_lock;

static uint32_t icache_hash(volume_t *vol, uint32_t ino_num)
{
    // mix volume pointer + inode number
    uint32_t hash = (uint32_t)(uintptr_t)vol;
    hash = hash * 31 + ino_num;
    return hash % ICACHE_SIZE;
}

// call fs->alloc_inode, insert icache
inode_t *inode_alloc(volume_t *vol)
{
    log_debug(MODULE, "inode_alloc(%p)", vol);
    if (vol == NULL || vol->fs == NULL)
        return NULL;

    // driver allocates its own inode type
    inode_t *ino = vol->fs->alloc_inode(vol);
    if (ino == NULL)
        return NULL;

    ino->volume = vol;
    ino->fs = vol->fs;
    ino->refcount = 1;
    ino->hash_next = NULL;

    return ino;
}

// call fs->free_inode, remove from icache
void inode_free(inode_t *ino)
{
    log_debug(MODULE, "inode_free(%p)", ino);
    if (ino == NULL)
        return;

    icache_remove(ino);

    // driver frees its own type (fat_inode_t etc.)
    ino->fs->free_inode(ino);
}

// increment refcount
inode_t *inode_get(inode_t *ino)
{
    log_debug(MODULE, "inode_get(%p)", ino);
    if (ino == NULL)
        return NULL;
    ino->refcount++;
    return ino;
}

// decrement refcount, free if zero
void inode_put(inode_t *ino)
{
    log_debug(MODULE, "inode_put(%p)", ino);
    if (ino == NULL)
        return;
    ino->refcount--;
    if (ino->refcount == 0)
        inode_free(ino);
}

// hash lookup by (volume ptr + inode number)
inode_t *icache_lookup(volume_t *vol, uint32_t ino_num)
{
    log_debug(MODULE, "icache_lookup(%p, %u)", vol, ino_num);
    uint32_t slot = icache_hash(vol, ino_num);

    spinlock_acquire(&icache_lock);
    inode_t *ino = icache[slot];
    while (ino != NULL)
    {
        if (ino->volume == vol && ino->ino == ino_num)
        {
            inode_get(ino);
            spinlock_release(&icache_lock);
            return ino;
        }
        ino = ino->hash_next;
    }
    spinlock_release(&icache_lock);
    return NULL;
}

// insert into hash table
void icache_insert(inode_t *ino)
{
    log_debug(MODULE, "icache_insert(%p)", ino);
    if (ino == NULL)
        return;

    uint32_t slot = icache_hash(ino->volume, ino->ino);

    spinlock_acquire(&icache_lock);
    ino->hash_next = icache[slot];
    icache[slot] = ino;
    spinlock_release(&icache_lock);
}

// remove from hash table
void icache_remove(inode_t *ino)
{
    log_debug(MODULE, "icache_remove(%p)", ino);
    if (ino == NULL)
        return;

    uint32_t slot = icache_hash(ino->volume, ino->ino);

    spinlock_acquire(&icache_lock);
    inode_t *prev = NULL;
    inode_t *curr = icache[slot];
    while (curr != NULL)
    {
        if (curr == ino)
        {
            if (prev == NULL)
                icache[slot] = ino->hash_next;
            else
                prev->hash_next = ino->hash_next;
            ino->hash_next = NULL;
            break;
        }
        prev = curr;
        curr = curr->hash_next;
    }
    spinlock_release(&icache_lock);
}

// zero the hash table
void icache_init()
{
    memset(icache, 0, sizeof(icache));
}
