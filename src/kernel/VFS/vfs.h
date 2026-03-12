/*
 * File: vfs.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <kernel_core/vfsBase.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#include "device/device.h"

#define MAX_FILE_NAME 64

struct __filesystem_t;

typedef struct
{
    char *path;
    device *dev;
    struct __filesystem_t *fs;
    void *fs_priv;
} mount_point;
typedef struct
{
    char name[256];
    uint32_t flags;
    size_t size;
    uint32_t inode;
    size_t offset;
    bool opened;
    struct __filesystem_t *fs;
    mount_point *mount;
    void *priv; // FS-private data
} vfs_node;

typedef struct
{
    char name[MAX_FILE_NAME]; /* entry name, null-terminated */
    uint32_t inode;           /* filesystem-local inode/cluster number */
    uint8_t type;             /* DT_REG, DT_DIR, DT_LNK, etc. */
} vfs_dirent;

typedef struct __filesystem_t
{
    char *name;
    bool (*probe)(device *dev);
    bool (*mount)(device *dev, mount_point *mnt);
    bool (*umount)(device *dev, mount_point *mnt);
    bool (*open)(vfs_node *node, device *dev, mount_point *mnt);
    bool (*close)(vfs_node *node, device *dev, mount_point *mnt);
    uint32_t (*read)(vfs_node *node, void *buffer, size_t offset, size_t length, device *dev, mount_point *mnt);

    bool (*read_dir)(vfs_node *dir, uint32_t index, vfs_dirent *out);
} filesystem;

int VFS_write(fd_t file, uint8_t *data, size_t size);
int VFS_read(fd_t file, uint8_t *data, size_t size);
fd_t VFS_open(char *path);
bool VFS_close(fd_t file);
bool VFS_mount(char *path, device *dev);
void VFS_register_fs(filesystem *fs);

void VFS_init();
