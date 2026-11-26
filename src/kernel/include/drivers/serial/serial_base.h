/*
 * File: serial_base.h
 * File Created: 23 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "serial_core_types.h"
#include "serial_port.h"
#include "dev/devicet.h"

#include <types.h>

status_t serial_base_driver_register(driver_t *driver);
status_t serial_base_driver_unregister(driver_t *driver);

status_t serial_base_ctrl_add(uart_port_t *port, device_t *parent, serial_ctrl_device_t **out);
status_t serial_base_port_add(uart_port_t *port, serial_ctrl_device_t *ctrl_dev, serial_port_device_t **out);
