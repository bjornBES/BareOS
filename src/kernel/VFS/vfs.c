/*
 * File: vfs.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 May 2026 12:31:54
 * Modified By: BjornBEs
 * -----
 */

#include "vfs.h"
#include "device/device.h"
#include "debug/debug.h"
#include "kernel.h"
#include "video/VGATextDevice.h"
#include "kernel/debug.h"

#include "libs/malloc.h"
#include "libs/string.h"
#include "libs/memory.h"
#include "libs/IO.h"

#define MODULE "VFS"

#define MAX_OPEN_FILES 256

filesystem **filesystems;
int last_fs_id;

volume_point **volume_points;
int last_volume_id;

static vfs_node *fd_table[MAX_OPEN_FILES];

volume_point *vfs_check_volume_point(char *name)
{
    for (size_t i = 0; i < last_volume_id; i++)
    {
        volume_point *point = volume_points[i];
        if (!point || !point->name)
        {
            continue;
        }

        if (strcmp(name, point->name) == 0)
        {
            return volume_points[i];
        }
    }
    return NULL;
}

volume_point *vfs_path_resolve(char *path)
{
    // path structure is
    // /volume:/...

    /*
/part:/Home/file.txt
  ↑      ↑
  │      │
  │      └─────── handed to FAT32 fs driver as "Home/file.txt"
  └────────────── volume
    */

    char *vfs_path = path;
    int path_length = strlen(path);
    char *cut_path = malloc(path_length + 1);
    memset(cut_path, 0, path_length);
    int path_index = 1;

    while (!vfs_check_volume_point(cut_path))
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

    volume_point *volume = vfs_check_volume_point(vfs_path);
    free(cut_path);
    return volume;
}

volume_point **vfs_get_volume_points(int *count)
{
    *count = last_volume_id;
    return volume_points;
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
    device *dev = node->volume->dev;
    uint32_t bytes = node->fs->read(node, data, offset, size, dev, node->volume);
    return bytes;
}

int VFS_write(fd_t file, uint8_t *data, size_t size)
{
/*     if ((uint32_64)data < (uint32_64)KERNEL_VIRT_BASE)
    {
        log_debug(MODULE, "writing %u bytes from %p to %u", size, data, file);
    } */
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
SYSCALL_DEFINE3(VFS_write, fd_t, void *, size_t)

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
SYSCALL_DEFINE3(VFS_read, fd_t, void *, size_t)

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

            node->volume = vfs_path_resolve(path);
            if (node->volume == NULL)
            {
                free(node);
                return VFS_INVALID_FD;
            }

            node->fs = node->volume->fs;
            node->offset = 0;
            int volume_length = strlen(node->volume->name);
            strcpy(node->path, path + volume_length);

            node->fs->open(node, node->volume->dev, node->volume);

            fd_table[fd] = node;
            return fd;
        }
    }
    return VFS_INVALID_FD;
}

bool VFS_close(fd_t fd)
{
    vfs_node *node = fd_table[fd];
    node->fs->close(node, node->volume->dev, node->volume);

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
    int state = node->fs->read_dir(node, current_index, out, node->volume->dev, node->volume);
    return state;
}

bool VFS_mount(char *volume_path, device *dev)
{
    log_info(MODULE, "getting: %s probe %p, volume %p", filesystems[0]->name, filesystems[0]->probe, filesystems[0]->volume);
    volume_point *volume = vfs_check_volume_point(volume_path);
    if (volume != NULL)
    {
        log_crit(MODULE, "volume point (%s) already in use", volume_path);
        return false;
    }
    allocator_print_status();
    allocator_print_blocks();
    volume = (volume_point *)malloc(sizeof(volume_point));
    allocator_print_status();
    allocator_print_blocks();
    log_info(MODULE, "getting: %s probe %p, volume %p", filesystems[0]->name, filesystems[0]->probe, filesystems[0]->volume);
    memset(volume, 0, sizeof(volume_point));
    volume_points[last_volume_id] = volume;
    last_volume_id++;

    size_t path_length = strlen(volume_path) + 1;
    volume->name = malloc(path_length);
    if (!volume->name)
    {
        free(volume);
        return false;
    }
    memcpy(volume->name, name, path_length);
    volume->dev = dev;

    filesystem *fs = NULL;

    for (size_t i = 0; i < last_fs_id; i++)
    {
        fs = filesystems[i];
        log_info(MODULE, "getting: %s probe %p, volume %p", fs->name, fs->probe, fs->volume);
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
        free(volume);
        return false;
    }

    volume->fs = fs;

    if (fs->volume(dev, volume))
    {
        log_debug(MODULE, "mounted the device");
        return true;
    }

    free(volume);
    return false;
}

void VFS_register_fs(filesystem *fs)
{
    log_info(MODULE, "registering: %s probe %p, volume %p", fs->name, fs->probe, fs->volume);
    filesystems[last_fs_id] = fs;
    last_fs_id++;
}

void VFS_init()
{
    last_fs_id = 0;
    last_volume_id = 0;

    // syscall_register_handler(1, );

    filesystems = (filesystem **)calloc(4, sizeof(filesystem *));
    volume_points = (volume_point **)calloc(8, sizeof(volume_point *));
    log_debug(MODULE, "done init");

    return;
}
