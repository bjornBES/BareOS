/*
 * File: i8042.c
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "i8042.h"

#include "drivers/platform/platform.h"

#include "drivers/base/class.h"

#include "debug/debug.h"

#include "dev/device.h"

#include "kerrno.h"

#define MODULE                 "i8042-driver"

#define DRV_NAME               "i8042"

#define I8042_KBD_PORT_NO	0
#define I8042_AUX_PORT_NO	1
#define I8042_MUX_PORT_NO	2
#define I8042_NUM_PORTS		(4 + 2)

static i8042_port_t i8042_ports[I8042_NUM_PORTS];

status_t serio_device_added(device_t *dev) { return KERRNO_SUCCESSES; }

static dev_class_t i8042_class = {
    .name = "serio",
    .type = DEVICE_SERIAL,
    .device_added = serio_device_added,
};

status_t i8042_class_register()
{
    return class_register(&i8042_class);
}

ARCHDONE_INITCALL(i8042_class_register);

status_t i8042_probe(platform_device_t *dev)
{
    ENTER_FUNC("%p", dev);

    dev->dev.class = &i8042_class;
    dev->dev.init_name = "i8042";
    dev_set_same_name(&dev->dev);

    i8042_port_t *port = &i8042_ports[I8042_KBD_PORT_NO];
    status_t ret = i8042_kbd_probe(port);
    KERRNO_CHECK_RETURN(ret);

    device_add(&dev->dev);

    // FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

platform_driver_t i8042_driver = {
    .driver = {
        .name = DRV_NAME,
    },
    .probe = i8042_probe,
};
MODULE_PLATFORM_DRIVER(i8042_driver);
