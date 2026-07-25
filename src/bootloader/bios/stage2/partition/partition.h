/*
 * File: partition.h
 * File Created: 17 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "fs/disk.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    DISK* disk;
    uint32_t partitionOffset;
    uint32_t partitionSize;
} Partition;

bool Partition_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void* lowerDataOut);
