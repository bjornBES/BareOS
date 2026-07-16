/*
 * File: fd_table.c
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fd_table.h"

#include "debug/debug.h"
#include "kernel/memory.h"

#define MODULE "VFS-FD"

spinlock_t fd_lock;
vfs_node_t *fd_table[FD_SIZE];

// find lowest free slot, store node ptr
fd_t fd_alloc(vfs_node_t *node)
{
    spinlock_acquire(&fd_lock);
    for (fd_t fd = VFS_FS_NEXT; fd < MAX_OPEN_FILES; fd++)
    {
        if (fd_table[fd] == NULL)
        {
            fd_table[fd] = node;
            spinlock_release(&fd_lock);
            return fd;
        }
    }
    spinlock_release(&fd_lock);
    return VFS_INVALID_FD;
}

// validate fd, return node ptr
vfs_node_t *fd_get(fd_t fd)
{
    log_debug(MODULE, "fd_get(%u)", fd);
    if (fd == VFS_INVALID_FD)
    {
        log_err(MODULE, "fd is invalid");
        return NULL;
    }
    
    vfs_node_t *node = fd_table[fd];
    return node;
}

// null the slot, free node
void fd_release(fd_t fd)
{
    spinlock_acquire(&fd_lock);
    fd_table[fd] = NULL;
    spinlock_release(&fd_lock);
}

vfs_node_t stdout_node;
vfs_node_t stdin_node;
vfs_node_t stderr_node;
#ifdef DEBUG
vfs_node_t stddebug_node;
#endif

// zero table, reserve stdin/stdout/stderr/debug
void fd_table_init()
{
    memset(fd_table, 0, sizeof(fd_table));
    fd_table[0] = &stdin_node;
    fd_table[1] = &stdout_node;
    fd_table[2] = &stderr_node;
#ifdef DEBUG
    fd_table[3] = &stddebug_node;
#endif
}
