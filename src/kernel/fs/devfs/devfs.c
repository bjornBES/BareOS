/*
 * File: devfs.c
 * File Created: 16 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "devfs.h"

#include "VFS/vfs.h"
#include "VFS/vfs_flags.h"

#include "errno/errno.h"

#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel.h"

#define MODULE "DEVFS"

typedef struct
{
    inode_t base;  // must be first
    device_t *dev; // which device this inode represents
} devfs_inode_t;

ssize_t devfs_read(vfs_node_t *node, void *buffer, off_t offset, size_t size, device_t *dev, mountpoint_t *mnt);
ssize_t devfs_write(vfs_node_t *node, const void *buffer, off_t offset, size_t size, device_t *dev, mountpoint_t *mnt);
int devfs_lookup(inode_t *parent, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt);
int devfs_open(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);
int devfs_close(vfs_node_t *node, device_t *dev, mountpoint_t *mnt);

int devfs_ioctl(vfs_node_t *node, int op, void *arg, device_t *dev, mountpoint_t *mnt);

inode_t *devfs_alloc_inode(volume_t *vol);
void devfs_free_inode(inode_t *ino);
int devfs_mount(device_t *dev, mountpoint_t *mnt);
int devfs_probe(device_t *dev);

// fat driver
ssize_t devfs_read(vfs_node_t *node, void *buffer, off_t offset, size_t size, device_t *dev, mountpoint_t *mnt)
{
    // log_debug(MODULE, "devfs_read(%p, %p, %u, %u, %p, %p)", node, buffer, offset, size, dev, mnt);
    devfs_inode_t *dev_ino = (devfs_inode_t *)node->inode;
    return device_read(dev_ino->dev, buffer, offset, size);
}

ssize_t devfs_write(vfs_node_t *node, const void *buffer, off_t offset, size_t size, device_t *dev, mountpoint_t *mnt)
{
    ENTER_FUNC(NO_MODULE, "%p, %p, %u, %u, %p, %p", node, buffer, offset, size, dev, mnt);
    devfs_inode_t *dev_ino = (devfs_inode_t *)node->inode;
    return device_write(dev_ino->dev, (void *)buffer, offset, size);
}

int devfs_ioctl(vfs_node_t *node, int op, void *arg, device_t *dev, mountpoint_t *mnt)
{
    // log_debug(MODULE, "devfs_write(%p, %p, %u, %u, %p, %p)", node, buffer, offset, size, dev, mnt);
    devfs_inode_t *dev_ino = (devfs_inode_t *)node->inode;
    return device_ioctl(dev_ino->dev, op, arg);
}

int devfs_lookup(inode_t *parent, const char *name, inode_t *out, device_t *dev, mountpoint_t *mnt)
{
    log_debug(MODULE, "fat_lookup(%p, %s(%p), %p, %p, %p)", parent, name, name, out, dev, mnt);

    // VFS asking for root
    if (parent == NULL && strcmp(name, "/") == 0)
    {
        devfs_inode_t *root = (devfs_inode_t *)devfs_alloc_inode(mnt->volume);
        root->base.type = DT_DIR;
        root->base.flags = 1;
        root->dev = NULL; // root dir has no backing device
        memcpy(out, root, sizeof(inode_t));
        return RETURN_GOOD;
    }
    
    if (parent && strcmp(name, "dev") == 0)
    {
        devfs_inode_t *root = (devfs_inode_t *)devfs_alloc_inode(mnt->volume);
        root->base.type = DT_DIR;
        root->base.flags = 1;
        root->dev = NULL; // root dir has no backing device
        memcpy(out, root, sizeof(inode_t));
        return RETURN_GOOD;
    }
    // ignore dev — devfs doesn't use it
    device_t *found = device_get_by_name(name);
    if (!found)
    {
        return -ENOENT;
    }

    devfs_inode_t *dev_ino = (devfs_inode_t *)devfs_alloc_inode(mnt->volume);
    dev_ino->dev = found;
    dev_ino->base.type = DT_CHR;
    memcpy(out, dev_ino, sizeof(inode_t));
    return RETURN_GOOD;
}

int devfs_readdir(vfs_node_t *dir, uint32_t index, vfs_dirent_t *out, device_t *dev, mountpoint_t *mnt)
{
    devfs_inode_t *dev_ino = (devfs_inode_t *)dir->inode;
    // walk device registry by index
    device_t *d = device_get_by_id(dev_ino->dev->type, index);
    if (!d)
    {
        return -1; // end of directory
    }
    strncpy(out->name, d->name, DEVICE_NAME_MAX);
    out->type = DT_CHR;
    return 0;
}

int devfs_open(vfs_node_t *node, device_t *dev, mountpoint_t *mnt)
{
    if (!node)
    {
        return RETURN_FAILED;
    }

    return RETURN_GOOD;
}

int devfs_close(vfs_node_t *node, device_t *dev, mountpoint_t *mnt)
{
    if (!node)
    {
        return RETURN_FAILED;
    }

    node->opened = false;
    node->offset = 0;
    node->inode = 0;
    node->size = 0;
    return RETURN_GOOD;
}

inode_t *devfs_alloc_inode(volume_t *vol)
{
    devfs_inode_t *ino = malloc(sizeof(devfs_inode_t));
    memset(ino, 0, sizeof(devfs_inode_t));
    ino->base.volume = vol;
    ino->base.fs = vol->fs;
    return &ino->base; // VFS only sees the base
}

void devfs_free_inode(inode_t *ino)
{
    devfs_inode_t *dev_ino = (devfs_inode_t *)ino;
    free(dev_ino);
}

int devfs_mount(device_t *dev, mountpoint_t *mnt)
{
    mnt->volume->sb = NULL;
    return RETURN_GOOD;
}

int devfs_probe(device_t *dev)
{
    if (!FLAG_IS_SET(dev->flags, DEVICE_FLAG_DEV))
    {
        return RETURN_FAILED;
    }

    return RETURN_GOOD;
}

void devfs_create_entry(device_t *dev, int flags)
{
    char path[MAX_PATH_SIZE];
    int count = snprintf(path, sizeof(path), "/DEVFS:/dev/%s", dev->name);
    path[count] = '\0';

    /* vfs_node_t *node = */ vfs_create_device_node(path, dev, flags);
    vfs_open(path, 0, 0);
}

static filesystem_t devfs_driver;

void devfs_init()
{
    devfs_driver.name = "devfs";
    devfs_driver.probe = devfs_probe;
    devfs_driver.mount = devfs_mount;
    devfs_driver.alloc_inode = devfs_alloc_inode;
    devfs_driver.free_inode = devfs_free_inode;
    devfs_driver.lookup = devfs_lookup;
    devfs_driver.open = devfs_open;
    devfs_driver.close = devfs_close;
    devfs_driver.ioctl = devfs_ioctl;
    devfs_driver.read = devfs_read;
    devfs_driver.write = devfs_write;
    vfs_register_fs(&devfs_driver);
}
