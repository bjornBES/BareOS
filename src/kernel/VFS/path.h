/*
 * File: path.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internal.h"

int path_has_volume(char *path);

int path_is_rooted(char *path);

// full resolver: prefix parse + segment walk
int path_lookup(const char *path, vfs_node_t **node_out);

// parse "vol:/rest" into parts
int path_split_prefix(const char *path, char *volume_id_out, const char **rel_out);

// consume one path component
int path_next_segment(const char *path, char *seg_out, const char **rest_out);

// dcache hit or fs->lookup + alloc
int path_resolve_segment(dentry_t *parent, const char *name, inode_t **inode_out);

int path_insert_volume(char *path, char *volume);

int path_combind(char *path, char *path2);
