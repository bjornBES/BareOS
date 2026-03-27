/*
 * File: device.c
 * File Created: 06 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "device.h"
#include "debug/debug.h"
#include "libs/malloc.h"

#include "libs/memory.h"

#define MODULE "DEVICE"

#define MAX_DEVICES 64

device** devices = 0;
int last_id = 0;

void device_init()
{
    devices = (device**)malloc(MAX_DEVICES * sizeof(device*));

    memset(devices, 0, MAX_DEVICES * sizeof(device*));

    last_id = 0;
    log_info(MODULE, "Devices are initialized");
}

uint32_t device_count()
{
    return last_id;
}

uint32_t device_add(device* dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -1;
    }

    if (dev->device_id == -1)
    {
        dev->device_id = last_id;
    }
    
    devices[last_id] = dev;
    last_id++;
    return last_id - 1;
}

device *device_get(uint32_t id)
{
    for (size_t i = 0; i < last_id; i++)
    {
        device *dev = devices[i];
        if (dev->device_id == id)
        {
            return dev;
        }
    }
    return NULL;
    
}
device *device_get_by_index(uint32_t index)
{
    return devices[index];
}