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

typedef struct device device_t;

typedef struct dev_class
{

} dev_class_t;


typedef struct driver
{
    const char *name;
    // dev_class_t class; // what kind of device this driver handles

    bus_type_t *bus;
    size_t match_count;

    status_t (*probe)(device_t *dev);   // called when a match is found, driver decides accept/reject
    void (*remove)(device_t *dev);      // hot-unplug / teardown
    status_t (*suspend)(device_t *dev); // optional, power state transitions
    status_t (*resume)(device_t *dev);

    list_node_t node; // linkage into the global driver registry
} driver_t;
