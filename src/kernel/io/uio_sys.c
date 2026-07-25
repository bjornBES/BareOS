/*
 * File: uio_sys.c
 * File Created: 21 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include <types.h>

#include "debug/debug.h"
#include "syscall/syscall.h"
#include "VFS/vfs.h"

typedef struct iovec
{
    void *iov_base;
    size_t iov_len;
} iovec_t;

ssize_t io_readv(fd_t fd, iovec_t *iov, int iovcnt)
{
    return 0;
}
SYSCALL_DEFINE3(io_readv, fd_t, iovec_t *, int)

ssize_t io_writev(fd_t fd, iovec_t *iov, int iovcnt)
{
    ssize_t ret = 0;
    for (int i = 0; i < iovcnt; i++)
    {
        ret += vfs_user_write(fd, iov[i].iov_base, iov[i].iov_len);
        log_debug(NO_MODULE, "ret = %u", ret);
    }
    // log_debug(NO_MODULE, "ret = %u", ret);
    return ret;
}
SYSCALL_DEFINE3(io_writev, fd_t, iovec_t *, int)

