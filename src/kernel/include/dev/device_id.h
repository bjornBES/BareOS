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

#define DEV_MAJ_BITS      12
#define DEV_DEV_BITS      24
#define DEV_MIN_BITS      24
#define DEV_RES_BITS      (64 - DEV_MAJ_BITS - DEV_DEV_BITS - DEV_MIN_BITS)

#define DEV_RES_SHIFT     0
#define DEV_MAJ_SHIFT     (DEV_RES_BITS)
#define DEV_DEV_SHIFT     (DEV_RES_BITS + DEV_MAJ_BITS)
#define DEV_MIN_SHIFT     (DEV_RES_BITS + DEV_MAJ_BITS + DEV_DEV_BITS)

#define DEV_AUTO_MAJ      ((1ull << DEV_MAJ_BITS) - 1)
#define DEV_AUTO_DEV      ((1ull << DEV_DEV_BITS) - 1)
#define DEV_AUTO_MIN      ((1ull << DEV_MIN_BITS) - 1)

#define MAJOR(dev)        (((dev) >> DEV_MAJ_SHIFT) & DEV_AUTO_MAJ)
#define DEVICE(dev)       (((dev) >> DEV_DEV_SHIFT) & DEV_AUTO_DEV)
#define MINOR(dev)        (((dev) >> DEV_MIN_SHIFT) & DEV_AUTO_MIN)

#define MAJOR_DEVICE(dev) (((dev) >> DEV_MAJ_SHIFT) & ((DEV_AUTO_MAJ) | (DEV_AUTO_DEV << DEV_MAJ_BITS)))

typedef union device_id
{
    struct
    {
        uint64_t res : DEV_RES_BITS;    //
        uint64_t major : DEV_MAJ_BITS;  // driver class
        uint64_t device : DEV_DEV_BITS; // physical device instance
        uint64_t minor : DEV_MIN_BITS;  // sub-resource (partition, channel, etc.)
    };

    kernel_dev_t id;
} device_id_t;

#define MKDEV_KERNEL(maj, dev, min)                                                                                            \
    (((kernel_dev_t)(maj) << DEV_MAJ_SHIFT) | ((kernel_dev_t)(dev) << DEV_DEV_SHIFT) | ((kernel_dev_t)(min) << DEV_MIN_SHIFT))

#define DEV_ID_AUTO ((kernel_dev_t)~0ull)

typedef struct
{
    uint16_t maj;
    uint32_t dev;
    uint32_t next_min; /* allocation hint, not a guarantee */
} dev_id_group_t;
