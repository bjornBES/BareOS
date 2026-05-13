/*
 * File: mount.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internal.h"

// alloc mountpoint, call fs->mount, insert tree
mountpoint_t *mount_create(volume_t *vol, const char *path, uint32_t flags);

// call fs->umount, remove from tree, free
void mount_destroy(mountpoint_t *mnt);

// longest prefix match in mount tree
mountpoint_t *mount_find(const char *path);

mountpoint_t *mount_find_for_volume(volume_t *vol);

// return root dentry of this mountpoint
dentry_t *mount_get_root(mountpoint_t *mnt);

// check if dentry has a mount on top of it
mountpoint_t *mount_cross(dentry_t *dentry);

void mount_init();
