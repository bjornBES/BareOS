/*
 * File: partition.h
 * File Created: 17 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/block/blkdevt.h"

#include <types.h>

status_t part_detect(device_t *dev, blkdev_t *blk_dev);

status_t mbr_detect(device_t *dev, void *mbr);