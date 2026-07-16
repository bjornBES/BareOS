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

// flags
#define DEVICE_FLAG_READABLE BIT(0)
#define DEVICE_FLAG_WRITABLE BIT(1)
#define DEVICE_FLAG_RW       BIT(0) | BIT(1)
#define DEVICE_FLAG_BLOCKDEV BIT(2)
#define DEVICE_FLAG_VIRTUAL  BIT(3) // pseudo devices


void device_init();
void device_debug();

// registration
int device_register(device_t *dev);
void device_unregister(device_t *dev);

// lookup
device_t *device_get_by_name(const char *name);
device_t *device_get_by_id(device_type_t type, dev_t id);
device_t *device_get_first(device_type_t type);
device_t *device_get_next(device_t *dev);

// ops wrappers — null checks the function pointer
size_t device_read(device_t *dev, void *buf, off_t off, size_t count);
size_t device_write(device_t *dev, void *buf, off_t off, size_t count);
int device_ioctl(device_t *dev, uint32_t cmd, void *arg);
