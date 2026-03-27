/*
 * File: loader.c
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "loader.h"

#include "libs/malloc.h"
#include "libs/memory.h"

#include "debug/debug.h"


#define MAX_LOADERS 16

loader **loaders;
int loader_index;

void Loader_register_loader(loader *loader)
{
    loaders[loader_index] = loader;
    loader_index++;
}

loader *Loader_probe(fd_t file)
{
    uint8_t *buffer = malloc(1024);
    for (size_t i = 0; i < loader_index; i++)
    {
        if (loaders[i] == NULL)
        {
            continue;
        }

        VFS_read(file, buffer, 1024);
        if (loaders[i]->probe(buffer, loaders[i]))
        {
            free(buffer);
            return loaders[i];
        }
    }
    free(buffer);
    log_crit("LOADER", "no loaders available");
    return NULL;
}

void Loader_init()
{
    loaders = (loader**)malloc(MAX_LOADERS * sizeof(loader*));
    memset(loaders, 0, sizeof(loader*));
    loader_index = 0;
}