/*
 * File: class.c
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/base/class.h"

#include "dev/devicet.h"

#include "lists/hash.h"
#include "lists/list.h"

#include "kerrno.h"
#include "init.h"

#define MODULE "class"

hash_map_t *classes;
list_t class_list;

uint32_t class_ids[BITS_TO_U32(256)];

status_t class_register(const dev_class_t *class)
{
    ENTER_FUNC("%p (%s)", class, class->name);
    if (!class->device_added || !class->device_removed)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "'%s' class isn't set correctly", class->name);
    }
    status_t ret = hash_insert(classes, (char *)class->name, &class);
    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_IN_USE, "'%s' class name is already in use", class->name);
    }

    if (TEST_BIT(class->type, class_ids))
    {
        int id = class->type + 1;
        while (TEST_BIT(id, class_ids))
        {
            id++;
        }
        trace_info(MODULE, "switched class id for '%s' from %u to %u", class->name, class->type, id);
        ((dev_class_t *)class)->type = id;
    }

    SET_BIT(class->type, class_ids);

    ADD_THIS_TO_LIST(class_list, (dev_class_t *)class);
    list_initialize(&((dev_class_t *)class)->devices);
    return KERRNO_SUCCESSES;
}

status_t class_get(dev_type_t type, dev_class_t **class_out)
{
    if (!TEST_BIT(type, class_ids))
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "class type 0x%02x is not registered", type);
    }

    list_node_t *node = class_list.tail;
    while (node)
    {
        dev_class_t *class = container_of(node, dev_class_t, LIST_ENTRY_NAME);
        if (class->type == type)
        {
            *class_out = class;
            break;
        }
        node = node->prev;
    }
    return KERRNO_SUCCESSES;
}

status_t class_add_device(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    // hash_node_t node;
    // status_t ret = hash_search(classes, (char *)dev->class->name, &node);
    // if (ret != KERRNO_SUCCESSES)
    // {
    //     inline_asm("int3");
    //     return KERRNO_UNSUCCESS;
    // }
    dev_class_t *class = (dev_class_t *)dev->class;
    trace_debug(MODULE, "class = %p, class->devices = %p", class, &class->devices);
    status_t ret = ADD_THIS_TO_LIST(class->devices, dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        // TODO
        return ret;
    }
    ret = class->device_added(dev);
    KERRNO_CHECK_RETURN(ret);
    return ret;
}

status_t class_remove_device(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    dev_class_t *class = (dev_class_t *)dev->class;
    trace_debug(MODULE, "class = %p, class->devices = %p", class, &class->devices);
    status_t ret = 0;

    ret = list_remove(&class->devices, &THIS_NODE(dev));
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        // TODO
        return ret;
    }

    ret = class->device_removed(dev);
    KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        // TODO
        return ret;
    }

    return ret;
}

INTERNAL status_t class_list_class(const dev_class_t *class)
{
    // ENTER_FUNC("%p", class);
    list_node_t *node = class->devices.head;
    // trace_debug(MODULE, "node = %p", node);
    int i = 0;
    while (node)
    {
        device_t *dev = to_device_t(node);
        uint64_t major = dev->devt.major;
        uint64_t device = dev->devt.device;
        uint64_t minor = dev->devt.minor;
        trace_info(MODULE, "device: %u (%p), id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p, parent: %p", i, dev, dev->id, major, device, minor, dev->class->name, dev->class->type, dev->name, dev->driver, dev->parent);
        // hexdump(dev->name, sizeof(dev->name), 16);
        node = node->next;
        i++;
    }
    return KERRNO_SUCCESSES;
}

status_t class_list_devices()
{
    ENTER_FUNC("", 0);
    list_node_t *node = class_list.tail;
    while (node)
    {
        const dev_class_t *class = container_of(node, dev_class_t, LIST_ENTRY_NAME);
        class_list_class(class);
        node = node->prev;
    }
    return KERRNO_SUCCESSES;
}

status_t class_init()
{
    list_initialize(&class_list);
    return hash_map_create(&classes, 256);
}

CORE_INITCALL(class_init);

status_t class_find_device(const dev_class_t *class, const device_t *start, const void *data, device_match_t match, device_t **out_dev)
{
    device_t *dev;
    list_node_t *node = class->devices.tail;
    if (start)
    {
        node = &THIS_NODE((device_t *)start);
    }
    while (node)
    {
        dev = to_device_t(node);
        if (match(dev, data))
        {
            break;
        }
        node = node->prev;
    }
    *out_dev = dev;
    return KERRNO_SUCCESSES;
}
