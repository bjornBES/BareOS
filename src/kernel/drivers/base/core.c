/*
 * File: core.c
 * File Created: 02 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/core.h"

#include "dev/device.h"

#include "memory.h"

#include "stdio.h"
#include "string.h"

#include "kerrno.h"

#define MODULE "core"

status_t device_create_groups_vargs(const dev_class_t *class, device_info_t *info, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, va_list args, device_t **out)
{
    ENTER_FUNC("%p, %p, %u, %u, %p, %s, %p", class, info, dev_size, dev_offset, drvdata, fmt, out);
    void *t_dev = NULL;
    device_t *dev = NULL;
    status_t ret = ENODEV;

    t_dev = kzmalloc(dev_size);
    if (!t_dev)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    dev = t_dev;
    if (dev_offset >= 0)
    {
        device_t *dev_field = ((void *)t_dev + dev_offset);
        trace_debug(MODULE, "t_dev = %p", t_dev);
        trace_debug(MODULE, "dev_field = %p", dev_field);
        dev = dev_field;
    }

    device_initialize(dev);

    if (!info->driver)
    {
        dev_set_no_driver(dev);
    }

    size_t size = 0;
    size = vsnprintf(dev->name, DEVICE_NAME_MAX, fmt, args);
    dev->name[size] = '\0';

    dev->class = class;
    dev->drv_data = drvdata;

    ret = device_register_id(dev, info);
    KERRNO_CHECK_AND_RETURN(ret);

    *out = t_dev;
    return KERRNO_SUCCESS;
}

void *core_device_create_size(const dev_class_t *class, device_info_t *info, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, ...)
{
    va_list vargs;
    device_t *dev;

    va_start(vargs, fmt);
    status_t ret = device_create_groups_vargs(class, info, dev_size, dev_offset, drvdata, fmt, vargs, &dev);
    va_end(vargs);
    if (ret != KERRNO_SUCCESS)
    {
        return NULL;
    }
    return dev;
}

device_t *core_device_create(const dev_class_t *class, device_info_t *info, void *drvdata, const char *fmt, ...)
{
    va_list vargs;
    device_t *dev;

    va_start(vargs, fmt);
    status_t ret = device_create_groups_vargs(class, info, sizeof(device_t), -1, drvdata, fmt, vargs, &dev);
    va_end(vargs);
    if (ret != KERRNO_SUCCESS)
    {
        return NULL;
    }
    return dev;
}
