/*
 * File: fs_registry.c
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/fs_registry.h"
#include "vfs/vfs_internals.h"

#include "lists/list.h"
#include "lists/hash.h"

#include "memory.h"
#include "kerrno.h"
#include "config.h"

#define MODULE "vfs-fs"

hash_map_t *hash;
list_t *list;

status_t vfs_fs_init()
{
    list = kmalloc(sizeof(list_t));
    if (list == NULL)
    {
        KERRNO_RETURN(KERRNO_OOM, "kmalloc returned null");
    }
    list_initialize(list);
    status_t ret = hash_map_create(&hash, CONFIG_INITIAL_FS_SIZE);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return KERRNO_SUCCESSES;
}

POSTCORE_INITCALL(vfs_fs_init);

status_t vfs_fs_register(filesystem_t *fs)
{
    ENTER_FUNC("%p", fs);
    status_t ret = hash_insert(hash, fs->name, fs);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    trace_debug(MODULE, "hash add returned %u", ret);
    ret = ADD_THIS_TO_LIST(*list, fs);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    trace_debug(MODULE, "list add returned %u", ret);
    return KERRNO_SUCCESSES;
}

status_t vfs_fs_find(const char *name, filesystem_t **out_fs)
{
    hash_node_t hash_node;
    status_t ret = hash_search(hash, (char *)name, &hash_node);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    *out_fs = hash_node.value;
    return KERRNO_SUCCESSES;
}

status_t vfs_fs_probe(device_t *dev, filesystem_t **out_fs)
{
    if (list->has_initialize)
    {
        KERRNO_RETURN(KERRNO_NOT_INITIALIZED, "list has not been initialized");
    }

    list_node_t *node = list->tail;
    while (!node)
    {
        filesystem_t *fs = to_filesystem_t(node);
        if (fs->probe == NULL)
        {
            trace_err(MODULE, "%s doesn't have probe set", fs->name);
            continue;
        }
        
        if (fs->probe(dev) == KERRNO_SUCCESSES)
        {
            *out_fs = fs;
            return KERRNO_SUCCESSES;
        }
    }
    KERRNO_RETURN(KERRNO_UNSUCCESS, "Didn't find any file systems that could do that");
}
