/*
 * File: vfs_table.c
 * File Created: 05 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/vfs_table.h"
#include "vfs/vfs.h"

#include "debug/debug.h"

#include "memory.h"

#include <config.h>

#define MODULE "vfs-table"

status_t vfs_clone_node(fd_table_t *table, fd_table_t *dest)
{
    ENTER_FUNC("%p, %p, %u", table, dest);
    for (size_t i = 0; i < CONFIG_PROCESS_MAX_OPEN_FD; i++)
    {
        if (table->entries[i] == NULL)
        {
            return KERRNO_SUCCESSES;
        }
        if (table->entries[i] != NULL)
        {
            trace_debug(MODULE, "table->entries[%u](%p) = dest->entries[%u](%p)", i, table->entries[i], i, dest->entries[i]);
            dest->entries[i] = table->entries[i];
            trace_debug(MODULE, "still have node %p open", table->entries[i]->node);
            table->entries[i]->refcount++;
        }
    }
    return KERRNO_SUCCESSES;
}

status_t vfs_table_find_free(fd_table_t *table, fd_t *out_fd)
{
    for (size_t i = 0; i < CONFIG_PROCESS_MAX_OPEN_FD; i++)
    {
        if (!table->entries[i])
        {
            *out_fd = i;
            return KERRNO_SUCCESSES;
        }
    }
    KERRNO_RETURN(KERRNO_NO_MORE_FDS, "");
}

status_t vfs_open_file(fd_table_t *table, const char *path, fd_t *out_fd)
{
    ENTER_FUNC("%p, %s, %p", table, path, out_fd);
    vfs_node_t *node;
    status_t ret = vfs_open(path, 0, 0, &node);
    KERRNO_CHECK_RETURN(ret);
    if (ret)
    {
        return ret;
    }

    fd_t fd;
    ret = vfs_table_find_free(table, &fd);
    KERRNO_CHECK_RETURN(ret);
    if (ret)
    {
        return ret;
    }

    if (out_fd)
    {
        *out_fd = fd;
    }

    table->entries[fd] = kzmalloc(sizeof(fd_entry_t));
    table->entries[fd]->node = node;
    table->entries[fd]->refcount++;

    return KERRNO_SUCCESSES;
}

status_t vfs_open_dev(fd_table_t *table, device_t *dev, fd_t *out_fd)
{
    return KERRNO_SUCCESSES;
}
