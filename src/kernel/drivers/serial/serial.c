/*
 * File: serial.c
 * File Created: 14 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial.h"
#include "drivers/base/class.h"

#include "init.h"

#include "kerrno.h"

status_t serial_added(device_t *dev)
{
    return KERRNO_SUCCESS;
}

status_t serial_removed(device_t *dev)
{
    return KERRNO_SUCCESS;
}

const dev_class_t serial_class = {
    .name = "serial",
    .type = DEVICE_SERIAL,
    .device_added = serial_added,
    .device_removed = serial_removed,
};
REGISTER_CLASS(serial_class);

status_t serial_init()
{
    return KERRNO_SUCCESS;
}

POSTCORE_INITCALL(serial_init);

