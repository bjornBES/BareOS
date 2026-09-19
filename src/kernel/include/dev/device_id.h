/*
 * File: device_id.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define MAJOR(dev)  (((dev)) & 0xFFFF)
#define DEVICE(dev) (((dev) >> 16) & 0xFFFFFF)
#define MINOR(dev)  (((dev) >> 40) & 0xFFFFFF)

typedef union device_id
{
    struct
    {
        uint64_t major : 16;  // driver class
        uint64_t device : 24; // physical device instance
        uint64_t minor : 24;  // sub-resource (partition, channel, etc.)
    };

    kernel_dev_t id;
} device_id_t;

#define MKDEV_KERNEL(maj, dev, min)                                                     \
    (((kernel_dev_t)(maj) << 0) | ((kernel_dev_t)(dev) << 16) | ((kernel_dev_t)(min) << 40))

