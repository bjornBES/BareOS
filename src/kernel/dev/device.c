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

#include "stdio.h"

#include "memory.h"

device_t *device_create()
{
    device_t *device = kmalloc(sizeof(device_t));
    memset(device, 0, sizeof(device_t));
    return device;
}

status_t device_set_name(device_t *dev, const char *fmt, ...)
{
    va_list vargs;
    status_t err = KERRNO_SUCCESSES;

    va_start(vargs, fmt);
    memset(dev->name, 0, sizeof(dev->name));
    size_t s = vsprintf(dev->name, fmt, vargs);
    dev->name[s] = '\0';
    hexdump(dev->name, sizeof(dev->name), 16);
    dev_set_same_name(dev);
    dev->init_name = dev->name;
    dev->init_name[s] = '\0';
    // err = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
    va_end(vargs);
    return err;
}
