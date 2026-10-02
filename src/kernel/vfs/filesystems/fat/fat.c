/*
 * File: fat.c
 * File Created: 29 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 29 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "fat.h"
#include "vfs/fs_registry.h"

static filesystem_t fat_fs = {
    .name = "FAT",
};

VFS_FS_DRIVER(fat_fs);