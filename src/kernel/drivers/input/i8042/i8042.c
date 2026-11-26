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
#include "drivers/input/serio/serio.h"

#include "drivers/base/class.h"

#include "irq/irq.h"

#include "debug/debug.h"

#include "dev/device.h"

#include "string.h"
#include "memory.h"

#include "kerrno.h"

#define MODULE            "i8042-driver"

#define DRV_NAME          "i8042"

#define I8042_KBD_PORT_NO 0
#define I8042_AUX_PORT_NO 1
#define I8042_MUX_PORT_NO 2
#define I8042_NUM_PORTS   (4 + 2)

static mutex_t i8042_mutex;

static i8042_port_t i8042_ports[I8042_NUM_PORTS];

status_t i8042_interrupt(int irq, void *dev_id)
{
	if (unlikely(!i8042_handle_data(irq))) {
		dbg("Interrupt %d, without any data\n", irq);
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

INTERNAL status_t i8042_create_kbd_port(platform_device_t *platform_dev)
{
    i8042_port_t *port = &i8042_ports[I8042_KBD_PORT_NO];
    serio_dev_t *serio_dev = kzmalloc(sizeof(serio_dev_t));
    if (!serio_dev)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED);
    }

    serio_dev->id.type = SERIO_8042;
    // serio_dev->start = i8042_start;
    // serio_dev->stop = i8042_stop;
    // serio_dev->close = i8042_port_close;
    // serio_dev->ps2_cmd_mutex = &i8042_mutex;
    serio_dev->port_data = port;
    serio_dev->dev.controller = &platform_dev->dev;
    strscpy(serio_dev->name, "i8042 KBD port", sizeof(serio_dev->name));

    port->serio = serio_dev;
    port->irq = I8042_KBD_IRQ;
    return KERRNO_SUCCESS;
}

INTERNAL status_t i8042_setup_kbd(platform_device_t *platform_dev)
{
    status_t ret = i8042_create_kbd_port(platform_dev);
    KERRNO_CHECK_AND_RETURN(ret);
    
    ret = irq_pick_one(I8042_KBD_IRQ, IRQ_SOURCE_IRQ, ANY_CPU, &platform_dev->kernel_irq);
    KERRNO_CHECK_AND_RETURN(ret);
}

status_t i8042_probe(platform_device_t *platform_dev)
{
    ENTER_FUNC("%p", platform_dev);

    platform_dev->dev.init_name = "i8042";
    dev_set_same_name(&platform_dev->dev);

    i8042_port_t *port = &i8042_ports[I8042_KBD_PORT_NO];
    status_t ret = i8042_kbd_probe(port);
    KERRNO_CHECK_RETURN(ret);

    // serio_register_port();

    device_add(&platform_dev->dev);

    // FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESS;
}

platform_driver_t i8042_driver = {
    .driver = {
        .name = DRV_NAME,
    },
    .probe = i8042_probe,
};
MODULE_PLATFORM_DRIVER(i8042_driver);
