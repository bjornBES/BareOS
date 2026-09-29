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

#include "drivers/drivert.h"
#include "module.h"

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

status_t drivers_initialize();

status_t drivers_register(driver_t *dri);

driver_t *driver_get(bus_type_t *bus);
status_t driver_next(driver_t **drv, bus_type_t *bus);
