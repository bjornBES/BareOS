/*
 * File: volume.c
 * File Created: 02 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/volume.h"
#include "vfs/fs_registry.h"
#include "vfs/path.h"

#include "sync/spinlock.h"
#include "lists/list.h"
#include "lists/hash.h"

#include "kerrno.h"
#include "init.h"

#include "memory.h"
#include "string.h"

#include <config.h>

#define MODULE "vfs-volume"

hash_map_t *volume_hash;
list_t *volume_registry;

status_t vfs_volume_init()
{
    volume_registry = kmalloc(sizeof(list_t));
    if (volume_registry == NULL)
    {
        KERRNO_RETURN(KERRNO_OOM, "kmalloc returned null");
    }
    list_initialize(volume_registry);
    status_t ret = hash_map_create(&volume_hash, CONFIG_INITIAL_FS_SIZE);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return KERRNO_SUCCESSES;
}

POSTCORE_INITCALL(vfs_volume_init);

status_t volume_do_register(const char *volume_id, device_t *dev, filesystem_t *fs, volume_t **out)
{
    ENTER_FUNC("\"%s\", %p, %p, %p", volume_id, dev, fs, out);
    if (hash_search(volume_hash, (char *)volume_id, NULL) == KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_IN_USE, "volume id \"%s\" is already in use", volume_id);
    }

    volume_t *vol = kzmalloc(sizeof(volume_t));
    size_t vol_len = strlen(volume_id);
    strncpy(vol->volume_id, volume_id, vol_len);
    vol->device = dev;
    vol->fs = fs;
    list_initialize(&vol->mountpoints);

    if (dev == NULL)
    {
        vol->flags |= VOLUME_VIRTUAL;
    }

    status_t ret = ADD_THIS_TO_LIST(*volume_registry, vol);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    ret = hash_insert(volume_hash, (char *)volume_id, vol);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    *out = vol;
    trace_debug(MODULE, "*out = %p", *out);
    return KERRNO_SUCCESSES;
}

status_t volume_register(const char *volume_id, device_t *dev, volume_t **out)
{
    filesystem_t *fs = NULL;
    if (dev == NULL)
    {
        status_t ret = vfs_fs_probe(dev, &fs);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
    }

    return volume_do_register(volume_id, dev, fs, out);
}

status_t volume_unregister(const char *volume_id)
{
    hash_node_t *node;
    if (hash_search(volume_hash, (char *)volume_id, &node) != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_NOT_FOUND, "volume id \"%s\" is already in use", volume_id);
    }

    volume_t *vol = node->value;

    status_t ret = list_remove(volume_registry, &THIS_NODE(vol));
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    ret = hash_delete(volume_hash, (char *)volume_id);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    ret = kfree(vol);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return KERRNO_SUCCESSES;
}

status_t volume_find(const char *volume_id, volume_t **out)
{
    ENTER_FUNC("%p, %p", volume_id, out);
    hash_node_t *node;
    if (hash_search(volume_hash, (char *)volume_id, &node) != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_NOT_FOUND, "volume id \"%s\" is not in use", volume_id);
        return KERRNO_NAME_NOT_FOUND;
    }
    volume_t *vol = node->value;
    *out = vol;
    return KERRNO_SUCCESSES;
}

status_t volume_from_prefix(const char *path, volume_t **out)
{
    char volume_name[CONFIG_MAX_VOLUME_LENGTH];
    status_t ret = path_split_prefix(path, volume_name, NULL);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        *out = NULL;
        return ret;
    }

    return volume_find(volume_name, out);
}

// return all registered volumes
status_t volume_list(volume_t **buf, size_t buf_size, size_t *out_size)
{
    int count = volume_registry->count < buf_size ? volume_registry->count : buf_size;
    list_node_t *node = volume_registry->tail;
    for (int i = 0; i < count; i++)
    {
        if (!node)
        {
            break;
        }
        buf[i] = to_volume_t(node);
        node = node->prev;
    }
    *out_size = count;
    return KERRNO_SUCCESSES;
}