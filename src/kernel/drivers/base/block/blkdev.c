/*
 * File: blkdev.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/block/blkdevt.h"

#include "debug/debug.h"

#include "dev/device.h"

#include "align.h"

#define MODULE "block-device"

status_t block_partition_read(device_t *dev, void *buf, size_t len, off_t off, ssize_t *size_out)
{
    ENTER_FUNC("%p, %p, count: %u, lba: %u, %p", dev, buf, len, off, size_out);
    
    device_t *parent_dev = dev->parent;
    blkdev_t *blk_dev = to_blkdev(dev);
    blkdev_t *parent_blk_dev = to_blkdev(parent_dev);
    
    sector_t sector_off = (ALIGN_2_UP(off, blk_dev->sector_size)) / blk_dev->sector_size;
    sector_t sector_count = (ALIGN_2_UP(len, blk_dev->sector_size)) / blk_dev->sector_size;
    
    ssize_t sector_count_out;
    device_debug();
    status_t ret = parent_blk_dev->ops.read_sectors(parent_blk_dev, buf, sector_count, sector_off + blk_dev->sector_start, &sector_count_out);
    *size_out = sector_count_out * blk_dev->sector_size;

    return ret;
}

status_t block_register(blkdev_t *blk_dev, device_t *parent)
{
    ENTER_FUNC("%p, %p", blk_dev, parent);
    blk_dev->dev.ops.read = block_partition_read;

    dev_set_no_driver(&blk_dev->dev);
    return device_register_child(&blk_dev->dev, parent);
}