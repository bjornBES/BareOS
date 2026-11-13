/*
 * File: vfs_sys.c
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/vfs.h"

#include "syscall/syscall.h"

#include "debug/debug.h"

#include "process/process.h"

#define MODULE "sys-vfs"

status_t vfs_user_write(intptr_t *out_val, fd_t fd, void *buf, size_t size)
{
    ENTER_FUNC("%p, 0x%x, %p, %u", out_val, fd, buf, size);

    process_t *proc;
    process_get_current(&proc);

    fd_entry_t *entry = proc->fd_table.entries[fd];
    if (!entry)
    {
        KERRNO_RETURN(EEXIST, "FD doesn't exist");
    }
    
    ssize_t byte_wrote = 0;
    status_t ret = vfs_write(entry->node, buf, entry->offset, size, &byte_wrote);
    if (ret == KERRNO_BAD_VALUE)
    {
        KERRNO_RETURN(EINVAL, "");
    }

    *((ssize_t *)out_val) = byte_wrote;
    RETURN_SUCCESSES();
}

SYSCALL_DEFINE3(vfs_user_write, fd_t, void *, size_t);
