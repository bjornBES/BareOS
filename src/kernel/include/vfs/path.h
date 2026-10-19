/*
 * File: path.h
 * File Created: 17 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internals.h"

#include <types.h>

status_t path_combind_volume_id(char *volume, char *path);

status_t path_combind(char *path, char *path2);

status_t path_split_prefix(const char *path, char *volume_id_out, const char **rel_out);

status_t path_next_segment(const char *path, char *seg_out, const char **rest_out);

status_t path_has_volume(char *path);

status_t path_is_rooted(char *path);

status_t path_lookup(const char *path, vfs_node_t **node_out);
status_t path_resolve_segment(dentry_t *parent, const char *name, inode_t **inode_out);

