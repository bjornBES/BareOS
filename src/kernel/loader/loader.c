/*
 * File: loader.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "loader/loader.h"

#include "lists/list.h"

#include "vfs/vfs.h"

#include "memory.h"

#include "kerrno.h"

#include "init.h"

#define MODULE "loader"

list_t loader_list;

status_t loader_init()
{
    list_initialize(&loader_list);
    CALL_INITCALL_FUNCTIONS(INITCALL_LVL_LOADER);
    return KERRNO_SUCCESSES;
}

ARCHDONE_INITCALL(loader_init);

status_t loader_register(loader_t *loader)
{
    if (!loader->probe || !loader->load)
    {
        return KERRNO_UNSUCCESS;
    }
    return ADD_THIS_TO_LIST(loader_list, loader);
}

status_t loader_load(vfs_node_t *file, process_t *proc)
{
    uint8_t *buffer = kzmalloc(512);
    vfs_read(file, buffer, 0, 512, NULL);

    list_node_t *curr = loader_list.head;
    loader_t *loader = NULL;
    while (curr)
    {
        loader = to_loader(curr);
        if (loader->probe(buffer, loader) == KERRNO_SUCCESSES)
        {
            kfree(buffer);
            break;
        }
        loader = NULL;
    }

    kfree(buffer);
    if (!loader)
    {
        KERRNO_RETURN(KERRNO_DEVICE_NOT_FOUND, "");
    }

    return loader->load(file, proc, loader);
}
