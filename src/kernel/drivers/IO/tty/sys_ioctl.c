/*
 * File: sys_ioctl.c
 * File Created: 21 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty_ioctl.h"
#include "tty.h"
#include "syscall/syscall.h"
#include "VFS/fd_table.h"
#include "task/process.h"
#include "mm/vmm/vmm.h"
#include <errno/errno.h>

#define MODULE "SYS-IOCTL"

int io_ioctl(fd_t fd, int op, void *argp)
{
    process_t *proc = process_get_current();
    vaddr_t argp_addr = (vaddr_t)argp;
    
    if (vma_inside(proc->vma, argp_addr) != RETURN_GOOD)
    {
        ERRNO_RETURN(EFAULT, "argp (%p) references an inaccessible memory area", argp_addr);
    }
    
    fd_entry_t *entry = fd_get_for_process(fd, &proc->fd_table);
    if (entry == NULL)
    {
        ERRNO_RETURN(EBADF, "fd (%i) is not a valid file descriptor", fd);
    }
    vfs_node_t *vNode = entry->node;
    mountpoint_t *mnt = vNode->mountpoint;
    device_t *dev = mnt->volume->device;
    if (dev->type != DEVICE_VIRTUAL)
    {
        ERRNO_RETURN(ENOTTY, "fd (%i) is not associated with a character special device (%s)", fd, dev->name);
    }
    return vNode->fs->ioctl(vNode, op, argp, dev, mnt);
}

SYSCALL_DEFINE3(io_ioctl, fd_t, int, void *);