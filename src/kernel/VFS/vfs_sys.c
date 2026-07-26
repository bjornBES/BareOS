/*
 * File: vfs_sys.c
 * File Created: 25 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs_sys.h"
#include "stat.h"
#include "vfs_internal.h"
#include "vfs.h"
#include "vfs_types.h"
#include "errno/errno.h"
#include "kernel/memory.h"

#include "task/threading/scheduling/scheduler.h"

int vfs_user_stat(char *path, stat_t *statbuf)
{
    ENTER_FUNC(NO_MODULE, "%s, %p", path, statbuf);
    
    thread_t *current_thread = scheduler_get_current();
    ctx_dump(&current_thread->ctx);

    vfs_stat_t vstat;
    vfs_node_t vnode;
    int state = vfs_do_stat(path, &vstat, &vnode);
    if (state != RETURN_GOOD)
    {
        return state;
    }

    log_debug(NO_MODULE, "statbuf = %p", statbuf);
    log_debug(NO_MODULE, "vnode = %p", &vnode);
    log_debug(NO_MODULE, "vnode.mountpoint = %p", vnode.mountpoint);
    log_debug(NO_MODULE, "vnode.mountpoint->volume = %p", vnode.mountpoint->volume);
    log_debug(NO_MODULE, "vnode.mountpoint->volume->device = %p", vnode.mountpoint->volume->device);
    log_debug(NO_MODULE, "vnode.inode = %p", vnode.inode);

    stat_t kstat;
    memset(&kstat, 0, sizeof(stat_t));

    kstat.st_atim = vstat.accessed;
    kstat.st_mtim = vstat.modified;
    kstat.st_ctim = vstat.created;
    kstat.st_dev = vnode.mountpoint->volume->device->device_id;
    kstat.st_ino = vnode.inode->ino;
    kstat.st_mode = S_IFREG | S_IRWXU;
    kstat.st_blksize = vstat.st_blksize;
    kstat.st_blocks = vstat.st_blocks;

    copy_to_user(statbuf, &kstat, sizeof(stat_t));
    return 0;
}
SYSCALL_DEFINE2(vfs_user_stat, char *, stat_t *);