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
#include "string.h"

#include "memory.h"

#include <types.h>
#include <array.h>

#define MODULE             "device-registry"

#define MAX_DEVICE_CLASSES 32

list_t device_lists[DEVICE_TYPE_MAX] = {0};
int dev_count;

#define LOCK_NAME registry_lock
DEF_LOCK(LOCK_NAME);

status_t device_registry_init()
{
    for (size_t i = 0; i < ARRAY_SIZE(device_lists); i++)
    {
        list_initialize(&device_lists[i]);
    }
    return KERRNO_SUCCESS;
}

CONSTRUCTOR_INITCALL(device_registry_init);

INTERNAL status_t device_internal_add(device_t *dev)
{
    LOCK();
    ENTER_FUNC("%p", dev);

    status_t ret = class_add_device(dev);
    if (ret)
    {
        UNLOCK();
        LOG_RETURN(ret);
    }

    dev->id = dev_count;
    uint64_t major = dev->devt.major;
    uint64_t res = dev->devt.res;
    uint64_t device = dev->devt.device;
    uint64_t minor = dev->devt.minor;
    trace_info(MODULE, "registered %s (type=%d id=%d dev_id=%u:%u:%u:%u/0x%016llx)", dev->name, dev->class, dev->id, major, res, device, minor, dev->devt.id);
    dev_count++;

    UNLOCK();
    return ret;
}

// INTERNAL status_t device_internal_assign_name(device_t *dev, uint32_t *class_id_out)
// {
//     ENTER_FUNC("%p, %p", dev, class_id_out);
//     if (dev->init_name != NULL)
//     {
//         uint32_t class_id = device_id_get_next_class_id(dev->init_name);
//         size_t s = 0;
//         // s = snprintf(dev->name, DEVICE_NAME_MAX, "%sCLS", dev->init_name);
//         s = snprintf(dev->name, DEVICE_NAME_MAX, "%s", dev->init_name);
//         if (class_id_out)
//         {
//             *class_id_out = class_id;
//         }
//         dev->name[s] = '\0';
//     }
//     else
//     {
//         trace_err(MODULE, "class name is NULL for type %u", dev->class);
//         return KERRNO_UNSUCCESS;
//     }
//     return KERRNO_SUCCESS;
// }

INTERNAL status_t device_lock(device_t *dev)
{
    spinlock_acquire(&dev->dev_spinlock);
    return KERRNO_SUCCESS;
}

INTERNAL status_t device_unlock(device_t *dev)
{
    spinlock_release(&dev->dev_spinlock);
    return KERRNO_SUCCESS;
}

status_t device_register_id(device_t *dev, device_info_t *info)
{
    ENTER_FUNC("%p, %p", dev, info);
    if (!dev || !info)
    {
        KERRNO_RETURN(KERRNO_BAD_ARGUMENT);
    }

    char *debug_name = info->init_name;
    if (debug_name)
    {
        if (*debug_name == '\0')
        {
            debug_name = dev->name;
        }
    }
    else
    {
        debug_name = info->name;
    }

    if (!dev->class)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "dev->class is NULL");
    }

    if (info->dev_id.major != dev->class->type)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "class major(%u) and dev_id major(%u) doesn't match for %s", dev->class->type, info->dev_id.major, debug_name);
    }

    status_t ret = class_find_device_by_ref(dev->class, dev, NULL);
    {
        switch (ret)
        {
            case KERRNO_DEV_NOT_FOUND :
                break;

            case KERRNO_SUCCESS :
                KERRNO_RETURN(KERRNO_DEV_ALREADY_EXISTS)

            default :
                LOG_RETURN(ret);
        }
    }

    if (info->parent)
    {
        ret = class_find_device_by_devt(info->parent->class, info->parent->devt.id, NULL);
        KERRNO_CHECK_AND_RETURN(ret);
    }

    device_lock(dev);

    if (!info->driver)
    {
        if (!dev_no_driver(dev))
        {
            device_unlock(dev);
            // dont assign a name yet bc that is the drivers and buses job to do so
            // the drivers and or buses job to set dev->devt, dev->init_name,
            // and then call a register function.
            ret = bus_probe_device(dev);
            KERRNO_CHECK_AND_RETURN(ret);
            return KERRNO_SUCCESS;
        }
    }
    else
    {
        dev->driver = info->driver;
    }

    info->dev_id.major = dev->class->type;
    if (info->dev_id.device == DEV_AUTO_DEV)
    {
        if (!info->parent)
        {
            uint32_t id;
            ret = class_get_next_device(dev->class, &id);
            KERRNO_CHECK_AND_RETURN(ret);
            info->dev_id.device = id;
        }
        else
        {
            info->dev_id.device = info->parent->devt.device;
        }
    }
    if (info->dev_id.minor == DEV_AUTO_MIN)
    {
        uint32_t id;
        ret = class_get_next_minor(dev->class, info->dev_id, &id);
        KERRNO_CHECK_AND_RETURN(ret);
        info->dev_id.minor = id;
    }

    dev->init_name = info->init_name;
    dev->devt.id = info->dev_id.id;

    if (dev->init_name)
    {
        if (*dev->init_name != '\0')
        {
            size_t s = strnlen(dev->init_name, DEVICE_NAME_MAX);
            memcpy(dev->name, dev->init_name, s);
            dev->name[s] = '\0';
        }
    }
    else
    {
        memcpy(dev->name, info->name, DEVICE_NAME_MAX);
    }

    if (info->parent)
    {
        dev->parent = info->parent;
        ret = ADD_THIS_TO_LIST(info->parent->children, dev);
        if (ret)
        {
            device_unlock(info->parent);
            LOG_RETURN(ret);
        }
        device_get(dev->parent);
    }
    ret = device_internal_add(dev);
    if (ret)
    {
        device_unlock(info->parent);
        LOG_RETURN(ret);
    }
    device_unlock(info->parent);
    return KERRNO_SUCCESS;
}

