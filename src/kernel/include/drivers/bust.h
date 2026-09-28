/*
 * File: bust.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "lists/list_node.h"

#include <types.h>

typedef struct device device_t;
typedef struct driver driver_t;

typedef struct bus_type
{
    list_node_t node; // linkage into the global driver registry
    const char *name;

    status_t (*match)(device_t *dev, driver_t *drv);

    status_t (*probe)(device_t *dev);

    // status_t (*suspend)(device_t *dev);
    // status_t (*resume)(device_t *dev);

    bool driver_override;
} bus_type_t;
