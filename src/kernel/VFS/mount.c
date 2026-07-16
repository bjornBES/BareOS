/*
 * File: mount.c
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mount.h"
#include "vfs_flags.h"
#include "vfs_config.h"

#include "mm/allocator/memory_allocator.h"

#include "kernel/string.h"
#include "kernel/memory.h"
#include "debug/debug.h"

#include "math.h"
#include "dentry.h"
#include "inode.h"
#include "volume.h"

#define MODULE "VFS-MOUNT"

spinlock_t mount_lock;

mountpoint_t *mount_list;
mountpoint_t *mount_table[MAX_VOLUMES] = {0};
int mount_count = 0;

// alloc mountpoint, call fs->mount, insert tree
mountpoint_t *mount_create(volume_t *vol, const char *path, uint32_t flags)
{
    log_debug(MODULE, "mount_create(%p, %s(%p), %x)", vol, path, path, flags);
    if (vol == NULL || path == NULL)
    {
        return NULL;
    }

    if (mount_count >= MAX_MOUNTS)
    {
        log_err(MODULE, "mount table full");
        return NULL;
    }

    // check not already mounted at this path
    if (mount_find(path) != NULL)
    {
        log_err(MODULE, "path '%s' already has a mountpoint", path);
        return NULL;
    }

    mountpoint_t *mnt = malloc(sizeof(mountpoint_t));
    if (mnt == NULL)
    {
        return NULL;
    }

    memset(mnt, 0, sizeof(mountpoint_t));
    strncpy(mnt->path, path, sizeof(mnt->path) - 1);
    mnt->volume = vol;
    mnt->flags = flags;

    if (vol->fs->mount == NULL)
    {
        log_err(MODULE, "%s mount function is NULL", vol->fs->name);
    }

    // call the driver — fills vol->sb with superblock data
    if (vol->fs->mount(vol->device, mnt) != RETURN_GOOD)
    {
        log_err(MODULE, "fs driver failed to mount '%s'", vol->volume_id);
        free(mnt);
        return NULL;
    }

    // allocate root inode — driver fills it in via alloc_inode
    inode_t *root_ino = inode_alloc(vol);
    if (root_ino == NULL)
    {
        vol->fs->umount(vol->device, mnt);
        free(mnt);
        return NULL;
    }

    if (vol->fs->lookup == NULL)
    {
        log_err(MODULE, "%s lookup function is NULL", vol->fs->name);
    }
    // ask driver to fill root inode (ino=0 or ino=2 for ext2 etc.)
    if (vol->fs->lookup(NULL, "/", root_ino, vol->device, mnt) != RETURN_GOOD)
    {
        log_err(MODULE, "driver failed to provide root inode");
        inode_free(root_ino);
        vol->fs->umount(vol->device, mnt);
        free(mnt);
        return NULL;
    }

    // allocate root dentry
    dentry_t *root_dentry = dentry_alloc("/", root_ino, NULL);
    if (root_dentry == NULL)
    {
        inode_free(root_ino);
        vol->fs->umount(vol->device, mnt);
        free(mnt);
        return NULL;
    }

    root_dentry->flags |= DENTRY_FLAG_ROOT;
    mnt->root_dentry = root_dentry;
    mnt->dentry = root_dentry;

    // back pointer on volume
    vol->mountpoint = mnt;

    // insert into linked list + table
    spinlock_acquire(&mount_lock);
    mnt->next = mount_list;
    mount_list = mnt;
    mount_table[mount_count++] = mnt;
    spinlock_release(&mount_lock);

    icache_insert(root_ino);
    dcache_insert(root_dentry);

    log_info(MODULE, "mounted '%s' at '%s'", vol->volume_id, path);
    return mnt;
}

// call fs->umount, remove from tree, free
void mount_destroy(mountpoint_t *mnt)
{
    if (mnt == NULL)
        return;

    // call driver umount — frees vol->sb
    mnt->volume->fs->umount(mnt->volume->device, mnt);

    // clear back pointer on volume
    mnt->volume->mountpoint = NULL;

    // free root dentry + inode
    if (mnt->root_dentry != NULL)
    {
        dcache_invalidate(mnt->root_dentry);
        inode_put(mnt->root_dentry->inode);
        dentry_free(mnt->root_dentry);
    }

    // remove from linked list
    spinlock_acquire(&mount_lock);
    mountpoint_t *prev = NULL;
    mountpoint_t *curr = mount_list;
    while (curr != NULL)
    {
        if (curr == mnt)
        {
            if (prev == NULL)
                mount_list = mnt->next;
            else
                prev->next = mnt->next;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // remove from table — swap with last
    for (int i = 0; i < mount_count; i++)
    {
        if (mount_table[i] == mnt)
        {
            mount_table[i] = mount_table[--mount_count];
            mount_table[mount_count] = NULL;
            break;
        }
    }
    spinlock_release(&mount_lock);

    free(mnt);
}

// longest prefix match in mount tree
mountpoint_t *mount_find(const char *path)
{
    log_debug(MODULE, "mount_find(%s(%p))", path, path);
    if (path == NULL)
        return NULL;

    spinlock_acquire(&mount_lock);

    mountpoint_t *best = NULL;
    size_t best_length = 0;
    size_t path_length = strlen(path);

    for (int i = 0; i < mount_count; i++)
    {
        mountpoint_t *mnt = mount_table[i];
        size_t length = max(path_length, strlen(mnt->path));

        if (strncmp(mnt->path, path, length) == 0 && length > best_length)
        {
            best = mnt;
            best_length = length;
        }
    }

    spinlock_release(&mount_lock);
    return best;
}

mountpoint_t *mount_find_for_volume(volume_t *vol)
{
    log_debug(MODULE, "mount_find_for_volume(%p)");
    if (vol == NULL)
        return NULL;

    // fastest path — use the back pointer
    return vol->mountpoint;
}

// return root dentry of this mountpoint
dentry_t *mount_get_root(mountpoint_t *mnt)
{
    if (mnt == NULL)
        return NULL;
    return mnt->root_dentry;
}

// check if dentry has a mount on top of it
mountpoint_t *mount_cross(dentry_t *dentry)
{
    if (dentry == NULL)
        return NULL;
    if (!(dentry->flags & DENTRY_FLAG_MOUNTED))
        return NULL;
    return dentry->mountpoint;
}

void mount_init()
{
    memset(mount_table, 0, sizeof(mount_table));
    mount_list = NULL;
    mount_count = 0;
    log_debug(MODULE, "mount init done");
}