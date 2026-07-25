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

typedef struct process process_t;

// kernel functions
void vfs_init();
void vfs_init_done();

// public API / syscalls
int vfs_mount(const char *path, device_t *dev, int flags);
int vfs_unmount(const char *path, int flags);
vfs_node_t *vfs_create_device_node(const char *path, device_t *dev, int flags);

fd_t vfs_open(const char *path, int flags, int mode);
fd_t vfs_user_do_open(const char *path, int flags, int mode, process_t *proc);
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

ssize_t vfs_user_write(fd_t file, void *data, size_t size);
ssize_t vfs_user_read(fd_t file, void *data, size_t size);