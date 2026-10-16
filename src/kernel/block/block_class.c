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

#include "class/class.h"

#include "dev/device.h"

#include "memory.h"

#include "kerrno.h"

status_t block_device_added(device_t *dev)
{
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
    (*out_dev)->class_data = kmalloc(sizeof(blkdev_t));
    return KERRNO_SUCCESSES;
}

status_t block_class_register()
{
    return class_register(&block_class);
}
ARCHDONE_INITCALL(block_class_register);