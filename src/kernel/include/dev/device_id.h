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

#define MAJOR(dev)  (((dev)) & 0xFF)
#define DEVICE(dev) (((dev) >> 16) & 0xFFFFFF)
#define MINOR(dev)  (((dev) >> 40) & 0xFFFFFF)

typedef union device_id
{
    struct
    {
        uint64_t major : 8;   // 56-63 driver class
        uint64_t res : 8;     // 48-55
        uint64_t device : 24; // 24-47 physical device instance
        uint64_t minor : 24;  // 0-23 sub-resource (partition, channel, etc.)
    };

    kernel_dev_t id;
} device_id_t;

#define MKDEV_KERNEL(maj, dev, min)                                                          \
    (((kernel_dev_t)(maj) << 0) | ((kernel_dev_t)(dev) << 16) | ((kernel_dev_t)(min) << 40))

