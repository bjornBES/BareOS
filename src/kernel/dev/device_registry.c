/*
 * File: device_registry.c
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "dev/device.h"
#include "dev/device_id.h"

#include "drivers/driver.h"
#include "drivers/bus.h"

#include "lists/list.h"

#include "kerrno.h"

#include "stdio.h"
#include "memory.h"

#include <types.h>
#include <array.h>

#define MODULE             "DEVICE-REGISTRY"

#define MAX_DEVICE_CLASSES 32

list_t device_lists[DEVICE_TYPE_MAX] = {0};
int dev_count;

status_t device_registry_init()
{
    for (size_t i = 0; i < ARRAY_SIZE(device_lists); i++)
    {
        list_initialize(&device_lists[i]);
    }
    return KERRNO_SUCCESSES;
}

EARLY_INITCALL(device_registry_init);

int device_register_under_dev_id(device_t *dev, dev_t device_id)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL; // TODO
    }

    trace_info(MODULE, "device is a partition of %s", dev->init_name);
    uint32_t class_id = device_id_get_next_class_id(dev->init_name);
    snprintf(dev->name, DEVICE_NAME_MAX, "%sp%u", dev->init_name, class_id);
    uint64_t raw_device = DEVICE(device_id);
    dev->devt.id = MKDEV_KERNEL(dev->class, raw_device, class_id);

    // dev->siblings = device_lists[dev->class];
    // device_lists[dev->class] = dev;

    uint64_t major = dev->devt.major;
    uint64_t minor = dev->devt.minor;
    trace_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u)", dev->name, dev->class, dev->id, major, raw_device, minor);
    dev_count++;

    /*     switch (dev->class)
        {
            case DEVICE_TTY :
            case DEVICE_CHAR : // DEVICE_SERIAL?
                devfs_create_entry(dev, DT_CHR);
                break;
            case DEVICE_BLOCK :
                devfs_create_entry(dev, DT_CHR);
                break;
            default :
                break; // not everything needs a /dev/ entry
        } */

    return KERRNO_SUCCESSES;
}

status_t device_register_child(device_t *dev, device_t *parent)
{
    ENTER_FUNC("%p, %p", dev, parent);
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }
    if (parent == NULL)
    {
        log_crit(MODULE, "Parent device is null");
        return -EINVAL;
    }

    if (parent->id == (~0))
    {
        parent->id = device_id_get_id(parent);
        parent->devt.device = parent->id;
    }

    if (dev->init_name != NULL)
    {
        uint32_t class_id = device_id_get_next_class_id(dev->init_name);
        snprintf(dev->name, DEVICE_NAME_MAX, "%s%u", dev->init_name, class_id);
    }
    else
    {
        log_err(MODULE, "class name is NULL for type %u", dev->class);
        return KERRNO_UNSUCCESS;
    }

    if (!parent->children.has_initialize)
    {
        list_initialize(&parent->children);
    }
    ADD_THIS_TO_LIST(parent->children, dev);

    dev->id = parent->id;
    dev->devt.device = dev->id;
    dev->devt.major = dev->class;
    dev->devt.minor = parent->children.count;
    dev->devt.id = MKDEV_KERNEL(dev->class, dev->id, parent->children.count);

    uint64_t major = dev->devt.major;
    uint64_t device = dev->devt.device;
    uint64_t minor = dev->devt.minor;
    trace_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u/0x%016x)", dev->name, dev->class, dev->id, major, device, minor, dev->devt.id);
    dev_count++;

    /*     switch (dev->class)
        {
            case DEVICE_TTY :
            case DEVICE_CHAR : // DEVICE_SERIAL?
                devfs_create_entry(dev, DT_CHR);
                break;
            case DEVICE_BLOCK :
                get_next_class_id(dev->name);
                devfs_create_entry(dev, DT_CHR);
                break;
            default :
                break; // not everything needs a /dev/ entry
        } */

    return KERRNO_SUCCESSES;
}

