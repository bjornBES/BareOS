/*
 * File: fork.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "process/process.h"

#include "mm/vma/vma.h"

#include "vfs/vfs.h"
#include "vfs/vfs_table.h"

#include "kerrno.h"

#include "memory.h"

#define MODULE "process-fork"

INTERNAL status_t proc_clone(process_t *dest, process_t *src)
{
    vma_clone(dest->vma, src->vma);
    memcpy(&dest->loader_data, &src->loader_data, sizeof(loader_data_t));
    dest->pledged = src->pledged;
    dest->pledge_mask = src->pledge_mask;
    dest->pid_blk = src->pid_blk;
    dest->user = src->user;
    memcpy(dest->path, src->path, CONFIG_MAX_PATH_LENGTH);
    memcpy(dest->volume, src->volume, CONFIG_MAX_VOLUME_LENGTH);
    memcpy(&dest->fd_table, &src->fd_table, sizeof(fd_table_t));

    return KERRNO_SUCCESSES;
}

status_t proc_vfork(process_t *parent, process_t *child)
{
    vma_allocate(&child->vma);
    proc_clone(child, parent);

    proc_add(parent, child);

    vfs_clone_node(&parent->fd_table, &child->fd_table);

    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

status_t proc_spawn(process_t *parent, process_t *child)
{
    vma_allocate(&child->vma);

    if (parent)
    {
        proc_add(parent, child);
        vfs_clone_node(&parent->fd_table, &child->fd_table);
    }
    return KERRNO_SUCCESSES;
}
