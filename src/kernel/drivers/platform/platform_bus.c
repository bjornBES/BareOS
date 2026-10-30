/*
 * File: platform_bus.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/platform/platform.h"

#include "drivers/base/bus.h"

#include "debug/debug.h"

#include "string.h"

#define MODULE "platform-bus"

status_t platform_bus_match(device_t *dev, driver_t *drv)
{
    ENTER_FUNC("%p, %p", dev, drv);
    platform_device_t *platform_dev = to_platform_dev(dev);
    platform_driver_t *platform_drv = to_platform_driver(drv);

    trace_debug(MODULE, "platform_dev->name = %p, drv->name = %p", platform_dev->name, drv->name);
    trace_debug(MODULE, "platform_dev->name = \"%s\", drv->name = \"%s\"", platform_dev->name, drv->name);

    if (strcmp(platform_dev->name, drv->name) == 0)
    {
        return KERRNO_SUCCESSES;
    }
    return KERRNO_UNSUCCESS;
}

status_t platform_bus_probe(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    platform_device_t *platform_dev = to_platform_dev(dev);
    platform_driver_t *platform_drv = to_platform_driver(dev->driver);

    status_t ret = 0;

    if (platform_drv->probe)
    {
        ret = platform_drv->probe(platform_dev);
    }

    if (ret != KERRNO_SUCCESSES)
    {
        platform_dev->driver = NULL;
        return ret;
    }
    return ret;
}

bus_type_t platform_bus_type = {
    .name = "platform",
    .driver_override = true,
    .match = platform_bus_match,
    .probe = platform_bus_probe,
};

status_t platform_register_bus()
{
    return bus_register(&platform_bus_type);
}

SUBSYS_INITCALL(platform_register_bus);