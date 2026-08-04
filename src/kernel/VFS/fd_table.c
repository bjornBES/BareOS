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
void fd_insert(vfs_node_t *node, fd_t file)
{
    spinlock_acquire(&fd_lock);
    fd_table[file] = node;
    spinlock_release(&fd_lock);
}

// find lowest free slot, store node ptr
fd_t fd_alloc(vfs_node_t *node)
{
    spinlock_acquire(&fd_lock);
    for (fd_t fd = 0; fd < MAX_OPEN_FILES; fd++)
    {
        log_debug(MODULE, "fd_table[%u] = %p", fd, fd_table[fd]);
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

// find lowest free slot, store node ptr
fd_t fd_alloc_for_process(vfs_node_t *node, fd_table_t *table)
{
    spinlock_acquire(&fd_lock);
    for (fd_t fd = 0; fd < MAX_OPEN_FILES; fd++)
    {
        if (table->entries[fd] == NULL)
        {
            table->entries[fd] = malloc(sizeof(fd_entry_t));
            table->entries[fd]->node = node;
            spinlock_release(&fd_lock);
            return fd;
        }
    }
    spinlock_release(&fd_lock);
    return VFS_INVALID_FD;
}

// validate fd, return node ptr
fd_entry_t *fd_get_for_process(fd_t fd, fd_table_t *table)
{
    // ENTER_FUNC(MODULE, "%u, %p", fd, table);
    if (fd == VFS_INVALID_FD)
    {
        log_err(MODULE, "fd is invalid");
        return NULL;
    }

    fd_entry_t *node = table->entries[fd];
    return node;
}

// validate fd, return node ptr
vfs_node_t *fd_get(fd_t fd)
{
    // log_debug(MODULE, "fd_get(%u)", fd);
    if (fd == VFS_INVALID_FD)
    {
        log_err(MODULE, "fd is invalid");
        return NULL;
    }

    vfs_node_t *node = fd_table[fd];
    if (node == NULL)
    {
        log_err(MODULE, "fd %u is not open", fd);
        return NULL;
    }
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
vfs_node_t stddebug2_node;
#endif

// zero table, reserve stdin/stdout/stderr/debug
void fd_table_init()
{
    memset(fd_table, 0, sizeof(fd_table));

    stdin_node.opened = 1;
    stdout_node.opened = 1;
    stderr_node.opened = 1;
    stddebug_node.opened = 1;
    stddebug2_node.opened = 1;

    fd_table[0] = &stdin_node;
    fd_table[1] = &stdout_node;
    fd_table[2] = &stderr_node;
#ifdef DEBUG
    fd_table[3] = &stddebug_node;
    fd_table[4] = &stddebug2_node;
#endif
}
