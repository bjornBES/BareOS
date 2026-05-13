/*
 * File: partition_manager.c
 * File Created: 09 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "partition_manager.h"
#include "debug/debug.h"
#include "libs/malloc.h"
#include "libs/memory.h"

#define MODULE "PARTITION"

// MBR partition entry (16 bytes each, little-endian)
typedef struct
{
    uint8_t boot_indicator; // 0x80 = bootable, 0x00 = non-bootable
    uint8_t start_chs[3];   // CHS (ignore for now)
    uint8_t partition_type; // FS type ID (e.g. 0x0B = FAT32)
    uint8_t end_chs[3];     // CHS (ignore for now)
    uint32_t lba_start;     // starting LBA of partition
    uint32_t sector_count;  // size of partition in sectors
} __attribute__((packed)) partition_entry;

// Partition "device" info, stored in device->priv
typedef struct
{
    device_t *parent;   // underlying disk
    off_t lba_start;    // first sector
    off_t sector_count; // length of partition
    uint8_t type;       // partition type
    uint8_t index;      // partition index (1..4)
} partition_info;
extern void hexdump(void *ptr, int len);
size_t partition_read(void *buffer, off_t offset, size_t len, device_t *dev)
{
    log_debug(MODULE, "partition_read(%p, %u, %u, %p)", buffer, offset, len, dev);
    partition_info *part_info = (partition_info *)(dev->priv);
    off_t lba = part_info->lba_start + offset;
    if (offset >= part_info->sector_count)
    {
        log_err(MODULE, "Trying to read out of bounds range");
        return 0; // out of bounds
    }
    device_t *parent = part_info->parent;
    size_t bytes = parent->read(buffer, lba, len, parent);
    return bytes;
}

size_t partition_write(void *buffer, off_t offset, size_t len, device_t *dev)
{
    partition_info *part_info = (partition_info *)(dev->priv);
    uint64_t lba = part_info->lba_start + offset;
    if (offset >= part_info->sector_count)
    {
        return 0; // out of bounds
    }
    device_t *parent = part_info->parent;
    return parent->write(buffer, lba, len, parent);
}

void partition_scan(device_t *dev)
{
    uint8_t mbr[512];
    if (dev->read(mbr, 0, 1, dev) != 1)
    {
        log_err(MODULE, "Failed to read MBR from %s", dev->name);
        return;
    }

    uint16_t sig = *(uint16_t *)(mbr + 510);
    if (sig != 0xAA55)
    {
        log_err(MODULE, "Invalid MBR signature on %s", dev->name);
        return;
    }

    partition_entry *entries = (partition_entry *)(mbr + 0x1BE);

    for (size_t i = 0; i < 4; i++)
    {
        partition_entry entry = entries[i];

        if (entry.partition_type == 0)
        {
            continue;
        }

        // Allocate partition info
        partition_info *pinfo = malloc(sizeof(partition_info));
        memset(pinfo, 0, sizeof(partition_info));
        pinfo->parent = dev;
        pinfo->lba_start = entries[i].lba_start;
        pinfo->sector_count = entries[i].sector_count;
        pinfo->type = entries[i].partition_type;
        pinfo->index = i + 1;

        // Create partition device
        device_t *pdev = malloc(sizeof(device_t));
        memset(pdev, 0, sizeof(device_t));

        char *name = malloc(16);
        int count = snprintf(name, 16, "part%d", i);
        name[count] = '\0';
        pdev->name = name;

        pdev->type = DEVICE_DISK;
        pdev->device_id = (dev->device_id << 8) + i;
        pdev->read = partition_read;
        pdev->write = partition_write;
        pdev->priv = pinfo;

        // Register partition device
        device_add(pdev);

        log_info(MODULE,
                 "Found partition %d on %s: start=0x%X, size=0x%X, type=0x%02X -> %s",
                 i, dev->name ? dev->name : "<unnamed>",
                 pinfo->lba_start, pinfo->sector_count,
                 pinfo->type, pdev->name);
    }
}

void partition_block_device_register(device_t *dev)
{
    partition_scan(dev);
}
