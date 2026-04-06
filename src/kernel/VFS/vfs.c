/*
 * File: vfs.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs.h"
#include "device/device.h"
#include "debug/debug.h"
#include "kernel.h"
#include "video/VGATextDevice.h"
#include "syscall/syscall.h"
#include "arch/x86/e9.h"

#include "libs/malloc.h"
#include "libs/string.h"
#include "libs/memory.h"
#include "libs/IO.h"

#define MODULE "VFS"

#define MAX_OPEN_FILES 256

filesystem **filesystems;
int last_fs_id;

mount_point **mount_points;
int last_mount_id;

static vfs_node *fd_table[MAX_OPEN_FILES];

mount_point *vfs_check_mount_point(char *path)
{
    for (size_t i = 0; i < last_mount_id; i++)
    {
        mount_point *point = mount_points[i];
        if (!point || !point->path)
        {
            continue;
        }

        if (strcmp(path, point->path) == 0)
        {
            return mount_points[i];
        }
    }
    return NULL;
}

mount_point *vfs_path_resolve(char *path)
{
    // path structure is
    // /device/partition/...

    /*
/sata0/Home/file.txt
  ↑      ↑
  │      │
  │      └─────── handed to FAT32 fs driver as "Home/file.txt"
  └────────────── looked up in mount table → fat32_mount_t*
    */

    char *vfs_path = path;
    int path_length = strlen(path);
    char *cut_path = malloc(path_length + 1);
    memset(cut_path, 0, path_length);
    int path_index = 1;

    while (!vfs_check_mount_point(cut_path))
    {
        cut_path = memcpy(cut_path, vfs_path, path_index);
        path_index++;
    }
    vfs_path = cut_path;

    // check for first char
    if (*vfs_path != '/')
    {
        // error if not
        log_err(MODULE, "Path ('%s') can't be resolved", path);
    }

    mount_point *mount = vfs_check_mount_point(vfs_path);
    free(cut_path);
    return mount;
}

mount_point **vfs_get_mount_points(int *count)
{
    *count = last_mount_id;
    return mount_points;
}

int VFS_read_file(fd_t file, uint8_t *data, size_t size)
{
    vfs_node *node = fd_table[file];
    if (!node->opened)
    {
        log_crit("VFS", "File is not opened");
        return 0;
    }

    uint32_t offset = node->offset;
    device *dev = node->mount->dev;
    uint32_t bytes = node->fs->read(node, data, offset, size, dev, node->mount);
    return bytes;
}

int VFS_write(fd_t file, uint8_t *data, size_t size)
{
    switch (file)
    {
    case VFS_INVALID_FD:
        log_debug("KERNEL", "why? just why?");
        __unreachable();
        return -1;
        
    case VFS_FD_STDIN:
        return 0;

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
        return -1;
    }
}
SYSCALL_DEFINE3(VFS_write, int, fd_t, void*, size_t)

int VFS_read(fd_t file, void *data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;

    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
    case VFS_FD_DEBUG:
        return 0;

    default:
        return VFS_read_file(file, data, size);
    }
    return 0;
}

int VFS_seek(fd_t file, int offset)
{
    // have not tested this it can be fucked fyi
    //
    // - BjornBEs 16-03-2026
    vfs_node *node = fd_table[file];
    node->offset = offset;
    return RETURN_GOOD;
}

fd_t VFS_open(char *path)
{
    for (fd_t fd = VFS_FS_NEXT; fd < MAX_OPEN_FILES; fd++)
    {
        if (fd_table[fd] == NULL)
        {
            vfs_node *node = malloc(sizeof(vfs_node));
            if (node == NULL)
            {
                return VFS_INVALID_FD;
            }

            node->mount = vfs_path_resolve(path);
            if (node->mount == NULL)
            {
                free(node);
                return VFS_INVALID_FD;
            }

            node->fs = node->mount->fs;
            node->offset = 0;
            int mount_length = strlen(node->mount->path);
            strcpy(node->name, path + mount_length);

            node->fs->open(node, node->mount->dev, node->mount);

            fd_table[fd] = node;
            return fd;
        }
    }
    return VFS_INVALID_FD;
}

bool VFS_close(fd_t fd)
{
    vfs_node *node = fd_table[fd];
    node->fs->close(node, node->mount->dev, node->mount);

    free(node);
    fd_table[fd] = NULL;
    return true;
}

fd_t current_fd = VFS_INVALID_FD;
int current_index;
int VFS_read_dir(fd_t fd, vfs_dirent *out)
{
    if (fd == VFS_INVALID_FD)
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
    int state = node->fs->read_dir(node, current_index, out, node->mount->dev, node->mount);
    return state;
}

bool VFS_mount(char *path, device *dev)
{
    mount_point *mount = vfs_check_mount_point(path);
    if (mount != NULL)
    {
        log_crit(MODULE, "mount point (%s) already in use", path);
        return false;
    }

    mount = (mount_point *)malloc(sizeof(mount_point));
    memset(mount, 0, sizeof(mount_point));
    mount_points[last_mount_id] = mount;
    last_mount_id++;

    size_t path_length = strlen(path) + 1;
    mount->path = malloc(path_length);
    if (!mount->path)
    {
        free(mount);
        return false;
    }
    memcpy(mount->path, path, path_length);
    mount->dev = dev;

    filesystem *fs = NULL;

    for (size_t i = 0; i < last_fs_id; i++)
    {
        fs = filesystems[i];
        if (fs->probe(dev))
        {
            break;
        }
        else
        {
            fs = NULL;
        }
    }

    if (fs == NULL)
    {
        log_crit(MODULE, "Can't find a filesystem");
        free(mount);
        return false;
    }

    mount->fs = fs;

    if (fs->mount(dev, mount))
    {
        log_debug(MODULE, "mounted the device");
        return true;
    }

    free(mount);
    return false;
}

void VFS_register_fs(filesystem *fs)
{
    filesystems[last_fs_id] = fs;
    last_fs_id++;
}

void VFS_init()
{
    last_fs_id = 0;
    last_mount_id = 0;

    // syscall_register_handler(1, );

    filesystems = (filesystem **)calloc(4, sizeof(filesystem *));
    mount_points = (mount_point **)calloc(8, sizeof(mount_point *));
    log_debug(MODULE, "done init");

    return;
}
