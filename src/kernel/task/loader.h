/*
 * File: loader.h
 * File Created: 16 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "process.h"
#include "VFS/vfs.h"

typedef struct __loader_t
{
    char *name;
    int (*probe)(uint8_t *buffer, struct __loader_t *loader);
    int (*load)(fd_t fd, process *proc, struct __loader_t *loader);
    void *priv;
} loader;

void Loader_register_loader(loader *loader);
loader *Loader_probe(fd_t file);
void Loader_init();
