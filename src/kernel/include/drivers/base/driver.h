/*
 * File: driver.h
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/base/drivert.h"

status_t drivers_initialize();

status_t drivers_register(driver_t *dri);

driver_t *driver_get(bus_type_t *bus);
status_t driver_next(driver_t **drv, bus_type_t *bus);
