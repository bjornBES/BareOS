/*
 * File: path.c
 * File Created: 17 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/path.h"
#include "vfs/volume.h"
#include "vfs/mount.h"
#include "vfs/dentry.h"
#include "vfs/inode.h"

#include "stdio.h"

#include "memory.h"
#include "string.h"

#include <config.h>

#define MODULE "vfs-path"

status_t path_has_volume(char *path)
{
    char *ret = NULL;
#if CONFIG_VOLUME_PREFIX != 0
    ret = strchr(path, CONFIG_VOLUME_PREFIX);
    if (!ret)
    {
        return KERRNO_UNSUCCESS;
    }
#endif
#if CONFIG_VOLUME_POSTFIX != 0
    ret = strchr(path, CONFIG_VOLUME_POSTFIX);
    if (!ret)
    {
        return KERRNO_UNSUCCESS;
    }
#endif
    return KERRNO_SUCCESSES;
}

status_t path_is_rooted(char *path)
{
    return *path == CONFIG_PATH_DIR_SEPARATOR;
}

status_t path_combind_volume_id(char *volume, char *path)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t path_combind(char *path, char *path2)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t path_split_prefix(const char *path, char *volume_id_out, const char **rel_out)
{
    char *cpath = (char *)path;
    if (path_has_volume(cpath) != KERRNO_SUCCESSES)
    {
        return KERRNO_SUCCESSES;
    }

#if CONFIG_VOLUME_POSTFIX != 0 || (CONFIG_VOLUME_POSTFIX != 0 && CONFIG_VOLUME_PREFIX != 0)
    const char *colon = strchr(cpath, CONFIG_VOLUME_POSTFIX); // strchr uses cpath to find it
#else
    const char *colon = strchr(cpath, CONFIG_VOLUME_PREFIX) + 1; // strchr uses cpath to find it
    colon = strchr(colon, CONFIG_PATH_DIR_SEPARATOR);
#endif
    cpath = (char *)path; // and so restore cpath from strchr
    if (colon == NULL)
    {
        if (rel_out != NULL)
        {
            *rel_out = NULL;
        }
        return KERRNO_UNSUCCESS;
    }

    if (path_is_rooted(cpath) == KERRNO_SUCCESSES)
    {
        cpath++;
    }

    if (rel_out != NULL)
    {
#if CONFIG_VOLUME_POSTFIX != 0
        *rel_out = colon + 1;
#else
        *rel_out = colon;
#endif
    }

    if (volume_id_out != NULL)
    {
        cpath++;
#if CONFIG_VOLUME_PREFIX != 0
        cpath++;
#endif
        size_t volume_length = (size_t)(colon - cpath);
        trace_debug(MODULE, "colon = %p (%s), cpath = %p (%s), colon - cpath = 0x%x", colon, colon, cpath, cpath, volume_length);
        if (volume_length > CONFIG_MAX_VOLUME_LENGTH)
        {
            KERRNO_RETURN(ENAMETOOLONG, "Path volume (%s) is too long.", path);
        }
        volume_id_out = memcpy(volume_id_out, cpath, volume_length);
        volume_id_out[volume_length] = '\0';
    }
    return KERRNO_SUCCESSES;
}

status_t path_next_segment(const char *path, char *seg_out, const char **rest_out)
{
    ENTER_FUNC("%p (%s), %p, %p", path, path, seg_out, rest_out);
    const char *cpath = path;
#if CONFIG_VOLUME_POSTFIX != 0 || (CONFIG_VOLUME_POSTFIX != 0 && CONFIG_VOLUME_PREFIX != 0)
    const char *colon = strchr(cpath, CONFIG_VOLUME_POSTFIX); // strchr uses cpath to find it
#else
    const char *colon = strchr(cpath, CONFIG_VOLUME_PREFIX); // strchr uses cpath to find it
    if (colon != NULL)
    {
        colon = strchr(colon, CONFIG_PATH_DIR_SEPARATOR);
    }
#endif
    if (colon == NULL)
    {
        cpath = path;
    }
    else
    {
#if CONFIG_VOLUME_POSTFIX != 0
        cpath = colon + 1;
#else
        cpath = colon;
#endif
    }

    // trace_debug(MODULE, "cpath = %p (%s)", cpath, cpath);
    if (*cpath == CONFIG_PATH_DIR_SEPARATOR)
    {
        cpath++;
    }

    const char *curr_seg = cpath;
    const char *next_seg = strchr(cpath, CONFIG_PATH_DIR_SEPARATOR);
    // trace_debug(MODULE, "curr_seg = %p, next_seg = %p", curr_seg, next_seg);
    if (next_seg == NULL)
    {
        size_t len = strlen(curr_seg);
        if (len > CONFIG_MAX_PATH_LENGTH)
        {
            KERRNO_RETURN(ENAMETOOLONG, "Path segment name (%s) is too long.", curr_seg);
        }
        // last segment — no more '/' ahead
        if (seg_out != NULL)
        {
            memcpy(seg_out, curr_seg, len);
            seg_out[len] = '\0';
        }
        if (rest_out != NULL)
        {
            *rest_out = NULL; // signal: nothing left
        }
        return KERRNO_SUCCESSES;
    }

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
    return KERRNO_SUCCESSES;
}

status_t path_resolve_segment(dentry_t *parent, const char *name, inode_t **inode_out)
{
    ENTER_FUNC("%p, %s, %p", parent, name, inode_out);
    if (parent == NULL || name == NULL || inode_out == NULL)
    {
        return KERRNO_UNSUCCESS;
    }

    // 1. try dcache first
    dentry_t *dentry;
    status_t ret = dcache_lookup(parent, name, &dentry);
    if (ret == KERRNO_SUCCESSES)
    {
        // hit — check it is not a negative dentry
        if (dentry->flags & DENTRY_FLAG_NEGATIVE)
        {
            return dentry_put(dentry);
        }
        ret = inode_get(dentry->inode, inode_out);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
        return dentry_put(dentry);
    }

    // 2. miss — ask the fs driver
    volume_t *vol = parent->inode->volume;
    inode_t *ino;
    ret = inode_alloc(vol, &ino);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    ret = vol->fs->lookup(parent->inode, name, ino, vol->device, parent->inode->volume->mountpoint);
    if (ret != KERRNO_SUCCESSES)
    {
        // not found — insert negative dentry so we don't ask again
        dentry_t *neg;
        ret = dentry_alloc(name, NULL, parent, &neg);
        if (ret != KERRNO_SUCCESSES)
        {
            neg->flags |= DENTRY_FLAG_NEGATIVE;
            ret = dcache_insert(neg);
            if (ret != KERRNO_SUCCESSES)
            {
                return ret;
            }
        }
        inode_free(ino);
        KERRNO_RETURN(ENOENT, "A component of pathname (%s) does not exist.", name);
    }

    // 3. found — cache it
    dentry_t *new_dentry;
    ret = dentry_alloc(name, ino, parent, &new_dentry);
    if (ret != KERRNO_SUCCESSES)
    {
        log_err(MODULE, "dentry_alloc(%s, %p, %p) returned NULL", name, ino, parent);
        inode_free(ino);
        KERRNO_RETURN(ENOENT, "A component of pathname (%s) does not exist.", name);
    }

    ret = icache_insert(ino);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    ret = dcache_insert(new_dentry);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    return inode_get(ino, inode_out);
}

status_t path_lookup(const char *path, vfs_node_t **node_out)
{
    if (path == NULL || node_out == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "bad arguments");
    }

    // 1. split prefix → vol_id + relative path
    char vol_id[64];
    const char *rel = NULL;

    status_t ret = path_split_prefix(path, vol_id, &rel);
    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "path_split_prefix returned %i", ret);
        return ret;
    }

    // 2. find volume
    volume_t *vol = NULL;
    ret = volume_find(vol_id, &vol);
    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "volume not found at %s", vol_id);
        return ret;
    }

    // 3. find mountpoint
    char seg[CONFIG_MAX_PATH_LENGTH];
    const char *rest = rel;
    // ret = path_next_segment(rel, seg, &rest);
    // trace_info(MODULE, "rel = %p, rest = %p", rel, rest);
    // trace_info(MODULE, "rel = %p, rest = %p, seg = %s", rel, rest, seg);
    mountpoint_t *mnt;
    ret = mount_find_for_volume(rel, vol, &mnt);
    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "Couldn't find a mountpoint at %s in volume %s", rel, vol->volume_id);
        return ret;
    }

    // 4. start at root dentry
    dentry_t *parent = mnt->root_dentry;
    if (parent == NULL)
    {
        KERRNO_RETURN(EBADF, "fd (%s) is not a valid open file descriptor.", path);
    }

    // 5. walk each segment
    inode_t *ino = parent->inode;

    while (rest != NULL)
    {
        ret = path_next_segment(rest, seg, &rest);
        if (ret != KERRNO_SUCCESSES)
        {
            trace_debug(MODULE, "path_next_segment returned %i", ret);
            return ret;
        }

        // skip empty segments (double slash etc.)
        if (seg[0] == '\0')
        {
            continue;
        }

        inode_t *next_ino = NULL;
        ret = path_resolve_segment(parent, seg, &next_ino);
        if (ret != KERRNO_SUCCESSES)
        {
            trace_debug(MODULE, "path_resolve_segment returned %i", ret);
            return ret;
        }
        trace_debug(MODULE, "next_ino = %p", next_ino);

        // move down the tree
        ret = dcache_lookup(parent, seg, &parent);
        if (ret != KERRNO_SUCCESSES)
        {
            trace_debug(MODULE, "dcache_lookup returned %i", ret);
            return ret;
        }
        ino = next_ino;
    }

    // 6. build the vfs_node
    vfs_node_t *node = kzmalloc(sizeof(vfs_node_t));
    if (node == NULL)
    {
        KERRNO_RETURN(KERRNO_OOM, "");
    }

    node->inode = ino;
    node->mountpoint = mnt;
    node->offset = 0;
    node->opened = false;
    node->fs = vol->fs;

    *node_out = node;
    return KERRNO_SUCCESSES;
}
