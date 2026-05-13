/*
 * File: device.h
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

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

typedef struct device
{
    char* name;
    device_type type;
    uint32_t device_id;
    size_t (*read)(void *buffer, off_t offset, size_t count, struct device * device);
    size_t (*write)(void *buffer, off_t offset, size_t count, struct device * device);
    void* priv;
} device_t;

void device_init();
void device_debug();
uint32_t device_count();
uint32_t device_add(device_t* dev);
device_t *device_get(uint32_t id);
device_t *device_get_by_index(uint32_t index);