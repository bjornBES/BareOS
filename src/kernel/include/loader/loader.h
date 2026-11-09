/*
 * File: loader.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "mm/mmut.h"

#include "loadert.h"

#include "process/processt.h"

#include <types.h>

typedef struct loader loader_t;

typedef struct loader
{
    LIST_ENTRY_THIS();
    const char *name;
    status_t (*probe)(uint8_t *buffer, loader_t *loader);
    status_t (*load)(vfs_node_t *file, process_t *proc, loader_t *loader);
    // int (*find_symbol)(fd_t fd, process_t *proc, loader_t *loader, extern_symbol_t *out_symbols);
    // int (*load_program_with_info)(fd_t fd, process_t *proc, loader_t *loader, program_info_t *program);
    // int (*link)(fd_t fd, process_t *proc, loader_t *loader);
    void *priv;
} loader_t;

#define to_loader(n) container_of(n, loader_t, LIST_ENTRY_NAME);

status_t loader_register(loader_t *loader);

status_t loader_load(vfs_node_t *file, process_t *proc);