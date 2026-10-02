/*
 * File: vfs.h
 * File Created: 26 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internals.h"

#include "dev/devicet.h"

#include <types.h>

status_t vfs_initialize();

// public API / syscalls
status_t vfs_mount(const char *label, device_t *dev, int flags);

status_t vfs_unmount(const char *label);

// vfs_node_t *vfs_create_device_node(const char *path, device_t *dev, int flags);

status_t vfs_lookup(const char *path, vfs_node_t **out);

status_t vfs_open(const char *path, int flags, int mode, vfs_node_t *node);
status_t vfs_close(vfs_node_t *node);
status_t vfs_read(vfs_node_t *node, void *buf, size_t len, size_t *out_read);
status_t vfs_write(vfs_node_t *node, const void *buf, size_t len, size_t *out_read);
status_t vfs_readdir(vfs_node_t *dir, off_t index, vfs_dirent_t *out);

status_t vfs_mkdir(const char *path, int mode);
status_t vfs_unlink(const char *path);

status_t vfs_stat(const char *path, vfs_stat_t *out);
status_t vfs_fstat(vfs_node_t *node, vfs_stat_t *out);

status_t vfs_seek(vfs_node_t *node, off_t offset, int whence);
