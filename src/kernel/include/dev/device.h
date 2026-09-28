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

uint32_t device_id_get_next_class_id(const char *class_name);
uint32_t device_id_get_id(device_t *dev);


status_t device_register(device_t *dev);
void device_debug();