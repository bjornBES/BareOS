/*
 * File: partition.h
 * File Created: 22 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "fs/disk.h"

typedef struct partition
{
    disk_t *disk;
    uint32_t partition_offset;
    uint32_t partition_size;
    void *priv;
} partition_t;

int partition_detect(partition_t *part, disk_t *disk, void *partition_data, int boot_partition_idx);
int partition_read(partition_t *part, uint32_t lba, size_t sector_count, void *buffer);