/*
status_t device_add(device_t *dev)
{
    device_lock(dev);

    ENTER_FUNC("%p", dev);

    status_t ret = 0;
    uint32_t class_id;
    ret = device_internal_assign_name(dev, &class_id);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESS)
    {
        device_unlock(dev);
        return ret;
    }

    // if (dev->id == (~0))
    {
        dev->devt.minor = device_id_get_id(dev);
    }

    trace_debug(MODULE, "dev->class @ %p", dev->class);
    dev->devt.major = dev->class->type;
    dev->devt.device = class_id;
    // dev->devt.id = MKDEV_KERNEL(dev->class->type, 0, dev->id);

    if (!dev->children.has_initialize)
    {
        list_initialize(&dev->children);
    }

    ret = device_internal_add(dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESS)
    {
        device_unlock(dev);
        return ret;
    }
    device_unlock(dev);

    return KERRNO_SUCCESS;
}

status_t device_register_child(device_t *dev, device_t *parent)
{
    device_lock(dev);

    ENTER_FUNC("%p, %p", dev, parent);

    if (dev == NULL)
    {
        device_unlock(dev);
        trace_crit(MODULE, "Device is null try again");
        return -EINVAL;
    }
    if (parent == NULL)
    {
        device_unlock(dev);
        trace_crit(MODULE, "Parent device is null");
        return -EINVAL;
    }
    if (dev->class == NULL)
    {
        device_unlock(dev);
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
    if (ret != KERRNO_SUCCESS)
    {
        device_unlock(dev);
        return ret;
    }

    dev->devt.major = dev->class->type;
    dev->devt.minor = class_id + 1;
    dev->devt.device = parent->devt.device;
    dev->parent = parent;
    // dev->devt.id = MKDEV_KERNEL(dev->class, dev->id, parent->children.count);

    ret = ADD_THIS_TO_LIST(parent->children, dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESS)
    {
        device_unlock(dev);
        return ret;
    }

    ret = device_internal_add(dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESS)
    {
        device_unlock(dev);
        return ret;
    }

    device_unlock(dev);
    return KERRNO_SUCCESS;
}

status_t device_register(device_t *dev)
{
    device_lock(dev);
    ENTER_FUNC("%p", dev);
    if (dev == NULL)
    {
        device_unlock(dev);
        trace_crit(MODULE, "Device is null try again");
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
            device_unlock(dev);
            return bus_probe_device(dev);
        }
    }

    if (dev->class == NULL)
    {
        device_unlock(dev);
        KERRNO_RETURN(KERRNO_BAD_VALUE, "device '%s' doesn't have a class", dev->init_name);
    }

    device_unlock(dev);

    return device_add(dev);
}
*/
status_t device_release(device_t *dev)
{
    status_t ret = 0;

    if (dev->driver)
    {
        KERRNO_RETURN(KERRNO_BAD_ARGUMENT);
    }
    if (dev->registered)
    {
        KERRNO_RETURN(KERRNO_BAD_ARGUMENT);
    }

    if (dev->parent)
    {
        ret = device_put(dev->parent);
        if (ret)
        {
            LOG_RETURN(ret);
        }
    }

    if (dev->ops.release)
    {
        ret = dev->ops.release(dev);
    }
    else if (dev->type->release)
    {
        ret = dev->type->release(dev);
    }
    else if (dev->class->release_dev)
    {
        ret = dev->class->release_dev(dev);
    }
    else
    {
        KERRNO_RETURN(KERRNO_DONT_DO_THAT);
    }
    if (ret)
    {
        LOG_RETURN(ret);
    }
    return ret;
}