status_t device_register(device_t *dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }
    dev->id = (~0);

    if (dev->driver == NULL)
    {
        bus_probe_device(dev);
    }

    if (dev->init_name != NULL)
    {
        uint32_t class_id = device_id_get_next_class_id(dev->init_name);
        snprintf(dev->name, DEVICE_NAME_MAX, "%s%u", dev->init_name, class_id);
    }
    else
    {
        log_err(MODULE, "class name is NULL for type %u", dev->class);
        return KERRNO_UNSUCCESS;
    }

    if (dev->id == (~0))
    {
        dev->id = device_id_get_id(dev);
        dev->devt.device = dev->id;
    }

    dev->devt.major = dev->class;
    dev->devt.minor = 0;
    dev->devt.id = MKDEV_KERNEL(dev->class, dev->id, 0);

    ADD_THIS_TO_LIST(device_lists[dev->class], dev);

    uint64_t major = dev->devt.major;
    uint64_t device = dev->devt.device;
    uint64_t minor = dev->devt.minor;
    trace_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u/0x%016x)", dev->name, dev->class, dev->id, major, device, minor, dev->devt.id);
    dev_count++;

    /*     switch (dev->class)
        {
            case DEVICE_TTY :
            case DEVICE_CHAR : // DEVICE_SERIAL?
                devfs_create_entry(dev, DT_CHR);
                break;
            case DEVICE_BLOCK :
                get_next_class_id(dev->name);
                devfs_create_entry(dev, DT_CHR);
                break;
            default :
                break; // not everything needs a /dev/ entry
        } */

    return KERRNO_SUCCESSES;
}

void device_unregister(device_t *dev)
{
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return;
    }

    list_node_t *curr = device_lists[dev->class].tail;
    while (curr && curr != &THIS_NODE(dev))
    {
        if (curr == &THIS_NODE(dev))
        {
            break;
        }
        curr = curr->prev;
    }
    if (curr == &THIS_NODE(dev))
    {
        list_remove(&device_lists[dev->class], curr);
        kfree(dev);
    }
}

void device_debug_dev(device_t *parent_dev)
{
    // trace_info(MODULE, "device count: %u", dev->children.count);
    char *DEVICE_TYPE_STRING[DEVICE_TYPE_MAX] = {
        [DEVICE_UNKNOWN] = "UNKNOWN",
        [DEVICE_SERIAL] = "SERIAL",
        [DEVICE_BLOCK] = "BLOCK",
        [DEVICE_CHAR] = "CHAR",
        [DEVICE_TTY] = "TTY",
        [DEVICE_NET] = "NET",
        [DEVICE_VIDEO] = "VIDEO",
        [DEVICE_TIMER] = "TIMER",
        [DEVICE_PSEUDO] = "PSEUDO",
        [DEVICE_VIRTUAL] = "VIRTUAL",
    };
    int i = 0;
    {
        list_node_t *dev_node = parent_dev->children.tail;
        while (dev_node)
        {
            if (dev_node == NULL)
            {
                trace_info(MODULE, "device: %u, NULL", i);
                continue;
            }

            device_t *dev = to_device_t(dev_node);

            uint64_t major = dev->devt.major;
            uint64_t device = dev->devt.device;
            uint64_t minor = dev->devt.minor;

            trace_info(MODULE, "\tdevice: %u.%u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, ops:%p, driver:%p", parent_dev->devt.device, i, dev->id, major, device, minor, DEVICE_TYPE_STRING[dev->class], dev->class, dev->name, dev->ops, dev->driver);
            dev_node = dev_node->prev;
            i++;
        }
    }
}

void device_debug()
{
    trace_info(MODULE, "device count: %u", dev_count);
    char *DEVICE_TYPE_STRING[DEVICE_TYPE_MAX] = {
        [DEVICE_UNKNOWN] = "UNKNOWN",
        [DEVICE_SERIAL] = "SERIAL",
        [DEVICE_BLOCK] = "BLOCK",
        [DEVICE_CHAR] = "CHAR",
        [DEVICE_TTY] = "TTY",
        [DEVICE_NET] = "NET",
        [DEVICE_VIDEO] = "VIDEO",
        [DEVICE_TIMER] = "TIMER",
        [DEVICE_PSEUDO] = "PSEUDO",
        [DEVICE_VIRTUAL] = "VIRTUAL",
    };
    for (size_t i = 0; i < DEVICE_TYPE_MAX; i++)
    {
        list_node_t *dev_node = device_lists[i].tail;
        while (dev_node)
        {
            if (dev_node == NULL)
            {
                trace_info(MODULE, "device: %u, NULL", i);
                continue;
            }

            device_t *dev = to_device_t(dev_node);

            uint64_t major = dev->devt.major;
            uint64_t device = dev->devt.device;
            uint64_t minor = dev->devt.minor;

            trace_info(MODULE, "device: %u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, ops:%p, driver:%p", i, dev->id, major, device, minor, DEVICE_TYPE_STRING[dev->class], dev->class, dev->name, dev->ops, dev->driver);
            if (dev->children.has_initialize)
            {
                device_debug_dev(dev);
            }
            dev_node = dev_node->prev;
        }
    }
}
