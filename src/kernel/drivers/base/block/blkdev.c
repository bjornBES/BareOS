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

#define MODULE "block-device"

status_t block_partition_read(device_t *dev, void *buf, size_t len, off_t off, ssize_t *size_out)
{
    ENTER_FUNC("%p, %p, count: %u, lba: %u, %p", dev, buf, len, off, size_out);
    device_t *parent = dev->parent;
    blkdev_t *block = dev->class_data;
    ENTER_FUNC("%p, %p, count: %u, lba: %u, %p", parent, buf, len, off + block->sector_start, size_out);
    return parent->ops.read(parent, buf, len, off + block->sector_start, size_out);
}

status_t block_register(blkdev_t *block_dev, device_t *parent)
{
    ENTER_FUNC("%p, %p", block_dev, parent);
    block_dev->dev->ops.read = block_partition_read;

    dev_set_no_driver(block_dev->dev);
    return device_register_child(block_dev->dev, parent);
}