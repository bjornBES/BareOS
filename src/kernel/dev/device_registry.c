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

#include "drivers/base/class.h"

#include "drivers/base/driver.h"
#include "drivers/base/bus.h"

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

status_t device_register_under_dev_id(device_t *dev, dev_t device_id)
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

INTERNAL status_t device_internal_add(device_t *dev, list_t *list)
{
    ENTER_FUNC("%p, %p", dev, list);

    status_t ret = class_add_device(dev);
    KERRNO_CHECK_RETURN(ret);

    dev->id = dev_count;
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

    return ret;
}

INTERNAL status_t device_internal_assign_name(device_t *dev, uint32_t *class_id_out)
{
    if (dev->init_name != NULL)
    {
        uint32_t class_id = device_id_get_next_class_id(dev->init_name);
        if (dev_same_name(dev) == false)
        {
            snprintf(dev->name, DEVICE_NAME_MAX, "%s%u", dev->init_name, class_id);
        }
        else
        {
            snprintf(dev->name, DEVICE_NAME_MAX, "%s", dev->init_name);
        }
        if (class_id_out)
        {
            *class_id_out = class_id;
        }
    }
    else
    {
        log_err(MODULE, "class name is NULL for type %u", dev->class);
        return KERRNO_UNSUCCESS;
    }
    return KERRNO_SUCCESSES;
}

status_t device_add(device_t *dev)
{
    ENTER_FUNC("%p", dev);

    status_t ret = 0;
    ret = device_internal_assign_name(dev, NULL);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    if (dev->id == (~0))
    {
        dev->devt.minor = device_id_get_id(dev);
    }

    trace_debug(MODULE, "dev->class @ %p", dev->class);
    dev->devt.major = dev->class->type;
    dev->devt.device = 0;
    // dev->devt.id = MKDEV_KERNEL(dev->class->type, 0, dev->id);

    if (!dev->children.has_initialize)
    {
        list_initialize(&dev->children);
    }

    ret = device_internal_add(dev, NULL);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

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
    if (dev->class == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "device '%s' doesn't have a class", dev->init_name);
    }

    if (parent->id == (~0))
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "parent device '%s' has not been added", parent->init_name);
    }

    uint32_t class_id = 0;
    status_t ret = 0;
    ret = device_internal_assign_name(dev, &class_id);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    dev->devt.major = dev->class->type;
    dev->devt.minor = class_id + 1;
    dev->devt.device = parent->devt.device;
    dev->parent = parent;
    // dev->devt.id = MKDEV_KERNEL(dev->class, dev->id, parent->children.count);

    ret = ADD_THIS_TO_LIST(parent->children, dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    ret = device_internal_add(dev, &parent->children);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }

    return KERRNO_SUCCESSES;
}

status_t device_register(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    if (dev == NULL)
    {
        log_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }

    dev->id = (~0);

    trace_debug(MODULE, "dev->flags = 0x%x", dev->flags);

    if (dev_no_driver(dev) == false)
    {
        if (dev->driver == NULL)
        {
            // dont assign a name yet bc dev->init_name could be null
            // bc this comes from sources like the PCI bus or other busses where the name isn't known
            // it is the drivers job to set dev->init_name and then call device_add
            return bus_probe_device(dev);
        }
    }

    if (dev->class == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "device '%s' doesn't have a class", dev->init_name);
    }

    return device_add(dev);
}

status_t device_unregister(device_t *dev)
{
    if (!dev)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "argument 'dev' is NULL");
    }
    
    if (!dev->class)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "Device class is NULL");
    }

    if (dev_offline(dev) == true)
    {
        KERRNO_RETURN(EINPROGRESS, "");
    }
    dev_set_offline(dev);

    status_t ret = 0;

    device_t *found_node = NULL;
    list_node_t *curr = dev->class->devices.tail;
    while (curr && curr != &THIS_NODE(dev))
    {
        if (curr == &THIS_NODE(dev))
        {
            found_node = to_device_t(curr);
            break;
        }
        curr = curr->prev;
    }
    if (!found_node)
    {
        KERRNO_RETURN(ENXIO, "Device is no longer in the registry");
    }

    dev_set_offline_disabled(dev);

    list_node_t *child = dev->children.head;
    while (dev->children.count > 0)
    {
        child = list_pop_head(&dev->children);
        device_t *child_dev = to_device_t(child);
        ret |= device_unregister(child_dev);
    }


    if (dev->bus)
    {
        ret |= bus_remove_device(dev);
    }

    if (dev->ops.destroy)
    {
        ret |= dev->ops.destroy(dev);
    }

    ret |= class_remove_device(found_node);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return ret;
}

void device_debug_dev(device_t *parent_dev)
{
    // trace_info(MODULE, "device count: %u", dev->children.count);
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

            trace_info(MODULE, "\tdevice: %u.%u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p", parent_dev->devt.device, i, dev->id, major, device, minor, dev->class->name, dev->class->type, dev->name, dev->driver);
            dev_node = dev_node->prev;
            i++;
        }
    }
}

void device_debug()
{
    trace_info(MODULE, "device count: %u", dev_count);
    class_list_devices();
    /* for (size_t i = 0; i < DEVICE_TYPE_MAX; i++)
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

            trace_info(MODULE, "device: %u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p", i, dev->id, major, device, minor, dev->class->name, dev->class->type, dev->name, dev->driver);
            if (dev->children.has_initialize)
            {
                device_debug_dev(dev);
            }
            dev_node = dev_node->prev;
        }
    } */
}
