/*
 * File: fs_registry.h
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs/vfs_internals.h"
#include "dev/devicet.h"

#include "init.h"

#include <types.h>

typedef struct filesystem
{
    LIST_ENTRY_THIS();
    char *name;
    int (*kill_fs)();

    status_t (*probe)(device_t *dev);

    status_t (*mount)(device_t *dev, mountpoint_t *mnt);
    status_t (*umount)(device_t *dev, mountpoint_t *mnt);

    status_t (*lookup)(inode_t *dir, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt);

    status_t (*open)(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);
    status_t (*close)(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);
    status_t (*read)(vfs_node_t *node, void *buf, off_t offset, size_t len, ssize_t **len_out, device_t *dev, mountpoint_t *mnt);
    status_t (*write)(vfs_node_t *node, const void *buf, off_t offset, size_t len, ssize_t **len_out, device_t *dev, mountpoint_t *mnt);
    status_t (*read_dir)(vfs_node_t *dir, uint32_t index, vfs_dirent_t *out, device_t *dev, mountpoint_t *mnt);

    status_t (*mkdir)(vfs_node_t *dir, const char *name, uint32_t flags, device_t *dev, mountpoint_t *mnt);
    status_t (*unlink)(vfs_node_t *dir, const char *name, device_t *dev, mountpoint_t *mnt);

    status_t (*stat)(vfs_node_t *node, vfs_stat_t *out, device_t *dev, mountpoint_t *mnt);

    status_t (*ioctl)(vfs_node_t *dir, int op, void *arg, device_t *dev, mountpoint_t *mnt);

    status_t (*alloc_inode)(volume_t *vol, inode_t **out_ino); // fs allocates its own type
    status_t (*free_inode)(inode_t *ino);                      // fs frees its own type
} filesystem_t;

THIS_TO_TYPE_DEF(filesystem_t);

/// @brief Adds a filesystem driver to the filesystem registry list
/// @param[in] fs The filesystem driver to add
/// @return KERRNO_SUCCESSES on successes or a +kerrno on failure
status_t vfs_fs_register(filesystem_t *fs);

/// @brief Removes a filesystem driver from the filesystem registry list
/// @param[in] name The filesystem driver's name to remove
/// @return KERRNO_SUCCESSES on successes or a +kerrno on failure
status_t vfs_fs_unregister(const char *name);

/// @brief tries to lookup a filesystem driver form the given @p name
/// @param[in] name The filesystem driver's name to try to find
/// @param[out] out_fs The found filesystem driver on successes or NULL on failure
/// @return KERRNO_SUCCESSES on successes or a +kerrno on failure
status_t vfs_fs_find(const char *name, filesystem_t **out_fs);

// try each driver's probe(), return first match
/// @brief tries each filesystem driver's probe function and sets the first match
/// @param[in] dev The device that will be used by probe
/// @param[out] out_fs The found filesystem driver on successes or NULL on failure
/// @return KERRNO_SUCCESSES on successes or a +kerrno on failure
status_t vfs_fs_probe(device_t *dev, filesystem_t **out_fs);

#define VFS_FS_DRIVER(__fs_driver)              \
    static status_t __fs_driver##_init()        \
    {                                           \
        return vfs_fs_register(&(__fs_driver)); \
    }                                           \
    FS_INITCALL(__fs_driver##_init)

