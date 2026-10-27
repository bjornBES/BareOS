/*
 * File: bus.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "bust.h"

status_t bus_initialize();

status_t bus_for_each_driver(bus_type_t *bus, driver_t *start, void *data, status_t (*func)(driver_t *, void *));

status_t bus_register(bus_type_t *bus);

status_t bus_probe_device(device_t *dev);