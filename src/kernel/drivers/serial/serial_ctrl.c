/*
 * File: serial_ctrl.c
 * File Created: 25 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial_port.h"
#include "drivers/serial/serial_uart_drv.h"

#include "drivers/base/drivert.h"

#include "dev/devicet.h"

#include "kerrno.h"

status_t serial_ctrl_probe(device_t *dev)
{
	// pm_runtime_enable(dev);

	return KERRNO_SUCCESS;
}

status_t serial_ctrl_remove(device_t *dev)
{
	// pm_runtime_disable(dev);

	return KERRNO_SUCCESS;
}

driver_t serial_ctrl_driver = {
	.name = "ctrl",
	.probe = serial_ctrl_probe,
	.remove = serial_ctrl_remove,
};

status_t serial_base_ctrl_init()
{
	return serial_base_driver_register(&serial_ctrl_driver);
}

status_t serial_base_ctrl_exit()
{
	return serial_base_driver_unregister(&serial_ctrl_driver);
}
