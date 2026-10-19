/*
 * File: mount.c
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/mount.h"
#include "vfs/dentry.h"
#include "vfs/inode.h"
#include "vfs/path.h"

#include "lists/list.h"

#include "config.h"

#include "memory.h"
#include "string.h"
#include "math.h"

#include "init.h"

#include "kerrno.h"

#define MODULE "vfs-mount"

spinlock_t mount_lock;

list_t mount_list;

status_t vfs_mount_init()
{
    list_initialize(&mount_list);
    return KERRNO_SUCCESSES;
}

POSTCORE_INITCALL(vfs_mount_init);

// alloc mountpoint, call fs->mount, insert tree
status_t mount_create(volume_t *vol, const char *path, uint32_t flags, mountpoint_t **out)
{
    ENTER_FUNC("%p, %p, 0x%x, %p", vol, path, flags, out);
    if (vol == NULL || path == NULL || out == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    const char *mount_path;
    path_split_prefix(path, NULL, &mount_path);

    // check not already mounted at this path
    if (mount_find_for_volume(path, vol, NULL) == KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_IN_USE, "path '%s' already has a mountpoint", path);
    }

    if (vol->fs->mount == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "%s has not set mount function", vol->fs->name);
    }

    mountpoint_t *mnt = kzmalloc(sizeof(mountpoint_t));
    if (mnt == NULL)
    {
        KERRNO_RETURN(KERRNO_OOM, "Out of memory");
    }

    strncpy(mnt->path, mount_path, sizeof(mnt->path) - 1);
    mnt->volume = vol;
    mnt->flags = flags;

    // call the driver — fills vol->sb with superblock data
    status_t ret = vol->fs->mount(vol->device, mnt);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        // trace_err(MODULE, "fs driver failed to mount '%s'", vol->volume_id);
        kfree(mnt);
        return ret;
    }

    inode_t *root_ino;
    dentry_t *root_dentry;
    trace_debug(MODULE, "vol->flags = 0x%x, vol->device = %p", vol->flags, vol->device);
    if (vol->device != NULL)
    {
        // allocate root inode — driver fills it in via alloc_inode
        ret = inode_alloc(vol, &root_ino);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            vol->fs->umount(vol->device, mnt);
            kfree(mnt);
            return ret;
        }

        if (vol->fs->lookup == NULL)
        {
            KERRNO_RETURN(KERRNO_BAD_VALUE, "%s has not set lookup function", vol->fs->name);
        }

        // ask driver to fill root inode (ino=0 or ino=2 for ext2 etc.)
        ret = vol->fs->lookup(NULL, "/", root_ino, vol->device, mnt);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            // trace_err(MODULE, "driver failed to provide root inode");
            inode_free(root_ino);
            vol->fs->umount(vol->device, mnt);
            kfree(mnt);
            return ret;
        }

        // allocate root dentry
        ret = dentry_alloc("/", root_ino, NULL, &root_dentry);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            inode_free(root_ino);
            vol->fs->umount(vol->device, mnt);
            kfree(mnt);
            return ret;
        }

        root_dentry->flags |= DENTRY_FLAG_ROOT;
        mnt->root_dentry = root_dentry;
        mnt->dentry = root_dentry;
    }

    // back pointer on volume
    vol->mountpoint = mnt;

    // insert into linked list + table
    ADD_THIS_TO_LIST(vol->mountpoints, mnt);

    if (vol->device != NULL)
    {
        ret = icache_insert(root_ino);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            inode_free(root_ino);
            vol->fs->umount(vol->device, mnt);
            kfree(mnt);
            return ret;
        }

        ret = dcache_insert(root_dentry);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            inode_free(root_ino);
            vol->fs->umount(vol->device, mnt);
            kfree(mnt);
            return ret;
        }
    }

    trace_info(MODULE, "mounted '%s' at '%s'", vol->volume_id, path);
    *out = mnt;
    return KERRNO_SUCCESSES;
}

// call fs->umount, remove from tree, free
status_t mount_destroy(mountpoint_t *mnt)
{
    if (mnt == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    // call driver umount — frees vol->sb
    status_t ret = mnt->volume->fs->umount(mnt->volume->device, mnt);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

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
    list_remove(&mnt->volume->mountpoints, &THIS_NODE(mnt));

    return kfree(mnt);
}

INTERNAL status_t mount_int_find(const char *mount_path, volume_t *volume, mountpoint_t **out)
{
    if (mount_path == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    mountpoint_t *best = NULL;
    size_t best_length = 0;
    size_t path_length = strlen(mount_path);

    list_node_t *node = volume->mountpoints.tail;
    while (node)
    {
        mountpoint_t *mnt = container_of(node, mountpoint_t, LIST_ENTRY_NAME);
        size_t length = strlen(mnt->path); // max(path_length, strlen(mnt->path));

        trace_debug(MODULE, "mnt->path = \"%s\", mount_path = \"%s\" length = %u", mnt->path, mount_path, length);

        if (strncmp(mnt->path, mount_path, length) == 0 && length > best_length)
        {
            best = mnt;
            best_length = length;
        }
        node = node->prev;
    }

    if (best == NULL)
    {
        return KERRNO_UNSUCCESS;
    }

    if (out != NULL)
    {
        *out = best;
    }
    return KERRNO_SUCCESSES;
}

status_t mount_find(const char *path, mountpoint_t **out)
{
    if (path == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    char volume_path[CONFIG_MAX_VOLUME_LENGTH];
    const char *mount_path;
    path_split_prefix(path, volume_path, &mount_path);
    // log_debug(MODULE, "path = %s { volume_path = %s, mount_path = %s}", path, volume_path, mount_path);
    volume_t *volume;
    status_t ret = volume_find(volume_path, &volume);
    // KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }

    return mount_int_find(mount_path, volume, out);
}

status_t mount_find_for_volume(const char *path, volume_t *vol, mountpoint_t **out)
{
    if (path == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    return mount_int_find(path, vol, out);
}

// return root dentry of this mountpoint
status_t mount_get_root(mountpoint_t *mnt, dentry_t **out)
{
    if (mnt == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    *out = mnt->root_dentry;
    return KERRNO_SUCCESSES;
}

// check if dentry has a mount on top of it
status_t mount_cross(dentry_t *dentry, mountpoint_t **out)
{
    if (dentry == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }
    if (!FLAG_IS_SET(dentry->flags, DENTRY_FLAG_MOUNTED))
    {
        return KERRNO_UNSUCCESS;
    }
    *out = dentry->mountpoint;
    return KERRNO_SUCCESSES;
}
