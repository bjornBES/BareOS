/*
 * File: serial_base_bus.c
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial_base.h"
#include "drivers/serial/serial_port.h"

#include "drivers/base/driver.h"
#include "drivers/base/class.h"
#include "drivers/base/bus.h"

#include "dev/devicet.h"
#include "dev/device.h"

#include "idalloc/ida/ida.h"

#include "string.h"
#include "memory.h"

#include "kerrno.h"

#define MODULE "serial-base-bus"

device_type_t serial_ctrl_type = {
    .name = "ctrl",
};

device_type_t serial_port_type = {
    .name = "port",
};

status_t serial_base_match(device_t *dev, driver_t *drv)
{
    if (dev->type == &serial_ctrl_type && str_has_prefix(drv->name, serial_ctrl_type.name))
    {
        return KERRNO_SUCCESS;
    }

    if (dev->type == &serial_port_type && str_has_prefix(drv->name, serial_port_type.name))
    {
        return KERRNO_SUCCESS;
    }

    KERRNO_RETURN(KERRNO_UNSUCCESS, "");
}

bus_type_t serial_base_bus_type = {
    .name = "serial-base",
    .match = serial_base_match,
};

status_t serial_base_driver_register(driver_t *driver)
{
	driver->bus = &serial_base_bus_type;

	return driver_register(driver);
}

status_t serial_base_driver_unregister(driver_t *driver)
{
	return driver_unregister(driver);
}

INTERNAL status_t serial_base_device_init(uart_port_t *port, device_t *dev, device_t *parent_dev, device_type_t *type, status_t (*release)(device_t *dev), uint32_t ctrl_id, uint32_t port_id)
{
    device_initialize(dev);
    dev->type = type;
    dev->parent = parent_dev;
    dev->bus = &serial_base_bus_type;
    dev->ops.release = release;
    dev->class = &serial_class;
    dev_set_same_name(dev);
    // dev_set_of_node_reused(dev);

    if (type == &serial_ctrl_type)
    {
        return device_set_name(dev, "%s:%d", dev_name(port->dev), ctrl_id);
    }

    if (type == &serial_port_type)
    {
        return device_set_name(dev, "%s:%d.%d", dev_name(port->dev), ctrl_id, port_id);
    }

    KERRNO_RETURN(EINVAL, "");
}

status_t serial_base_ctrl_release(device_t *dev)
{
    serial_ctrl_device_t *ctrl_dev = to_serial_base_ctrl_device(dev);

    // fwnode_handle_put(dev_fwnode(dev));
    return kfree(ctrl_dev);
}

status_t serial_base_ctrl_add(uart_port_t *port, device_t *parent, serial_ctrl_device_t **out)
{
    serial_ctrl_device_t *ctrl_dev;
    status_t ret;

    ctrl_dev = kzmalloc(sizeof(serial_ctrl_device_t));
    if (!ctrl_dev)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED, "");
    }

    ret = ida_init(&ctrl_dev->port_ida);
    if (ret)
    {
        goto err_release_ida;
    }

    ret = serial_base_device_init(port, &ctrl_dev->dev, parent, &serial_ctrl_type, serial_base_ctrl_release, port->ctrl_id, 0);
    if (ret)
    {
        goto err_put_device;
    }

    ret = device_add(&ctrl_dev->dev);
    if (ret)
    {
        goto err_put_device;
    }

    *out = ctrl_dev;
    return KERRNO_SUCCESS;

err_put_device:
    ret = device_put(&ctrl_dev->dev);

err_release_ida:
    ret = ida_release(&ctrl_dev->port_ida);

    return ret;
}

status_t serial_base_port_release(device_t *dev)
{
    serial_port_device_t *port_dev = to_serial_base_port_device(dev);

    // fwnode_handle_put(dev_fwnode(dev));
    return kfree(port_dev);
}

status_t serial_base_port_add(uart_port_t *port, serial_ctrl_device_t *ctrl_dev, serial_port_device_t **out)
{
    serial_port_device_t *port_dev;
    int min = 0;
    int max = -1; /* Use -1 for max to apply IDA defaults */
    status_t ret;

    port_dev = kzmalloc(sizeof(serial_port_device_t));
    if (!port_dev)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED, "");
    }

    /* Device driver specified port_id vs automatic assignment? */
    if (port->port_id)
    {
        min = port->port_id;
        max = port->port_id;
    }

    ret = ida_alloc_range(&ctrl_dev->port_ida, min, max, &port->port_id);
    if (ret)
    {
        device_put(&port_dev->dev);
        LOG_RETURN(ret)
    }

    ret = serial_base_device_init(port, &port_dev->dev, &ctrl_dev->dev, &serial_port_type, serial_base_port_release, port->ctrl_id, port->port_id);
    if (ret)
    {
        goto err_put_device;
    }

    port_dev->port = port;

    ret = device_add(&port_dev->dev);
    if (ret)
    {
        goto err_put_device;
    }

    *out = port_dev;
    return KERRNO_SUCCESS;

err_put_device:
    device_put(&port_dev->dev);
    ida_release_index(&ctrl_dev->port_ida, port->port_id);

    LOG_RETURN(ret);
}

status_t serial_base_init()
{
    status_t ret = bus_register(&serial_base_bus_type);
    KERRNO_CHECK_AND_RETURN(ret);
    
    ret = serial_base_ctrl_init();
    KERRNO_CHECK_AND_RETURN(ret);
    
    ret = serial_base_port_init();
    if (ret)
    {
        LOG_RETURN(ret);
    }
    return KERRNO_SUCCESS;
}

ARCH_INITCALL(serial_base_init);
