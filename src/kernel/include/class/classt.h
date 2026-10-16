/*
 * File: classt.h
 * File Created: 15 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "lists/list_node.h"

#include <types.h>

typedef enum dev_type
{
    DEVICE_UNKNOWN = 0,
    DEVICE_CHAR,   // keyboards, serial, mice — byte stream
    DEVICE_SERIAL,
    DEVICE_BLOCK,  // disks, partitions — random access blocks
    DEVICE_TTY,    // TTY devices
    DEVICE_NET,    // network interfaces
    DEVICE_VIDEO,  // framebuffer
    DEVICE_TIMER,
    DEVICE_PSEUDO, // /dev/null, /dev/zero, /dev/random
    DEVICE_VIRTUAL,
    DEVICE_PCI,
    DEVICE_TYPE_MAX,
} dev_type_t;

typedef struct device device_t;

typedef struct dev_class
{
    const char *name;  // "block", "timer"
    dev_type_t type;   // keep the old enum here, so major = class->type
    list_t devices;    // every device in this class
    LIST_ENTRY_THIS(); // link in the class registry

    // hooks the registry calls
    status_t (*device_added)(device_t *dev);
    void (*device_removed)(device_t *dev);
} dev_class_t;
