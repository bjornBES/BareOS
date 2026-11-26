/*
 * File: serio.c
 * File Created: 26 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/input/serio/serio.h"

#include "drivers/base/class.h"

#include "dev/device.h"

#include "kerrno.h"

status_t serio_device_added(device_t *dev) { return KERRNO_SUCCESS; }
status_t serio_device_removed(device_t *dev) { return KERRNO_SUCCESS; }

static const dev_class_t i8042_class = {
    .name = "serio",
    .type = DEVICE_SERIAL_IO,
    .device_added = serio_device_added,
    .device_removed = serio_device_removed,
};
REGISTER_CLASS(i8042_class);

status_t serio_register_port(serio_dev_t *serio)
{
    
}
