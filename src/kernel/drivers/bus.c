/*
 * File: bus.c
 * File Created: 25 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/bus.h"
#include "drivers/driver.h"
#include "drivers/dd.h"

#include "lists/list.h"

#include "init.h"

#include "kerrno.h"

#define MODULE "bus"

list_t bus_registry;

__init status_t bus_initialize()
{
    list_initialize(&bus_registry);
    return KERRNO_SUCCESSES;
}

EARLY_INITCALL(bus_initialize);

status_t bus_register(bus_type_t *bus)
{
    status_t ret = list_push_tail(&bus_registry, &bus->node);
    if (ret != KERRNO_SUCCESSES)
    {
        return KERRNO_UNSUCCESS;
    }
    return KERRNO_SUCCESSES;
}

status_t bus_for_each_driver(bus_type_t *bus, driver_t *curr, void *data, status_t (*func)(driver_t *, void *))
{
    ENTER_FUNC("%p, %p, %p, %p", bus, curr, data, func);
    status_t state;
    do
    {
        state = driver_next(&curr, bus);
        trace_debug(MODULE, "curr = %p", curr);
        trace_debug(MODULE, "state = %u", state);
        if (state != KERRNO_SUCCESSES || curr == NULL)
        {
            break;
        }
        state = func(curr, data);
    }  while (state != KERRNO_SUCCESSES);
    return state;
}

status_t bus_probe_device(device_t *dev)
{
    device_initial_probe(dev);

/*     dev->bus->probe(dev);
    trace_info(MODULE, "device %p needs a driver", dev);
    driver_t *drv = driver_get(dev->bus);
    if (drv != NULL)
    {
        trace_debug(MODULE, "got driver %p", drv);
        trace_debug(MODULE, "got driver %s", drv->name);
        if (dev->bus->match(dev, drv) == KERRNO_SUCCESSES)
        {
            driver_call_probe();
            drv->probe(dev);
            return KERRNO_UNSUCCESS;
        }
    } */

    return KERRNO_SUCCESSES;
}
