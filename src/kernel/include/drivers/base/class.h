/*
 * File: class.h
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "classt.h"

#include "core.h"

typedef int (*device_match_t)(struct device *dev, const void *data);

status_t class_add_device(device_t *dev);
status_t class_remove_device(device_t *dev);

status_t class_list_devices();

status_t class_get(dev_type_t type, dev_class_t **class_out);

status_t class_register(const dev_class_t *class);

status_t class_find_device(const dev_class_t *class, const device_t *start, const void *data, device_match_t match, device_t **out_dev);

static status_t class_find_device_by_devt(const dev_class_t *class, kernel_dev_t devt, device_t **out_dev)
{
    return class_find_device(class, NULL, &devt, device_match_devt, out_dev);
}
