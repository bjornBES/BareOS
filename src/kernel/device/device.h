/*
 * File: device.h
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum __device_type {
	DEVICE_UNKNOWN = 0,
	DEVICE_SERIAL = 1,
	DEVICE_NET = 2,
    DEVICE_PSEUDO = 3,
    DEVICE_VIDEO = 4,
    DEVICE_DISK = 5,
    DEVICE_CHAR = 6,
    DEVICE_TTY = 7,
} device_type;

struct device;

typedef struct __device_t
{
    char* name;
    device_type type;
    uint32_t device_id;
    uint32_t (*read)(void *buffer, uint64_t offset, size_t count, struct __device_t * device);
    uint32_t (*write)(void *buffer, uint64_t offset, size_t count, struct __device_t * device);
    void* priv;
} device;

void device_init();
uint32_t device_count();
uint32_t device_add(device* dev);
device *device_get(uint32_t id);
device *device_get_by_index(uint32_t index);