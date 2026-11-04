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

status_t device_initialize(device_t *dev)
{
    // dev->kobj.kset = devices_kset;
    // kobject_init(&dev->kobj, &device_ktype);
    // INIT_LIST_HEAD(&dev->dma_pools);
    // mutex_init(&dev->mutex);
    // spin_lock_init(&dev->driver_override.lock);
    // lockdep_set_novalidate_class(&dev->mutex);
    // spin_lock_init(&dev->devres_lock);
    // INIT_LIST_HEAD(&dev->devres_head);
    // device_pm_init(dev);
    // set_dev_node(dev, NUMA_NO_NODE);
    // INIT_LIST_HEAD(&dev->links.consumers);
    // INIT_LIST_HEAD(&dev->links.suppliers);
    // INIT_LIST_HEAD(&dev->links.defer_sync);
    // dev->links.status = DL_DEV_NO_DRIVER;
    // dev_assign_dma_coherent(dev, dma_default_coherent);
    // swiotlb_dev_init(dev);
    return KERRNO_SUCCESSES;
}

status_t device_create_groups_vargs(const dev_class_t *class, device_t *parent, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, bool set_init, va_list args, device_t **out)
{
    ENTER_FUNC("%p, %p, %u, %u, %p, %s, %s, 0, %p", class, parent, dev_size, dev_offset, drvdata, fmt, set_init BOOL_TO_STRING, out);
    device_t *dev = NULL;
    status_t ret = ENODEV;

    dev = kzmalloc(dev_size);
    if (!dev)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    if (dev_offset != -1 && dev_offset > 0)
    {
        device_t *dev_field = ((void *)dev + dev_offset);
        trace_debug(MODULE, "dev = %p", dev);
        trace_debug(MODULE, "dev_field = %p", dev_field);
        dev = dev_field;
    }

    device_initialize(dev);

    size_t size = 0;
    if (set_init)
    {
        size = vsprintf(dev->init_name, fmt, args);
        dev->init_name[size] = '\0';
    }
    else
    {
        size = vsnprintf(dev->name, DEVICE_NAME_MAX, fmt, args);
        dev->name[size] = '\0';
        dev->init_name = (char *)dev->name;
        dev_set_same_name(dev);
    }

    dev->class = class;
    dev->parent = parent;
    dev->drv_data = drvdata;
    dev->id = (~0);

    ret = device_add(dev);
    if (ret)
    {
        return ret;
    }

    *out = dev;
    return KERRNO_SUCCESSES;
}

void *core_device_create_size(const dev_class_t *class, device_t *parent, size_t dev_size, off_t dev_offset, void *drvdata, const char *fmt, ...)
{
    va_list vargs;
    device_t *dev;

    va_start(vargs, fmt);
    status_t ret = device_create_groups_vargs(class, parent, dev_size, dev_offset, drvdata, fmt, false, vargs, &dev);
    va_end(vargs);
    if (ret != KERRNO_SUCCESSES)
    {
        return NULL;
    }
    return dev;
}

device_t *core_device_create(const dev_class_t *class, device_t *parent, void *drvdata, const char *fmt, ...)
{
    va_list vargs;
    device_t *dev;

    va_start(vargs, fmt);
    status_t ret = device_create_groups_vargs(class, parent, sizeof(device_t), -1, drvdata, fmt, false, vargs, &dev);
    va_end(vargs);
    if (ret != KERRNO_SUCCESSES)
    {
        return NULL;
    }
    return dev;
}
