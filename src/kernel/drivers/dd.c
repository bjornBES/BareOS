/*
 * File: dd.c
 * File Created: 27 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/dd.h"
#include "drivers/bus.h"
#include "drivers/driver.h"

#include "dev/device.h"

#include "kerrno.h"

#define MODULE "dd"

typedef struct device_attach_data
{
    device_t *dev;
} device_attach_data_t;

status_t call_driver_probe(device_t *dev, driver_t *drv)
{
    int ret = 0;

    if (dev->bus->probe)
    {
        ret = dev->bus->probe(dev);
    }
    else if (drv->probe)
    {
        ret = drv->probe(dev);
    }

    switch (ret)
    {
        case KERRNO_SUCCESSES :
            break;
        case -KERRNO_PROBE_DEFER :
            /* Driver requested deferred probing */
            // dev_dbg(dev, "Driver %s requests probe deferral\n", drv->name);
            trace_err(MODULE, "Driver %s requests probe deferral", drv->name);
            break;
        case -ENODEV :
        case -ENXIO :
            trace_err(MODULE, "probe with driver %s rejects match %d", drv->name, ret);
            // dev_dbg(dev, "probe with driver %s rejects match %d\n", drv->name, ret);
            break;
        default :
            /* driver matched but the probe failed */
            trace_err(MODULE, "probe with driver %s failed with error %d", drv->name, ret);
            // dev_err(dev, "probe with driver %s failed with error %d\n", drv->name, ret);
            break;
    }

    return ret;
}

status_t driver_probe_device(driver_t *drv, device_t *dev)
{

    if (!dev_ready_to_probe(dev))
    {
        KERRNO_RETURN(KERRNO_PROBE_DEFER, "Device not ready to probe");
    }

    dev->driver = drv;

    return call_driver_probe(dev, drv);
}

status_t device_try_attach_driver(driver_t *drv, void *_data)
{
    ENTER_FUNC("%p, %p", drv, _data);
    device_attach_data_t *data = _data;
    device_t *dev = data->dev;

    status_t ret = drv->bus->match ? drv->bus->match(dev, drv) : KERRNO_UNSUCCESS;
    if (ret == KERRNO_UNSUCCESS)
    {
        // no matches
        return KERRNO_SUCCESSES;
    }

    ret = driver_probe_device(drv, dev);
    return ret;
}

status_t device_attach(device_t *dev)
{
    ENTER_FUNC("%p", dev);
    device_attach_data_t data = {
        .dev = dev,
    };

    driver_t *drv = NULL;
    bus_for_each_driver(dev->bus, drv, &data, device_try_attach_driver);

    return KERRNO_SUCCESSES;
}

status_t device_initial_probe(device_t *dev)
{
    return device_attach(dev);
}
