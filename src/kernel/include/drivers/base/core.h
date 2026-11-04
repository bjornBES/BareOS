/*
 * File: core.h
 * File Created: 02 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/devicet.h"
#include "drivert.h"
#include "classt.h"

#define core_device_create_type(class, parent, type, drvdata, ...) (type *)core_device_create_size(class, parent, sizeof(type), offsetof(type, dev), drvdata, __VA_ARGS__)

status_t device_initialize(device_t *dev);
status_t device_create_groups_vargs(const dev_class_t *class, device_t *parent, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, bool set_init, va_list args, device_t **out);
device_t *core_device_create(const dev_class_t *class, device_t *parent, void *drvdata, const char *fmt, ...);
void *core_device_create_size(const dev_class_t *class, device_t *parent, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, ...);

static int device_match_devt(device_t *dev, const void *pdevt)
{
    return dev->devt.id == *(kernel_dev_t *)pdevt;
}