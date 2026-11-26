/*
 * File: core_types.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "lists/list_node.h"
#include "lists/hasht.h"

#include "idalloc/ida/idat.h"

#include <types.h>

typedef enum dev_type
{
    DEVICE_UNKNOWN = 0,
    DEVICE_CHAR,   // keyboards, serial, mice — byte stream
    DEVICE_BLOCK,  // disks, partitions — random access blocks
    DEVICE_TTY,    // TTY devices
    DEVICE_SERIAL,
    DEVICE_SERIAL_IO,
    DEVICE_INPUT,
    DEVICE_NET,    // network interfaces
    DEVICE_VIDEO,  // framebuffer
    DEVICE_TIMER,
    DEVICE_PCI,
    DEVICE_CONSOLE,


    DEVICE_VIRTUAL,
    DEVICE_PSEUDO, // /dev/null, /dev/zero, /dev/random
    DEVICE_TYPE_MAX,
} dev_type_t;

typedef struct device device_t;

typedef struct dev_class
{
    LIST_ENTRY_THIS(); // link in the class registry
    
    list_t devices;    // every device in this class
    
    const char *name;  // "block", "timer"
    dev_type_t type;   // keep the old enum here, so major = class->type

    refcount_t refcount;

    hash_map_t *dev_id_group_minor;
    ida_t *ida_device;

    // hooks the registry calls
    /// @brief called when the device is added to this class
    status_t (*device_added)(struct device *dev);

    /// @brief called when the device is removed from this class
    status_t (*device_removed)(struct device *dev);

    /// @brief called to release the device
    status_t (*release_dev)(struct device *dev);
} dev_class_t;

struct driver;

typedef struct bus_type
{
    LIST_ENTRY_THIS();
    list_node_t node; // linkage into the global driver registry
    const char *name;

    status_t (*match)(struct device *dev, struct driver *drv);

    status_t (*probe)(struct device *dev);
    
    /// @brief called when the device is removed from this bus.
    status_t (*remove)(struct device *dev);

    // status_t (*suspend)(device_t *dev);
    // status_t (*resume)(device_t *dev);

    bool driver_override;
} bus_type_t;

typedef struct driver
{
    LIST_ENTRY_THIS();
    const char *name;

    bus_type_t *bus;
    size_t match_count;

    status_t (*probe)(struct device *dev);   // called when a match is found, driver decides accept/reject

    /// @brief called when the device is removed from the system.
    status_t (*remove)(struct device *dev);  // hot-unplug / teardown
    status_t (*suspend)(struct device *dev); // optional, power state transitions
    status_t (*resume)(struct device *dev);

    list_node_t node; // linkage into the global driver registry
} driver_t;