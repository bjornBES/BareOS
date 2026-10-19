/*
 * File: mount.h
 * File Created: 19 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs/vfs_internals.h"
#include "vfs/volume.h"

#include <types.h>

// alloc mountpoint, call fs->mount, insert tree
status_t mount_create(volume_t *vol, const char *path, uint32_t flags, mountpoint_t **out);

// call fs->umount, remove from tree, free
status_t mount_destroy(mountpoint_t *mnt);

// longest prefix match in mount tree
status_t mount_find(const char *path, mountpoint_t **out);

status_t mount_find_for_volume(volume_t *vol, mountpoint_t **out);

// return root dentry of this mountpoint
status_t mount_get_root(mountpoint_t *mnt, dentry_t **out);

// check if dentry has a mount on top of it
status_t mount_cross(dentry_t *dentry, mountpoint_t **out);
