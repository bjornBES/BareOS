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

#include "debug/debug.h"

#include <config.h>

#define MODULE "vfs-table"

status_t vfs_open_node(fd_table_t *table, int index)
{
    {
        if (table->entries[index])
        {
            trace_debug(MODULE, "still have node %p open", table->entries[index]->node);
            table->entries[index]->refcount++;
        }
    }
    return KERRNO_SUCCESSES;
}
