/*
 * File: blkdevt.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/page_arch.h"

#include "dev/devicet.h"

#include <binary.h>
#include <types.h>

#define SECTOR_SHIFT       9
#define SECTOR_SIZE        (1 << SECTOR_SHIFT)

#define PAGE_SECTORS_SHIFT (PAGE_SHIFT - SECTOR_SHIFT)
#define PAGE_SECTORS       (1 << PAGE_SECTORS_SHIFT)
#define SECTOR_MASK        (PAGE_SECTORS - 1)

typedef struct blkdev
{
    sector_t sector_start;
    sector_t sectors_count;
    uint16_t partition_count;
    uint32_t flags; // BLOCK_* flags below

    device_t *dev;
} blkdev_t;

#define BLOCK_READ_ONLY    BIT(0) // read-only policy
#define BLOCK_IS_PARTITION BIT(1)

status_t block_register(blkdev_t *block_dev, device_t *parent);
