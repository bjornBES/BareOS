/*
 * File: device.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "devicet.h"
#include "device_id.h"
#include "init.h"

#include "module.h"

#include <types.h>
#include <defs.h>

typedef status_t (*device_irq_handler_t)(device_t *dev);

device_t *device_create();
status_t device_register_irq(device_t *dev, device_irq_handler_t generic_handler);

status_t device_id_print(device_id_t *devt);
uint32_t device_id_get_next_class_id(const char *class_name);
uint32_t device_id_get_id(device_t *dev);

status_t device_get_device(device_id_t *id, device_t **out_dev);

status_t device_register(device_t *dev);
status_t device_add(device_t *dev);
status_t device_register_child(device_t *dev, device_t *parent);

status_t device_unregister(device_t *dev);

status_t device_set_name(device_t *dev, const char *fmt, ...);

void device_print_debug(device_t *dev);
void device_debug();

ALWAYS_INLINE void device_set_drvdata(device_t *dev, void *data)
{
    dev->drv_data = data;
}

status_t device_open(device_t *dev, int flags);
