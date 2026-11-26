/*
 * File: serial_port.c
 * File Created: 23 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial_port.h"
#include "drivers/serial/serial_uart_drv.h"

#include "drivers/base/drivert.h"

#include "dev/devicet.h"

#include "kerrno.h"

#define MODULE "serial-port"

status_t serial_port_probe(device_t *dev)
{
	// pm_runtime_enable(dev);
	// pm_runtime_set_autosuspend_delay(dev, SERIAL_PORT_AUTOSUSPEND_DELAY_MS);
	// pm_runtime_use_autosuspend(dev);

	return KERRNO_SUCCESS;
}

status_t serial_port_remove(device_t *dev)
{
	// pm_runtime_dont_use_autosuspend(dev);
	// pm_runtime_disable(dev);

	return KERRNO_SUCCESS;
}

status_t uart_add_one_port(uart_driver_t *drv, uart_port_t *port)
{
	ENTER_FUNC("%p, %p", drv, port);
	return serial_core_register_port(drv, port);
}

void uart_remove_one_port(uart_driver_t *drv, uart_port_t *port)
{
	// serial_ctrl_unregister_port(drv, port);
}

driver_t serial_port_driver = {
	.name = "port",
	// .suppress_bind_attrs = true,
	.probe = serial_port_probe,
	.remove = serial_port_remove,
	// .pm = pm_ptr(&serial_port_pm),
};

status_t serial_base_port_init()
{
	return serial_base_driver_register(&serial_port_driver);
}