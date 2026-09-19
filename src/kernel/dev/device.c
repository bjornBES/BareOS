/*
 * File: device.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "dev/device.h"
#include "dev/device_id.h"

#include "memory.h"

device_t *device_create()
{
    device_t *device = kmalloc(sizeof(device_t));
    memset(device, 0, sizeof(device_t));
    return device;
}