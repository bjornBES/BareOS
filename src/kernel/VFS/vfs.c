/*
 * File: vfs.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs.h"
#include "vfs_internal.h"
#include "vfs_flags.h"

#include "task/process_types.h"
#include "task/process.h"

#include "device/device.h"
#include "debug/debug.h"
#include "kernel.h"
#include "drivers/video/vga/vga_text_device.h"
#include "kernel/debug.h"

#include "kernel/string.h"
#include "kernel/memory.h"
#include "kernel/io.h"

// VFS subsystems
#include "dentry.h"
#include "fd_table.h"
#include "fs_registry.h"
#include "inode.h"
#include "mount.h"
#include "path.h"
#include "volume.h"

#define MODULE "VFS"

bool vfs_init_is_done = false;

static inline ssize_t vfs_read_file(vfs_node_t *node, uint8_t *data, size_t size)
{
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

static inline ssize_t vfs_write_file(vfs_node_t *node, uint8_t *data, size_t size)
{
    if (!node->opened)
    {
        log_crit("VFS", "File is not opened");
        return size;
    }

    off_t offset = node->offset;
    device_t *dev = node->mountpoint->volume->device;
    size_t bytes = node->fs->write(node, data, offset, size, dev, node->mountpoint);
    return bytes;
}

int vfs_write(fd_t file, const void *vdata, size_t size)
{
    log_debug(MODULE, "%p", &vdata);
    // ENTER_FUNC(MODULE, "%u, %p, %u", file, vdata, size);
    uint8_t *data = (uint8_t *)vdata;
    if (file == VFS_INVALID_FD)
    {
        log_debug("KERNEL", "why? just why?");
        __unreachable();
        return RETURN_ERROR;
    }
    switch (file)
    {
        case VFS_FD_STDIN :
            return RETURN_FAILED;

        case VFS_FD_DEBUG :
            {
                if (!vfs_init_is_done)
                {
                    for (size_t i = 0; i < size; i++)
                    {
                        debug_write_char(data[i]);
                    }
                    return size;
                }
                else
                {
                    goto _default;
                }
            }
        case VFS_FD_STDOUT :
        case VFS_FD_STDERR :
        default :
_default:
            {
                vfs_node_t *node = fd_get(file);
                return vfs_write_file(node, data, size);
            }
    }
}

int vfs_read(fd_t file, void *data, size_t size)
{
    switch (file)
    {
        case VFS_INVALID_FD :
        log_debug("KERNEL", "why? just why?");
        __unreachable();
        return RETURN_ERROR;
        
        case VFS_FD_STDOUT :
        case VFS_FD_STDERR :
        case VFS_FD_DEBUG :
        return RETURN_FAILED;
        
        case VFS_FD_STDIN :
        default :
        {
            vfs_node_t *node = fd_get(file);
            return vfs_read_file(node, data, size);
        }
    }
    return RETURN_ERROR;
}

ssize_t vfs_user_write(fd_t file, void *data, size_t size)
{
    process_t *proc = process_get_current();
    fd_entry_t *node = fd_get_for_process(file, &proc->fd_table);
    return vfs_write_file(node->node, data, size);
}
SYSCALL_DEFINE3_NON_PRINT(vfs_user_write, fd_t, void *, size_t)

ssize_t vfs_user_read(fd_t file, void *data, size_t size)
{
    process_t *proc = process_get_current();
    fd_entry_t *node = fd_get_for_process(file, &proc->fd_table);
    return vfs_read_file(node->node, data, size);
}

SYSCALL_DEFINE3_NON_PRINT(vfs_user_read, fd_t, void *, size_t)

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

vfs_node_t *vfs_open_internal(const char *path, int flags, int mode)
{
    ENTER_FUNC(MODULE, "%s, 0x%x, 0x%x", path, flags, mode);
    vfs_node_t *node = NULL;
    if (path_lookup(path, &node) != RETURN_GOOD)
    {
        return NULL;
    }

    volume_t *vol = node->mountpoint->volume;
    mountpoint_t *mnt = node->mountpoint;

    if (vol->fs->open(node, vol->device, mnt) != RETURN_GOOD)
    {
        free(node);
        return NULL;
    }

    node->flags = flags;
    node->opened = true;

    return node;
}

fd_t vfs_open(const char *path, int flags, int mode)
{
    ENTER_FUNC(MODULE, "%s, 0x%x, 0x%x", path, flags, mode);
    vfs_node_t *node = vfs_open_internal(path, flags, mode);

    volume_t *vol = node->mountpoint->volume;
    mountpoint_t *mnt = node->mountpoint;

    fd_t fd = fd_alloc(node);
    if (fd == VFS_INVALID_FD)
    {
        vol->fs->close(node, vol->device, mnt);
        free(node);
    }

    node->fs = vol->fs;

    return fd;
}

fd_t vfs_user_do_open(const char *path, int flags, int mode, process_t *proc)
{
    ENTER_FUNC(MODULE, "%s, 0x%x, 0x%x", path, flags, mode);
    vfs_node_t *node = vfs_open_internal(path, flags, mode);

    volume_t *vol = node->mountpoint->volume;
    mountpoint_t *mnt = node->mountpoint;

    fd_t fd = fd_alloc_for_process(node, &proc->fd_table);
    if (fd == VFS_INVALID_FD)
    {
        vol->fs->close(node, vol->device, mnt);
        free(node);
    }

    node->fs = vol->fs;

    return fd;
}

fd_t vfs_user_open(const char *path, int flags, int mode)
{
    ENTER_FUNC(MODULE, "%s, 0x%x, 0x%x", path, flags, mode);

    process_t *proc = process_get_current();

    return vfs_user_do_open(path, flags, mode, proc);
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
    vfs_node_t *node = NULL;
    if (path_lookup(path, &node) != RETURN_GOOD)
    {
        fd_t fd = vfs_open(path, VFS_O_RDWR, 0);
        int state = vfs_fstat(fd, out);
        vfs_close(fd);
        return state;
    }
    if (node->fs->stat(node, out, node->mountpoint->volume->device, node->mountpoint) != RETURN_GOOD)
    {
        return RETURN_FAILED;
    }
    return RETURN_GOOD;
}

int vfs_fstat(fd_t file, vfs_stat_t *out)
{
    vfs_node_t *node = fd_get(file);
    if (node->fs->stat(node, out, node->mountpoint->volume->device, node->mountpoint) != RETURN_GOOD)
    {
        return RETURN_FAILED;
    }
    return RETURN_GOOD;
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

vfs_node_t *vfs_create_device_node(const char *path, device_t *dev, int flags)
{
    log_debug(MODULE, "vfs_create_device_node(%s (%p), %p, %u, %x)", path, path, dev, flags);
    char volume_path[MAX_VOLUME_NAME];
    const char *mount_path = path;
    path_split_prefix(path, volume_path, &mount_path);
    log_debug(MODULE, "path = %s { volume_path = %s, mount_path = %s}", path, volume_path, mount_path);

    volume_t *volume = volume_find(volume_path);
    if (volume == NULL)
    {
        volume = volume_create_synthetic(volume_path, "devfs", dev);
    }
    if (volume == NULL)
    {
        log_crit(MODULE, "volume (%s) already in use", volume_path);
        return NULL;
    }

    filesystem_t *fs = volume->fs;
    if (fs == NULL)
    {
        log_crit(MODULE, "volume (%s) could not be probed", volume_path);
        volume_unregister(volume->volume_id);
        return NULL;
    }

    mountpoint_t *mount = mount_create(volume, mount_path, 0);
    if (mount == NULL)
    {
        log_crit(MODULE, "volume (%s) could not be mounted to (%s)", volume_path, mount_path);
        volume_unregister(volume->volume_id);
        return NULL;
    }
    return NULL;
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
    device_t *devfs_stub = device_create(DEVICE_VIRTUAL, DEVICE_FLAG_STUB);
    devfs_stub->class_name = "devfs";
    device_register(devfs_stub);
    vfs_create_device_node("/DEVFS:/dev", devfs_stub, DT_DIR);

    return;
}

void vfs_init_done()
{
    ENTER_FUNC(MODULE, "", "");
    vfs_init_is_done = true;
    return;
}
