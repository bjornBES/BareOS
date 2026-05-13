/*
 * File: fs_registry.h
 * File Created: 12 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vfs_internal.h"

// add driver to registry list
int fs_register(filesystem_t *fs);

// remove driver, fail if mounts exist
int fs_unregister(const char *name);

// lookup driver by name string
filesystem_t *fs_find(const char *name);

// try each driver's probe(), return first match
filesystem_t *fs_probe(device_t *dev);
