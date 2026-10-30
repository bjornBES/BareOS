/*
 * File: display.c
 * File Created: 24 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/display.h"

#include "dev/device.h"
#include "drivers/base/class.h"

#include "lists/list.h"

#include "init.h"

#include "math.h"
#include "memory.h"

#define MODULE "display"

typedef struct display_listeners
{
    display_listener_t cb;
    void *ctx;
    LIST_ENTRY_THIS();
} display_listener_data_t;

THIS_TO_TYPE_DEF(display_listener_data_t);

list_t listeners;
list_t display_list;

display_device_t *primary_display_dev;

status_t display_device_added(device_t *dev)
{
    return KERRNO_SUCCESSES;
}

static dev_class_t video_class = {
    .name = "video",
    .type = DEVICE_VIDEO,
    .device_added = display_device_added
};

status_t display_init()
{
    class_register(&video_class);
    list_initialize(&display_list);
    list_initialize(&listeners);
    return KERRNO_SUCCESSES;
}
ARCHDONE_INITCALL(display_init);

display_device_t *display_create()
{
    display_device_t *display_dev = kzmalloc(sizeof(display_device_t));
    return display_dev;
}


status_t display_register(display_device_t *display_dev)
{
    ADD_THIS_TO_LIST(display_list, display_dev);

    if (!primary_display_dev)
    {
        primary_display_dev = display_dev;
        display_set_default_mode(display_dev);
        display_notify_listeners(display_dev);
    }
    else
    {
        FUNC_NOT_IMPLEMENTED();
    }

    display_dev->dev.class = &video_class;


    dev_set_no_driver(&display_dev->dev);
    return device_register(&display_dev->dev);
}

status_t display_add_listener(display_listener_t cb, void *ctx)
{
    display_listener_data_t *listener = kzmalloc(sizeof(display_listener_data_t));
    listener->cb = cb;
    listener->ctx = ctx;
    return ADD_THIS_TO_LIST(listeners, listener);
}

status_t display_notify_listeners(display_device_t *display_dev)
{
    list_node_t *node = listeners.tail;
    while (node)
    {
        display_listener_data_t *listener = to_display_listener_data_t(node);
        
        status_t ret = listener->cb(display_dev, listener->ctx);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }

        node = node->prev;
    }
    return KERRNO_SUCCESSES;
}
