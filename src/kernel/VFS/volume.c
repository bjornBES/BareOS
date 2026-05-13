/*
 * File: volume.c
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "volume.h"
#include "volume_flags.h"

#include "debug/debug.h"

#include "libs/string.h"
#include "libs/memory.h"
#include "libs/malloc.h"

#include "fs_registry.h"
#include "path.h"

#define MODULE "VFS-VOLUME"

spinlock_t volume_lock;

volume_t *volume_registry;
volume_t *volume_table[MAX_VOLUMES] = {0};
int volume_count = 0;

// probe fs drivers, alloc + insert volume
volume_t *volume_register(const char *volume_id, device_t *dev)
{
    log_debug(MODULE, "volume_register(%s(%p), %p)", volume_id, volume_id, dev);
    filesystem_t *fs = fs_probe(dev);
    if (fs == NULL && dev != NULL)
    {
        log_err(MODULE, "no filesystem found on device");
        return NULL;
    }

    log_debug(MODULE, "spinlock");
    spinlock_acquire(&volume_lock);

    if (volume_count >= MAX_VOLUMES)
    {
        log_err(MODULE, "volume table full");
        spinlock_release(&volume_lock);
        return NULL;
    }

    if (volume_registry != NULL)
    {
        volume_t *existing = volume_registry;
        while (existing != NULL)
        {
            if (strcmp(existing->volume_id, volume_id) == 0)
            {
                log_err(MODULE, "volume '%s' already registered", volume_id);
                spinlock_release(&volume_lock);
                return NULL;
            }
            existing = existing->next;
        }
    }

    volume_t *vol = malloc(sizeof(volume_t));
    log_debug(MODULE, "volume allocate = %p", vol);
    memset(vol, 0, sizeof(volume_t));
    strncpy(vol->volume_id, volume_id, MAX_VOLUME_NAME - 1);
    vol->device = dev;
    vol->fs = fs;
    
    if (dev == NULL)
    {
        log_debug(MODULE, "volume is virtual");
        vol->flags |= VOLUME_VIRTUAL;
    }
    
    log_debug(MODULE, "insert volume");
    vol->next = volume_registry;
    volume_registry = vol;
    volume_table[volume_count++] = vol;

    spinlock_release(&volume_lock);
    return vol;
}

// remove from registry, free
void volume_unregister(const char *volume_id)
{
    spinlock_acquire(&volume_lock);
    // remove from linked list
    volume_t *prev = NULL;
    volume_t *vol = volume_registry;
    while (vol != NULL)
    {
        if (strcmp(vol->volume_id, volume_id) == 0)
        {
            if (prev == NULL)
            {
                volume_registry = vol->next;
            }
            else
            {
                prev->next = vol->next;
            }
            break;
        }
        prev = vol;
        vol = vol->next;
    }

    if (vol == NULL)
    {
        spinlock_release(&volume_lock);
        return; // not found
    }

    for (int i = 0; i < volume_count; i++)
    {
        if (volume_table[i] == vol)
        {
            volume_table[i] = volume_table[--volume_count];
            volume_table[volume_count] = NULL;
            break;
        }
    }

    free(vol);
    spinlock_release(&volume_lock);
}

// lookup by id string
volume_t *volume_find(const char *volume_id)
{
    log_debug(MODULE, "volume_find(%s(%p))", volume_id, volume_id);
    spinlock_acquire(&volume_lock);
    volume_t *vol = volume_registry;
    while (vol != NULL)
    {
        if (strcmp(vol->volume_id, volume_id) == 0)
            break;
        vol = vol->next;
    }
    spinlock_release(&volume_lock);
    return vol;
}

// parse "vol_id:/" prefix, return volume
volume_t *volume_from_prefix(const char *path)
{
    char volume_name[MAX_VOLUME_NAME];
    if (path_split_prefix(path, volume_name, NULL) == RETURN_FAILED)
    {
        return NULL;
    }

    return volume_find(volume_name);
}

// return all registered volumes
int volume_list(volume_t **buf, int buf_size)
{
    spinlock_acquire(&volume_lock);
    int count = volume_count < buf_size ? volume_count : buf_size;
    for (int i = 0; i < count; i++)
        buf[i] = volume_table[i];
    spinlock_release(&volume_lock);
    return count;
}

void volume_init()
{
    volume_registry = NULL;
    volume_count = 0;
}