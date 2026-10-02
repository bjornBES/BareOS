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

    if (hash_search(volume_hash, volume_id, NULL) == KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_IN_USE, "volume id \"%s\" is already in use", volume_id);
    }

    volume_t *vol = kmalloc(sizeof(volume_t));
    log_debug(MODULE, "volume allocate = %p", vol);
    memset(vol, 0, sizeof(volume_t));
    strncpy(vol->volume_id, volume_id, CONFIG_MAX_VOLUME_LENGTH - 1);
    vol->device = dev;
    vol->fs = fs;

    if (dev == NULL)
    {
        log_debug(MODULE, "volume is virtual");
        vol->flags |= VOLUME_VIRTUAL;
    }

    log_debug(MODULE, "insert volume");
    ADD_THIS_TO_LIST((*volume_list), vol);

    *out = vol;
    return KERRNO_SUCCESSES;
}
