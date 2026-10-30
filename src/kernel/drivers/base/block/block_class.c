/*
 * File: block_class.c
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/block/blkdev.h"
#include "drivers/base/block/partition.h"

#include "drivers/base/class.h"

#include "dev/device.h"

#include "memory.h"

#include "kerrno.h"

#define MODULE "block-class"

status_t block_device_added(device_t *dev)
{
    ENTER_FUNC("%p", dev);

    blkdev_t *blk_dev = to_blkdev(dev);

    trace_debug(MODULE, "blk_dev = %p", blk_dev);
    if (blk_dev == NULL)
    {
        KERRNO_RETURN(KERRNO_UNSUCCESS, "This is not a block device");
    }

    // trace_debug(MODULE, "block->flags = 0x%x | 0x%x", blk_dev->flags, FLAG_GET(blk_dev->flags, BLOCK_IS_PARTITION));
    if (FLAG_GET(blk_dev->flags, BLOCK_IS_PARTITION) == 0)
    {
        return part_detect(blk_dev);
    }

    return KERRNO_SUCCESSES;
}

dev_class_t block_class = {
    .name = "block",
    .type = DEVICE_BLOCK,
    .device_added = block_device_added,
    .devices = { 0 },
};

status_t block_device_create(blkdev_t **blk_dev_out)
{
    ENTER_FUNC("%p", blk_dev_out);
    blkdev_t *blk_dev = kzmalloc(sizeof(blkdev_t));
    device_t *dev = &blk_dev->dev;

    dev_set_no_driver(dev);

    dev->class = &block_class;

    *blk_dev_out = blk_dev; // FUCK ME NOW PLZ
    // - BjornBEs 28.10.2026 21:42
    // ... ok context i have been debugging this for about 1:30 HOURS
    // and i forgot to set the output.
    return KERRNO_SUCCESSES;
}

status_t block_class_register()
{
    return class_register(&block_class);
}

ARCHDONE_INITCALL(block_class_register);
