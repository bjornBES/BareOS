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

status_t class_register(dev_class_t *class)
{
    ENTER_FUNC("%p", class);
    if (!class->device_added)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "'%s' class isn't set correctly", class->name);
    }
    status_t ret = hash_insert(classes, (char *)class->name, &class);
    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_NAME_IN_USE, "'%s' class name is already in use", class->name);
    }
    ADD_THIS_TO_LIST(class_list, class);
    list_initialize(&class->devices);
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
    dev_class_t *class = dev->class;
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

INTERNAL status_t class_list_class(dev_class_t *class)
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
        trace_info(MODULE, "device: %u, id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p, parent: %p", i, dev->id, major, device, minor, dev->class->name, dev->class->type, dev->name, dev->driver, dev->parent);
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
        dev_class_t *class = container_of(node, dev_class_t, LIST_ENTRY_NAME);
        class_list_class(class);
        node = node->prev;
    }
    return KERRNO_SUCCESSES;
}

status_t class_init()
{
    list_initialize(&class_list);
    return hash_map_create(&classes, 50);
}

POSTCORE_INITCALL(class_init);
