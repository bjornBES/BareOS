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

status_t device_initialize(device_t *dev);

status_t device_id_print(device_id_t *devt);
status_t device_id_get_next_minor_id(device_t *dev, uint32_t *out_minor);
uint32_t device_id_get_id(device_t *dev);

status_t device_get_device(device_id_t *id, device_t **out_dev);

/// @brief
/// @param[in] dev
/// @param[in] info
/// @return KERRNO_SUCCESS on successes or an kerrno number
/// @retval KERRNO_ALLOC_FAILED: If an allocation failed
status_t device_register_id(device_t *dev, device_info_t *info);

static ALWAYS_INLINE status_t device_add_under_id(device_t *dev, const char *name, device_id_t id)
{
    device_info_t info = {
        .dev_id = id,
        .init_name = (char *)name,
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_register_with_name(device_t *dev, const char *name)
{
    device_info_t info = {
        .dev_id = {
            .major = dev->class->type,
            .device = DEV_AUTO_DEV,
            .minor = DEV_AUTO_MIN,
        },
        .init_name = (char *)name,
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_register_child_with_name_under_id(device_t *dev, const char *name, device_id_t id, device_t *parent)
{
    device_info_t info = {
        .dev_id = id,
        .parent = parent,
        .init_name = (char *)name,
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_register_child_with_name(device_t *dev, const char *name, device_t *parent)
{
    device_info_t info = {
        .dev_id = {
            .major = dev->class->type,
            .device = DEV_AUTO_DEV,
            .minor = DEV_AUTO_MIN,
        },
        .parent = parent,
        .init_name = (char *)name,
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_register(device_t *dev)
{
    device_info_t info = {
        .dev_id = {
            .major = dev->class->type,
            .device = DEV_AUTO_DEV,
            .minor = DEV_AUTO_MIN,
        },
        .init_name = "\0",
        .driver = NULL,
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_add(device_t *dev)
{
    device_info_t info = {
        .parent = dev->parent,
        .dev_id = dev->devt,
        .driver = dev->driver,
        .init_name = "\0",
    };
    return device_register_id(dev, &info);
}

static ALWAYS_INLINE status_t device_register_child(device_t *dev, device_t *parent)
{
    device_info_t info = {
        .parent = parent,
        .dev_id.id = DEV_ID_AUTO,
        .init_name = "\0",
    };
    return device_register_id(dev, &info);
}

status_t device_get(device_t *dev);
status_t device_put(device_t *dev);

status_t device_unregister(device_t *dev);

status_t device_set_name(device_t *dev, const char *fmt, ...);

void device_print_debug(device_t *dev);
void device_debug();

ALWAYS_INLINE void device_set_drvdata(device_t *dev, void *data)
{
    dev->drv_data = data;
}

ALWAYS_INLINE const char *dev_name(const device_t *dev)
{
    if (dev->name)
    {
        return dev->name;
    }
    return dev->init_name;
}

status_t device_open(device_t *dev, int flags);
