/*
 * File: fs_registry.c
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fs_registry.h"
#include "debug/debug.h"
#include "libs/string.h"

// first ref to another file
#include "volume.h"

#define MODULE "VFS-FS"

int filesystem_count;
filesystem_t *registry[MAX_FILESYSTEMS];

void fs_registry_remove(filesystem_t *fs)
{
    for (size_t i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (registry[i] == fs)
        {
            fs->kill_fs();
            registry[i] = NULL;
            filesystem_count--;
        }
    }

    log_err(MODULE, "Couldn't remove fs(%s) from registry");
}

int fs_register(filesystem_t *fs)
{
    if (filesystem_count >= MAX_FILESYSTEMS)
    {
        log_err(MODULE, "filesystem registry is full");
        return RETURN_FAILED;
    }
    log_info(MODULE, "registering: %s probe %p", fs->name, fs->probe);
    for (size_t i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (registry[i] == NULL)
        {
            registry[i] = fs;
            break;
        }
    }
    filesystem_count++;
    return RETURN_GOOD;
}

int fs_unregister(const char *name)
{
    filesystem_t *fs = fs_find(name);
    if (fs == NULL)
        return RETURN_FAILED;

    // safety: reject if any mounted volume uses this driver
    volume_t *volumes[MAX_VOLUMES];
    int count = volume_list(volumes, MAX_VOLUMES);
    for (int i = 0; i < count; i++)
    {
        if (volumes[i]->fs == fs)
        {
            log_err(MODULE, "can't unregister '%s': volume '%s' still mounted",
                    name, volumes[i]->volume_id);
            return RETURN_FAILED;
        }
    }

    // remove from linked list
    fs_registry_remove(fs);

    return RETURN_GOOD;
}

filesystem_t *fs_find(const char *name)
{
    if (filesystem_count == 0)
    {
        return NULL;
    }

    for (size_t i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (registry[i] == NULL)
        {
            continue;
        }
        filesystem_t *entry = registry[i];
        if (strcmp(entry->name, name) == 0)
        {
            return entry;
        }
    }
    return NULL;
}

filesystem_t *fs_probe(device_t *dev)
{
    log_debug(MODULE, "fs_probe(%p)", dev);
    for (size_t i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (registry[i] == NULL)
        {
            continue;
        }
        filesystem_t *entry = registry[i];
        if (entry->probe == NULL)
        {
            log_warn(MODULE, "removing entry %s", entry->name);
            fs_unregister(entry->name);
            continue;
        }

        if (entry->probe(dev) == RETURN_GOOD)
        {
            return entry;
        }
    }
    return NULL;
}