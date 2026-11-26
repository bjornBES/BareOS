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

#include "debug/debug.h"

#include "lists/list.h"

#include "stdio.h"

#include "memory.h"

#define MODULE "device"

device_t *device_create()
{
    device_t *device = kmalloc(sizeof(device_t));
    memset(device, 0, sizeof(device_t));
    return device;
}

status_t device_initialize(device_t *dev)
{
    list_initialize(&dev->children);
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
    return KERRNO_SUCCESS;
}

status_t device_set_name(device_t *dev, const char *fmt, ...)
{
    va_list vargs;
    status_t err = KERRNO_SUCCESS;

    va_start(vargs, fmt);
    memset(dev->name, 0, sizeof(dev->name));
    size_t s = vsprintf(dev->name, fmt, vargs);
    dev->name[s] = '\0';
    // hexdump(dev->name, sizeof(dev->name), 16);
    dev_set_same_name(dev);
    dev->init_name = dev->name;
    dev->init_name[s] = '\0';
    // err = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
    va_end(vargs);
    return err;
}

status_t device_open(device_t *dev, int flags)
{
    ENTER_FUNC("%p, 0x%x", dev, flags);
    if (!dev->ops.open)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }

    vfs_node_t node;
    status_t ret = dev->ops.open(dev, flags, &node);
    return ret;
}
