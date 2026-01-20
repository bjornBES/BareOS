#pragma once

typedef int fd_t;

#define VFS_FD_STDIN (fd_t) 0
#define VFS_FD_STDOUT (fd_t) 1
#define VFS_FD_STDERR (fd_t) 2
#define VFS_FD_DEBUG (fd_t) 3

#define VFS_INVALID_FD (fd_t) -1