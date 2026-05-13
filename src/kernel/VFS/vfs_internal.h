/*
 * File: vfs_internal.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vfs_config.h"
#include "kernel.h"

#include "device/device.h"

struct filesystem;

typedef struct vfs_dirent
{
    char name[MAX_FILE_NAME]; // entry name, null-terminated
    uint32_t inode;           // filesystem-local inode/cluster number
    uint32_t type;             // DT_REG, DT_DIR, DT_LNK, etc.
} vfs_dirent_t;

struct mountpoint;

typedef struct volume
{
    char volume_id[MAX_VOLUME_NAME]; // unique name: "system", "boot", "sda1"
    struct filesystem *fs;           //
    device_t *device;                // backing block device (NULL for virtual fs)
    void *sb;                        // filesystem superblock once loaded
    uint32_t flags;                  // VOL_READONLY, VOL_VIRTUAL, VOL_REMOVABLE
    uint32_t refcount;               // how many mounts reference this
    struct mountpoint *mountpoint;
    struct volume *next; // linked list in volume registry
} volume_t;

typedef struct inode
{
    uint32_t ino; // inode number, unique per volume
    uint32_t type; // DT_REG, DT_DIR, DT_LNK
    uint32_t flags;
    size_t size;
    uint32_t uid, gid;
    uint64_t created;
    uint64_t modified;
    uint32_t refcount;
    volume_t *volume;
    struct filesystem *fs;
    struct inode *hash_next; // icache hash chain
} inode_t;

typedef struct dentry
{
    char name[MAX_FILE_NAME];
    inode_t *inode;
    struct dentry *parent;
    struct dentry *children;       // first child
    struct dentry *next;           // next sibling
    struct dentry *hash_next;      // next in dcache hash bucket
    struct mountpoint *mountpoint; // if this dentry is a mountpoint
    uint32_t flags;
    uint32_t refcount;
} dentry_t;

typedef struct mountpoint
{
    char path[256];            // "/", "/system", "/dev"
    volume_t *volume;          // what's mounted here
    dentry_t *dentry;          // the dentry this mount covers
    dentry_t *root_dentry;     // root dentry of the mounted volume
    uint32_t flags;            // inherited or override mount flags
    struct mountpoint *parent; // mount this is mounted on top of
    struct mountpoint *next;   // linked list
} mountpoint_t;

typedef struct vfs_node
{
    inode_t *inode;
    uint32_t flags;
    size_t size;
    off_t offset;
    bool opened;
    struct filesystem *fs;
    mountpoint_t *mountpoint;
} vfs_node_t;

typedef struct vfs_stat
{
    uint32_t inode;
    uint32_t type;
    size_t size;
    uint32_t flags;
    uint32_t uid;
    uint32_t gid;
    uint64_t created; // unix timestamp or tick count
    uint64_t modified;
} vfs_stat_t;

typedef struct filesystem
{
    char *name;
    int (*kill_fs)();

    int (*probe)(device_t *dev);
    int (*mount)(device_t *dev, mountpoint_t *mnt);
    int (*umount)(device_t *dev, mountpoint_t *mnt);
    int (*lookup)(inode_t *dir, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt);
    int (*open)(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);
    int (*close)(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);
    size_t (*read)(vfs_node_t *node, void *buf, off_t offset, size_t len, device_t *dev, mountpoint_t *mnt);
    size_t (*write)(vfs_node_t *node, const void *buf, off_t offset, size_t len, device_t *dev, mountpoint_t *mnt);
    int (*read_dir)(vfs_node_t *dir, uint32_t index, vfs_dirent_t *out, device_t *dev, mountpoint_t *mnt);
    int (*stat)(vfs_node_t *node, vfs_stat_t *out, device_t *dev, mountpoint_t *mnt);
    int (*mkdir)(vfs_node_t *dir, const char *name, uint32_t flags, device_t *dev, mountpoint_t *mnt);
    int (*unlink)(vfs_node_t *dir, const char *name, device_t *dev, mountpoint_t *mnt);

    inode_t *(*alloc_inode)(volume_t *vol); // fs allocates its own type
    void (*free_inode)(inode_t *ino);       // fs frees its own type
} filesystem_t;