status_t device_get(device_t *dev)
{
    dev->refcount++;
    return KERRNO_SUCCESS;
}

status_t device_put(device_t *dev)
{
    dev->refcount--;
    if (dev->refcount == 0)
    {
        return device_release(dev);
    }
    return KERRNO_SUCCESS;
}

status_t device_kill(device_t *dev)
{
    if (dev_offline(dev) == true)
    {
        KERRNO_RETURN(EINPROGRESS);
    }
    dev_set_offline(dev);
    if (dev->dead)
    {
        UNLOCK();
        KERRNO_RETURN(EINPROGRESS, "");
    }
    dev->dead = 1;
    return KERRNO_SUCCESS;
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

    status_t ret = 0;
    ret = device_kill(dev);
    if (ret)
    {
        dev_assign_offline(dev, false);
        dev->dead = 0;
        return ret;
    }

    {
        device_t *found_node = NULL;
        ret = class_find_device_by_devt(dev->class, dev->devt.id, &found_node);

        if (ret == KERRNO_NOT_INITIALIZED)
        {
            LOG_RETURN(ret);
        }

        if (ret)
        {
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
        }
        if (!found_node)
        {
            KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "Device is no longer in the registry");
        }
    }

    dev_set_offline_disabled(dev);

    LOCK();
    list_node_t *child = dev->children.head;
    while (dev->children.count > 0)
    {
        child = list_pop_head(&dev->children);
        device_t *child_dev = to_device_t(child);
        ret = device_unregister(child_dev);
        if (ret)
        {
            UNLOCK();
            LOG_RETURN(ret);
        }
        ret = device_put(dev);
        if (ret)
        {
            UNLOCK();
            LOG_RETURN(ret);
        }
    }
    UNLOCK();

    if (dev->driver)
    {
        ret = driver_unbind_dev(dev->driver, dev);
        if (ret)
        {
            LOG_RETURN(ret);
        }
    }

    if (dev->bus)
    {
        ret = bus_remove_device(dev);
        if (ret)
        {
            LOG_RETURN(ret);
        }
    }

    LOCK();
    ret = class_remove_device(dev);
    if (ret)
    {
        LOG_RETURN(ret);
    }
    dev->registered = 0;
    UNLOCK();

    ret = device_put(dev);

    if (ret != KERRNO_SUCCESS)
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

void device_print_debug(device_t *dev)
{
    uint64_t major = dev->devt.major;
    uint64_t device = dev->devt.device;
    uint64_t minor = dev->devt.minor;

    trace_info(MODULE, "device: 0, id:0x%x, dev_id=%u:%u:%u, name:%s, driver:%p", dev->id, major, device, minor, dev->name, dev->driver);
    trace_info(MODULE, "device: 0, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p", dev->id, major, device, minor, dev->class->name, dev->class->type, dev->name, dev->driver);
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

status_t device_get_device(device_id_t *id, device_t **out_dev)
{
    ENTER_FUNC("%p, %p", id, out_dev);
    device_id_print(id);
    dev_class_t *class;
    status_t ret = class_get(id->major, &class);
    KERRNO_CHECK_RETURN(ret);
    if (ret)
    {
        return ret;
    }
    list_node_t *node = class->devices.head;
    while (node)
    {
        device_t *dev = to_device_t(node);
        if (dev->devt.id == id->id)
        {
            *out_dev = dev;
        }
        node = node->next;
    }
    if (*out_dev == NULL)
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "class '%s' did not have id %u:%u:%u", class->name, id->major, id->device, id->minor);
    }
    return KERRNO_SUCCESS;
}
