/*
 * File: part_mbr.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/block/blkdev.h"

#include "dev/device.h"

#include "debug/debug.h"

#include "stdio.h"

#include "vfs/vfs.h"

#include <array.h>
#include <types.h>
#include <defs.h>

#define MODULE "partitions-mbr"

typedef struct mbr_entry
{
    // 0x00	1	Drive attributes (bit 7 set = active or bootable)
    uint8_t attributes;

    // 0x01	3	CHS Address of partition start
    uint8_t chs_start[3];

    // 0x04	1	Partition type
    uint8_t partition_type;

    // 0x05	3	CHS address of last partition sector
    uint8_t chs_end[3];

    // 0x08	4	LBA of partition start
    uint32_t lba_start;

    // 0x0C	4	Number of sectors in partition
    uint32_t size;

} PACKED mbr_entry_t;

status_t mbr_detect(blkdev_t *blk_dev, void *mbr)
{
    mbr_entry_t *partitions = mbr + 0x1BE;

    for (size_t i = 0; i < 4; i++)
    {
        mbr_entry_t *entry = &partitions[i];
        if (entry->partition_type == 0 && entry->lba_start == 0 && entry->size == 0)
        {
            continue;
        }
        trace_info(MODULE, "Got partition %u @ %p", i, entry);
        trace_info(MODULE, "      attributes 0x%x", entry->attributes);
        trace_info(MODULE, "       chs start 0x%x%x%x", entry->chs_start[2], entry->chs_start[1], entry->chs_start[0]);
        trace_info(MODULE, "  partition type 0x%x", entry->partition_type);
        trace_info(MODULE, "         chs end 0x%x%x%x", entry->chs_end[2], entry->chs_end[1], entry->chs_end[0]);
        trace_info(MODULE, "       lba start 0x%x", entry->lba_start);
        trace_info(MODULE, "            size 0x%x", entry->size);
        if (entry->partition_type == 0xEE)
        {
            KERNEL_PANIC(MODULE, "disk has a GPT table that is not supportted yet");
        }

        blkdev_t *part_blk_dev;
        block_device_create(&part_blk_dev);
        part_blk_dev->flags |= BLOCK_IS_PARTITION;
        part_blk_dev->partition_count = 0;
        part_blk_dev->sector_size = blk_dev->sector_size;
        part_blk_dev->sector_start = entry->lba_start;
        part_blk_dev->sectors_count = entry->size;

        part_blk_dev->dev.init_name = blk_dev->dev.name;

        status_t ret = block_register(part_blk_dev, &blk_dev->dev);
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
        ret = vfs_mount(&part_blk_dev->dev, 0);
        if (ret == KERRNO_ENTRY_NOT_FOUND)
        {
            continue;
        }
        KERRNO_CHECK_RETURN(ret);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
    }
    return KERRNO_SUCCESSES;
}
