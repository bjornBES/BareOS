/*
 * File: platform.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/platform/platform.h"

#include "drivers/base/driver.h"

#include "dev/device.h"

#include "kerrno.h"

#define MODULE "platform"

extern bus_type_t platform_bus_type;

status_t platform_device_register(platform_device_t *dev)
{
    dev->dev.bus = &platform_bus_type;
    dev_set_ready_to_probe(&dev->dev);
    return device_register(&dev->dev);
}

status_t platform_register_driver(platform_driver_t *driver)
{
    driver->driver.bus = &platform_bus_type;
    return drivers_register(&driver->driver);
}

status_t platform_unregister_driver(platform_driver_t *driver)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}
