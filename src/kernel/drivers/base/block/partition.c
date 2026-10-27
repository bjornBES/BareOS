/*
 * File: partition.c
 * File Created: 13 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/block/partition.h"

#include "drivers/base/block/blkdev.h"

#include "kerrno.h"

#define MODULE "partitions"

extern void hexdump(void *ptr, size_t len, size_t size);

status_t part_detect(device_t *dev, blkdev_t *blk_dev)
{
    ENTER_FUNC("%p, %p", dev, blk_dev);
    char buf[512];
    ssize_t sector_size;
    status_t ret = dev->ops.read(dev, buf, 1, 0, &sector_size);
    if (sector_size != 1 && ret != KERRNO_SUCCESSES)
    {
        trace_err(MODULE, "something is wrong");
    }
    
    hexdump(buf + 0x1be, 0x10*4, 16);
    return mbr_detect(dev, buf);
}