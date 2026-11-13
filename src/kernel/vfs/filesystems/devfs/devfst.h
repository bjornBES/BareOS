/*
 * File: devfst.h
 * File Created: 09 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/device_id.h"

#include "vfs/vfs_internals.h"

#include "lists/hasht.h"

#include <types.h>

typedef struct dev_inode
{
    inode_t base;
    device_id_t dev_id;
    hash_map_t *map;
    uint8_t is_alias : 1;
    uint8_t is_char : 1;
    uint8_t is_blk : 1;
    uint8_t res : 5;
} dev_inode_t;

#define to_dev_inode(n) container_of(n, dev_inode_t, base);

typedef struct
{
    hash_map_t *map;
} devfs_priv_data_t;
