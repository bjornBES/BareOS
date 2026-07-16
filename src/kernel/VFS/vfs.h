/*
 * File: vfs.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vfs_internal.h"
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <types.h>

#include "syscall/syscall.h"

#define VFS_FD_STDIN (fd_t)0
#define VFS_FD_STDOUT (fd_t)1
#define VFS_FD_STDERR (fd_t)2
#define VFS_FD_DEBUG (fd_t)3
#define VFS_FS_NEXT (fd_t)4
#define VFS_INVALID_FD (fd_t) -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// read only
#define VFS_O_RDONLY 0x0000

// write only
#define VFS_O_WRONLY 0x0001

// read + write
#define VFS_O_RDWR 0x0002

// create if not exists
#define VFS_O_CREAT 0x0004

// truncate to zero on open
#define VFS_O_TRUNC 0x0008

// writes always go to end
#define VFS_O_APPEND 0x0010

// fail if file already exists (with O_CREAT)
#define VFS_O_EXCL 0x0020

// don't follow symlinks
#define VFS_O_NOFOLLOW 0x0040

// fail if not a directory
#define VFS_O_DIRECTORY 0x0080

// owner read
#define VFS_S_IRUSR 0x0100

// owner write
#define VFS_S_IWUSR 0x0200

// owner execute
#define VFS_S_IXUSR 0x0400

// group read
#define VFS_S_IRGRP 0x0010

// group write
#define VFS_S_IWGRP 0x0020

// group execute
#define VFS_S_IXGRP 0x0040

// other read
#define VFS_S_IROTH 0x0001

// other write
#define VFS_S_IWOTH 0x0002

// other execute
#define VFS_S_IXOTH 0x0004

// common combos
#define VFS_S_IRWXU VFS_S_IRUSR | VFS_S_IWUSR | VFS_S_IXUSR,
#define VFS_S_IRWXG VFS_S_IRGRP | VFS_S_IWGRP | VFS_S_IXGRP,

// kernel functions
void vfs_init();

// public API / syscalls
int vfs_mount(const char *path, device_t *dev, int flags);
int vfs_unmount(const char *path, int flags);

fd_t vfs_open(const char *path, int flags, int mode);
int vfs_close(fd_t file);
int vfs_read(fd_t file, void *data, size_t size);
int vfs_write(fd_t file, const void *data, size_t size);
int vfs_seek(fd_t file, off_t offset, int whence);

int vfs_stat(const char *path, vfs_stat_t *out);
int vfs_fstat(fd_t file, vfs_stat_t *out);
int vfs_readdir(fd_t dir, vfs_dirent_t *out); // iterate directory
int vfs_mkdir(const char *path, int mode);
int vfs_unlink(const char *path);

void vfs_register_fs(filesystem_t *fs);