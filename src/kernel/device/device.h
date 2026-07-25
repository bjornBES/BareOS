/*
 * File: device.h
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <util/binary.h>

#include "device_types.h"

#define MAJOR(dev)  (((dev) >> 48) & 0xFFFF)
#define DEVICE(dev) (((dev) >> 24) & 0xFFFFFF)
#define MINOR(dev)  ((dev) & 0xFFFFFF)

#define MKDEV(maj, dev, min)                                       \
    (((dev_t)(maj) << 48) | ((dev_t)(dev) << 24) | ((dev_t)(min)))

typedef struct
{
    uint64_t major : 16;  // driver class
    uint64_t device : 24; // physical device instance
    uint64_t minor : 24;  // sub-resource (partition, channel, etc.)
} dev_id_t;

// flags
#define DEVICE_FLAG_READABLE  BIT(0)
#define DEVICE_FLAG_WRITABLE  BIT(1)
#define DEVICE_FLAG_RW        BIT(0) | BIT(1)
#define DEVICE_FLAG_BLOCKDEV  BIT(2)
#define DEVICE_FLAG_VIRTUAL   BIT(3) // pseudo devices
#define DEVICE_FLAG_STUB      BIT(4)
#define DEVICE_FLAG_DEV       BIT(5)
#define DEVICE_FLAG_PARTITION BIT(6)

void device_init();
void device_debug();

// creation
device_t *device_create(device_type_t type, uint32_t flags);

// registration
int device_register_under_dev_id(device_t *dev, dev_t device_id);
int device_register(device_t *dev);
void device_unregister(device_t *dev);

// lookup
device_t *device_get_by_name(const char *name);
device_t *device_get_by_dev_id(dev_t id);
device_t *device_get_by_id(device_type_t type, dev_t id);
device_t *device_get_first(device_type_t type);
device_t *device_get_next(device_t *dev);

// ops wrappers — null checks the function pointer
ssize_t device_read(device_t *dev, void *buf, off_t off, size_t count);
ssize_t device_write(device_t *dev, void *buf, off_t off, size_t count);
int device_ioctl(device_t *dev, uint32_t cmd, void *arg);
