/*
 * File: vfs.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs.h"
#include "vfs_internal.h"

#include "device/device.h"
#include "debug/debug.h"
#include "kernel.h"
#include "video/VGATextDevice.h"
#include "kernel/debug.h"

#include "libs/malloc.h"
#include "libs/string.h"
#include "libs/memory.h"
#include "libs/IO.h"

// VFS subsystems
#include "dentry.h"
#include "fd_table.h"
#include "fs_registry.h"
#include "inode.h"
#include "mount.h"
#include "path.h"
#include "volume.h"

#define MODULE "VFS"

int vfs_read_file(fd_t file, uint8_t *data, size_t size)
{
    vfs_node_t *node = fd_get(file);
    if (!node->opened)
    {
        log_crit("VFS", "File is not opened");
        return 0;
    }

    off_t offset = node->offset;
    device_t *dev = node->mountpoint->volume->device;
    size_t bytes = node->fs->read(node, data, offset, size, dev, node->mountpoint);
    return bytes;
}

int vfs_write(fd_t file, const void *vdata, size_t size)
{
    uint8_t *data = (uint8_t*)vdata;
    switch (file)
    {
    case VFS_INVALID_FD:
        log_debug("KERNEL", "why? just why?");
        __unreachable();
        return RETURN_ERROR;

    case VFS_FD_STDIN:
        return RETURN_FAILED;

    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
        {
            vga_put_char(data[i]);
        }
        return size;
    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
        {
            e9_putc(data[i]);
        }
        return size;

    default:
        return RETURN_ERROR;
    }
}
SYSCALL_DEFINE3(vfs_write, fd_t, void *, size_t)

int vfs_read(fd_t file, void *data, size_t size)
{
    switch (file)
    {
    case VFS_INVALID_FD:
        log_debug("KERNEL", "why? just why?");
        __unreachable();
        return RETURN_ERROR;

    case VFS_FD_STDIN:
        return RETURN_GOOD;

    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
    case VFS_FD_DEBUG:
        return RETURN_FAILED;

    default:
        return vfs_read_file(file, data, size);
    }
    return RETURN_ERROR;
}
SYSCALL_DEFINE3(vfs_read, fd_t, void *, size_t)

int vfs_seek(fd_t file, off_t offset, int whence)
{
    vfs_node_t *node = fd_get(file);
    if (whence == SEEK_SET)
    {
        node->offset = offset;
        return RETURN_GOOD;
    }
    // have not tested this it can be fucked fyi
    //
    // - BjornBEs 16-03-2026

    return RETURN_FAILED;
}

fd_t vfs_open(const char *path, int flags, int mode)
{
    vfs_node_t *node = NULL;
    if (path_lookup(path, &node) != RETURN_GOOD)
    {
        return VFS_INVALID_FD;
    }

    volume_t     *vol = node->mountpoint->volume;
    mountpoint_t *mnt = node->mountpoint;

    if (vol->fs->open(node, vol->device, mnt) != RETURN_GOOD)
    {
        free(node);
        return VFS_INVALID_FD;
    }

    node->opened = true;

    fd_t fd = fd_alloc(node);
    if (fd == VFS_INVALID_FD)
    {
        vol->fs->close(node, vol->device, mnt);
        free(node);
    }

    node->fs = vol->fs;

    return fd;
}

int vfs_close(fd_t fd)
{
    vfs_node_t *node = fd_get(fd);
    
    node->fs->close(node, node->mountpoint->volume->device, node->mountpoint);
    inode_free(node->inode);
    fd_release(fd);
    free(node);

    return RETURN_GOOD;
}

int vfs_readdir(fd_t dir, vfs_dirent_t *out)
{
/*     if (fd == VFS_INVALID_FD)
    {
        return -1;
    }

    if (current_fd != fd)
    {
        // init
        current_fd = fd;
        current_index = 0;
    }

    // loop based on the current_index
    vfs_node *node = fd_table[fd];
    int state = node->fs->read_dir(node, current_index, out, node->volume->dev, node->volume); */
    return RETURN_FAILED;
}

int vfs_stat(const char *path, vfs_stat_t *out)
{
    return RETURN_FAILED;
}

int vfs_fstat(fd_t file, vfs_stat_t *out)
{
    return RETURN_FAILED;
}

int vfs_mkdir(const char *path, int mode)
{
    return RETURN_FAILED;
}

int vfs_unlink(const char *path)
{
    return RETURN_FAILED;
}


int vfs_mount(const char *path, device_t *dev, int flags)
{
    log_debug(MODULE, "vfs_mount(%s (%p), %p, %u)", path, path, dev, flags);
    char volume_path[MAX_VOLUME_NAME];
    const char *mount_path;
    path_split_prefix(path, volume_path, &mount_path);
    log_debug(MODULE, "path = %s { volume_path = %s, mount_path = %s}", path, volume_path, mount_path);
    volume_t *volume = volume_register(volume_path, dev);
    if (volume == NULL)
    {
        log_crit(MODULE, "volume (%s) already in use", volume_path);
        return RETURN_FAILED;
    }

    filesystem_t *fs = volume->fs;
    if (fs == NULL)
    {
        log_crit(MODULE, "volume (%s) could not be probed", volume_path);
        volume_unregister(volume->volume_id);
        return RETURN_FAILED;
    }

    mountpoint_t *mount = mount_create(volume, mount_path, 0);
    if (mount == NULL)
    {
        log_crit(MODULE, "volume (%s) could not be mounted to (%s)", volume_path, mount_path);
        volume_unregister(volume->volume_id);
        return RETURN_FAILED;
    }
    return RETURN_GOOD;
}

int vfs_unmount(const char *path, int flags)
{
    return RETURN_FAILED;
}

void vfs_register_fs(filesystem_t *fs)
{
    fs_register(fs);
}

void vfs_init()
{
    mount_init();
    volume_init();
    // dcache_init();
    // icache_init();
    log_debug(MODULE, "done init");

    return;
}
