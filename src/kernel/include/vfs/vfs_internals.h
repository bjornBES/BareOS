/*
 * File: vfs_internals.h
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/devicet.h"

#include "lists/list_node.h"

#include <config.h>
#include <types.h>

typedef struct volume volume_t;
typedef struct filesystem filesystem_t;
typedef struct mountpoint mountpoint_t;

typedef struct vfs_dirent
{
    char name[CONFIG_MAX_PATH_LENGTH]; // entry name, null-terminated
    uint32_t inode;                    // filesystem-local inode/cluster number
    uint32_t type;                     // DT_REG, DT_DIR, DT_LNK, etc.
} vfs_dirent_t;

typedef struct inode
{
    LIST_ENTRY_THIS();
    ino_t ino;     // inode number, unique per volume
    uint32_t type; // DT_REG, DT_DIR, DT_LNK
    uint32_t flags;
    size_t size;
    uid_t uid;
    gid_t gid;
    timespec_t created;
    timespec_t modified;
    uint32_t refcount;
    volume_t *volume;
    filesystem_t *fs;
    struct inode *hash_next; // icache hash chain
} inode_t;

typedef struct dentry
{
    LIST_ENTRY_THIS();
    char name[CONFIG_MAX_PATH_LENGTH];
    inode_t *inode;
    struct dentry *parent;
    struct dentry *children;  // first child
    struct dentry *next;      // next sibling
    struct dentry *hash_next; // next in dcache hash bucket
    mountpoint_t *mountpoint; // if this dentry is a mountpoint
    uint32_t flags;
    uint32_t refcount;
} dentry_t;

typedef struct mountpoint
{
    LIST_ENTRY_THIS();
    char label[CONFIG_MAX_MOUNTPOINT_LENGTH]; // "/", "/system", "/dev"
    volume_t *volume;                         // what's mounted here
    dentry_t *dentry;                         // the dentry this mount covers
    dentry_t *root_dentry;                    // root dentry of the mounted volume
    uint32_t flags;                           // inherited or override mount flags
    mountpoint_t *parent;                     // mount this is mounted on top of
    mountpoint_t *next;                       // linked list
} mountpoint_t;

typedef struct vfs_node
{
    inode_t *inode;
    uint32_t flags;
    size_t size;
    off_t offset;
    bool opened;
    filesystem_t *fs;
    mountpoint_t *mountpoint;
} vfs_node_t;

typedef struct vfs_stat
{
    inode_t *inode;

    uint32_t type;

    size_t size;

    uint32_t flags;

    uid_t uid;
    gid_t gid;

    blksize_t st_blksize;
    blkcnt_t st_blocks;

    timespec_t created; // unix timestamp or tick count
    timespec_t accessed;
    timespec_t modified;
} vfs_stat_t;

