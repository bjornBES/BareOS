/*
 * File: drivert.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#include "lists/list_node.h"

#include "bust.h"

#include "module.h"

typedef struct device device_t;

typedef struct driver
{
    const char *name;

    bus_type_t *bus;
    size_t match_count;

    status_t (*probe)(device_t *dev);   // called when a match is found, driver decides accept/reject
    status_t (*remove)(device_t *dev);  // hot-unplug / teardown
    status_t (*suspend)(device_t *dev); // optional, power state transitions
    status_t (*resume)(device_t *dev);

    list_node_t node; // linkage into the global driver registry
} driver_t;

#define MODULE_DRIVER(__driver, __register, __unregister, ...) \
    static status_t __init __driver##_init()                   \
    {                                                          \
        return __register(&(__driver));                        \
    }                                                          \
    MODULE_INIT(__driver##_init)

// static void __exit __driver##_exit()
// {
// __unregister(&(__driver) , ##__VA_ARGS__);
// }
// module_exit(__driver##_exit);
