/*
 * File: block_class.c
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "block/blkdev.h"
#include "block/partition.h"

#include "class/class.h"

#include "dev/device.h"

#include "memory.h"

#include "kerrno.h"

#define MODULE "block-class"

status_t block_device_added(device_t *dev)
{
    ENTER_FUNC("%p", dev);

    blkdev_t *block = dev->class_data;

    trace_debug(MODULE, "block = %p", block);
    if (block == NULL)
    {
        KERRNO_RETURN(KERRNO_UNSUCCESS, "This is not a block device");
    }
    
    trace_debug(MODULE, "block->flags = 0x%x | 0x%x", block->flags, FLAG_GET(block->flags, BLOCK_IS_PARTITION));
    if (FLAG_GET(block->flags, BLOCK_IS_PARTITION) == 0)
    {
        return part_detect(dev, block);
    }
        
    return KERRNO_SUCCESSES;
}

static dev_class_t block_class = {
    .name = "block",
    .type = DEVICE_BLOCK,
    .device_added = block_device_added,
};

status_t block_device_create(device_t **out_dev)
{
    *out_dev = device_create();
    (*out_dev)->class = &block_class;
    blkdev_t *block = kzmalloc(sizeof(blkdev_t));
    (*out_dev)->class_data = block;
    block->dev = *out_dev;
    return KERRNO_SUCCESSES;
}

status_t block_class_register()
{
    return class_register(&block_class);
}
ARCHDONE_INITCALL(block_class_register);