/*
 * File: inode.c
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/inode.h"

#include "kerrno.h"

#include "memory.h"
#include "string.h"

#include <config.h>

#define MODULE "vfs-inode"

inode_t *icache[CONFIG_ICACHE_SIZE];
spinlock_t icache_lock;

// zero the hash table
void icache_init()
{
    memset(icache, 0, sizeof(icache));
}

static uint32_t icache_hash(volume_t *vol, uint32_t ino_num)
{
    // mix volume pointer + inode number
    uint32_t hash = (uint32_t)(uintptr_t)vol;
    hash = hash * 31 + ino_num;
    return hash % CONFIG_ICACHE_SIZE;
}

// call fs->alloc_inode, insert icache
status_t inode_alloc(volume_t *vol, inode_t **out)
{
    if (vol == NULL || vol->fs == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    // driver allocates its own inode type
    inode_t *ino;
    status_t ret = vol->fs->alloc_inode(vol, &ino);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    ino->volume = vol;
    ino->fs = vol->fs;
    ino->refcount = 0;
    ino->hash_next = NULL;

    *out = ino;
    return KERRNO_SUCCESSES;
}

// call fs->free_inode, remove from icache
status_t inode_free(inode_t *ino)
{
    if (ino == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    status_t ret = icache_remove(ino);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    return ino->fs->free_inode(ino);
}

// increment refcount
status_t inode_get(inode_t *ino, inode_t **out)
{
    if (ino == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    ino->refcount++;
    if (out != NULL)
    {
        *out = ino;
    }
    return KERRNO_SUCCESSES;
}

// decrement refcount, free if zero
status_t inode_put(inode_t *ino)
{
    if (ino == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    ino->refcount--;
    if (ino->refcount == 0)
    {
        return inode_free(ino);
    }
    return KERRNO_SUCCESSES;
}

// hash lookup by (volume ptr + inode number)
status_t icache_lookup(volume_t *vol, uint32_t ino_num, inode_t **out)
{
    uint32_t slot = icache_hash(vol, ino_num);

    spinlock_acquire(&icache_lock);
    inode_t *ino = icache[slot];
    while (ino != NULL)
    {
        if (ino->volume == vol && ino->ino == ino_num)
        {
            inode_get(ino, NULL);
            spinlock_release(&icache_lock);
            *out = ino;
            return KERRNO_SUCCESSES;
        }
        ino = ino->hash_next;
    }
    spinlock_release(&icache_lock);
    return KERRNO_UNSUCCESS;
}

// insert into hash table
status_t icache_insert(inode_t *ino)
{
    if (ino == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    uint32_t slot = icache_hash(ino->volume, ino->ino);

    spinlock_acquire(&icache_lock);
    ino->hash_next = icache[slot];
    icache[slot] = ino;
    spinlock_release(&icache_lock);
    return KERRNO_SUCCESSES;
}

// remove from hash table
status_t icache_remove(inode_t *ino)
{
    if (ino == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    uint32_t slot = icache_hash(ino->volume, ino->ino);

    spinlock_acquire(&icache_lock);
    inode_t *prev = NULL;
    inode_t *curr = icache[slot];
    while (curr != NULL)
    {
        if (curr == ino)
        {
            if (prev == NULL)
            {
                icache[slot] = ino->hash_next;
            }
            else
            {
                prev->hash_next = ino->hash_next;
            }
            ino->hash_next = NULL;
            break;
        }
        prev = curr;
        curr = curr->hash_next;
    }
    spinlock_release(&icache_lock);
    return KERRNO_SUCCESSES;
}
