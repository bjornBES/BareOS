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

typedef struct hw_id
{
    char model[41];
    char serial[21];
    char firmware[9];
    uint64_t wwn;
} hw_id_t;

typedef enum
{
    PTABLE_NONE,
    PTABLE_MBR
} ptable_kind_t;

typedef struct ptable_id
{
    ptable_kind_t kind;
    uint32_t mbr_signature;
} ptable_id_t;

typedef struct fs_ident
{
    uint8_t uuid[16];
    uint8_t uuid_len;
    char label[64];
} fs_ident_t;

typedef struct blkdev blkdev_t;
typedef struct blkdev
{
    uint32_t sector_size;
    sector_t sector_start;
    sector_t sectors_count;

    uint32_t flags; // BLOCK_* flags below
    
    hw_id_t hw;
    ptable_id_t ptable;
    uint16_t partition_count;
    
    blkdev_t *parent;
    uint32_t part_index;
    sector_t start_lba;

    fs_ident_t fs_ident;
    
    device_t *dev;
} blkdev_t;

#define BLOCK_READ_ONLY    BIT(0) // read-only policy
#define BLOCK_IS_PARTITION BIT(1)

