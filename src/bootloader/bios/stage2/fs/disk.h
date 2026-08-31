/*
 * File: disk.h
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
#include <stdbool.h>

#include <defs.h>

typedef struct disk
{
    uint16_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
    bool have_extensions;
} disk_t;

int disk_initialize(disk_t *disk, uint16_t boot_drive);

int disk_read_sectors(disk_t *disk, uint32_t lba, size_t sectors, void *buffer);

INLINE void disk_lba_to_chs(disk_t *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *sectorOut, uint16_t *headOut)
{
    if (disk->have_extensions)
    {
        return;
    }
    // sector = (LBA % sectors per track + 1)
    *sectorOut = lba % disk->sectors + 1;

    // cylinder = (LBA / sectors per track) / heads
    *cylinderOut = (lba / disk->sectors) / disk->heads;

    // head = (LBA / sectors per track) % heads
    *headOut = (lba / disk->sectors) % disk->heads;
}
