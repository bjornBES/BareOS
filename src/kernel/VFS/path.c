/*
 * File: path.c
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "path.h"
#include "libs/string.h"
#include "libs/memory.h"
#include "libs/malloc.h"
#include "debug/debug.h"

#include "vfs_flags.h"
#include "volume.h"
#include "mount.h"
#include "dentry.h"
#include "inode.h"

#define MODULE "VFS-PATH"

// full resolver: prefix parse + segment walk
int path_lookup(const char *path, vfs_node_t **node_out)
{
    log_debug(MODULE, "path_lookup(%s(%p), %p)", path, path, node_out);
    if (path == NULL || node_out == NULL)
        return RETURN_FAILED;

    // 1. split prefix → vol_id + relative path
    char vol_id[64];
    const char *rel = NULL;

    if (path_split_prefix(path, vol_id, &rel) != RETURN_GOOD)
    {
        log_err(MODULE, "failed to split prefix from '%s'", path);
        return RETURN_FAILED;
    }

    // 2. find volume
    volume_t *vol = volume_find(vol_id);
    if (vol == NULL)
    {
        log_err(MODULE, "volume '%s' not found", vol_id);
        return RETURN_FAILED;
    }

    // 3. find mountpoint
    mountpoint_t *mnt = mount_find_for_volume(vol);
    if (mnt == NULL)
    {
        log_err(MODULE, "volume '%s' has no mountpoint", vol_id);
        return RETURN_FAILED;
    }

    // 4. start at root dentry
    dentry_t *parent = mnt->root_dentry;
    if (parent == NULL)
    {
        log_err(MODULE, "mountpoint has no root dentry");
        return RETURN_FAILED;
    }

    // 5. walk each segment
    char seg[MAX_FILE_NAME];
    const char *rest = rel;
    inode_t *ino = parent->inode;

    while (rest != NULL)
    {
        if (path_next_segment(rest, seg, &rest) != RETURN_GOOD)
        {
            log_err(MODULE, "failed to get next segment");
            return RETURN_FAILED;
        }

        // skip empty segments (double slash etc.)
        if (seg[0] == '\0')
            continue;

        inode_t *next_ino = NULL;
        if (path_resolve_segment(parent, seg, &next_ino) != RETURN_GOOD)
        {
            log_err(MODULE, "segment '%s' not found", seg);
            return RETURN_FAILED;
        }

        // move down the tree
        parent = dcache_lookup(parent, seg);
        ino = next_ino;
    }

    // 6. build the vfs_node
    vfs_node_t *node = malloc(sizeof(vfs_node_t));
    if (node == NULL)
    {
        return RETURN_FAILED;
    }

    memset(node, 0, sizeof(vfs_node_t));
    node->inode = ino;
    node->mountpoint = mnt;
    node->offset = 0;
    node->opened = false;

    *node_out = node;
    return RETURN_GOOD;
}

// parse "vol:/rest" into parts
int path_split_prefix(const char *path, char *volume_id_out, const char **rel_out)
{
    // path structure is
    // /VOLUME:/

    const char *cpath = path;
    const char *colon = strchr(cpath, ':'); // strchr uses cpath to find it
    cpath = path;                           // and so restore cpath from strchr
    if (colon == NULL)
    {
        if (rel_out != NULL)
        {
            *rel_out = NULL;
        }
        return RETURN_FAILED;
    }

    if (*cpath == '/')
    {
        cpath++;
    }

    if (rel_out != NULL)
    {
        *rel_out = colon + 1;
    }

    if (volume_id_out != NULL)
    {
        size_t volume_length = (size_t)(colon - cpath);
        volume_id_out = memcpy(volume_id_out, cpath, volume_length);
        volume_id_out[volume_length] = '\0';
    }
    return RETURN_GOOD;
}

// consume one path component
int path_next_segment(const char *path, char *seg_out, const char **rest_out)
{
    // path structure is
    // /VOLUME:/../../..

    // for path_next_segment
    // stage 1
    // strchr(':') returns !NULL
    // /VOLUME:/../../..
    // ^------^
    // SKIP VOLUME IF IT IS THERE
    // cpath = strchr(cpath, ':') + 1;
    // CAN BE DONE WITH path_split_prefix

    const char *cpath = path;
    const char *colon = strchr(cpath, ':');
    if (colon == NULL)
    {
        // there was not a ':' in the path
        cpath = path;
    }
    else
    {
        // skip the ':'
        cpath = colon + 1;
    }

    // stage 2
    // /VOLUME:/../../..
    //         ^--------....
    // if *cpath == '/'
    //  then cpath++
    //  else nop
    if (*cpath == '/')
    {
        cpath++;
    }

    // stage 3
    // next_seg_start = strchr(cpath, '/')
    // if next_seg_start == NULL
    //  then final segment...
    const char *curr_seg = cpath;
    const char *next_seg = strchr(cpath, '/');
    if (next_seg == NULL)
    {
        // last segment — no more '/' ahead
        if (seg_out != NULL)
        {
            size_t len = strlen(curr_seg);
            memcpy(seg_out, curr_seg, len);
            seg_out[len] = '\0';
        }
        if (rest_out != NULL)
        {
            *rest_out = NULL; // signal: nothing left
        }
        return RETURN_GOOD; // not a failure
    }

    // stage 4
    // endof_curr_seg = next_seg_start - 1
    // length_curr_seg = cpath - next_seg_start
    // *rest_out = next_seg_start
    // seg_out = memcpy(seg_out, cpath, length_curr_seg)
    size_t length_curr_seg = (size_t)(next_seg - curr_seg);
    if (rest_out != NULL)
    {
        *rest_out = next_seg + 1; // skip '/'
    }
    if (seg_out != NULL)
    {
        memcpy(seg_out, curr_seg, length_curr_seg);
        seg_out[length_curr_seg] = '\0';
    }
    return RETURN_GOOD;
}

// dcache hit or fs->lookup + alloc
int path_resolve_segment(dentry_t *parent, const char *name, inode_t **inode_out)
{
    log_debug(MODULE, "path_resolve_segment(%p, %s(%p), %p)", parent, name, name, inode_out);
    if (parent == NULL || name == NULL || inode_out == NULL)
        return RETURN_FAILED;

    // 1. try dcache first
    dentry_t *dentry = dcache_lookup(parent, name);
    if (dentry != NULL)
    {
        // hit — check it is not a negative dentry
        if (dentry->flags & DENTRY_FLAG_NEGATIVE)
        {
            dentry_put(dentry);
            return RETURN_FAILED;
        }
        *inode_out = inode_get(dentry->inode);
        dentry_put(dentry);
        return RETURN_GOOD;
    }

    // 2. miss — ask the fs driver
    volume_t *vol = parent->inode->volume;
    inode_t *ino = inode_alloc(vol);
    if (ino == NULL)
        return RETURN_FAILED;

    if (vol->fs->lookup(parent->inode, name, ino, vol->device, parent->inode->volume->mountpoint) != RETURN_GOOD)
    {
        // not found — insert negative dentry so we don't ask again
        dentry_t *neg = dentry_alloc(name, NULL, parent);
        if (neg != NULL)
        {
            neg->flags |= DENTRY_FLAG_NEGATIVE;
            dcache_insert(neg);
        }
        inode_free(ino);
        return RETURN_FAILED;
    }

    // 3. found — cache it
    dentry_t *new_dentry = dentry_alloc(name, ino, parent);
    if (new_dentry == NULL)
    {
        log_err(MODULE, "dentry_alloc(%s, %p, %p) returned NULL", name, ino, parent);
        inode_free(ino);
        return RETURN_FAILED;
    }

    icache_insert(ino);
    dcache_insert(new_dentry);

    *inode_out = inode_get(ino);
    return RETURN_GOOD;
}
