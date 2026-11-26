/*
 * File: 8250_platform.c
 * File Created: 18 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/8250/8250.h"

#include "drivers/serial/serial_core.h"

#include "drivers/platform/platform_driver.h"
#include "drivers/platform/platform.h"

#include "drivers/base/classt.h"

#include "asm/serial_port_arch.h"

#include "irq/irq.h"
#include "ivt/ivt.h"

#include "debug/debug.h"

#include "memory.h"

#include <array.h>

#define MODULE "8250-platform"

static bool share_irqs = IS_ENABLED(CONFIG_SERIAL_8250_SHARE_IRQ);

// TODO: CONFIG_SERIAL_8250_RUNTIME_UARTS

unsigned int nr_uarts = 1;

const old_serial_port_t old_serial_ports[] = {
    SERIAL_PORT_DFNS,
};

static void __serial8250_isa_init_ports()
{
    ENTER_FUNC("", 0);
    int i;

    if (nr_uarts > UART_NR)
    {
        nr_uarts = UART_NR;
    }

    // univ8250_port_ops = *univ8250_port_base_ops;
    // univ8250_rsa_support(&univ8250_port_ops, univ8250_port_base_ops);

    for (i = 0; i < ARRAY_SIZE(old_serial_ports) && i < nr_uarts; i++)
    {
        uart_8250_port_t *up = serial8250_core_get_port(i);
        uart_port_t *port = &up->port;

        old_serial_port_t *old_port = (old_serial_port_t *)&old_serial_ports[i];

        port->iobase = old_port->port;
        irq_mask_t mask;
        SET_BIT(old_port->irq, mask.bits);
        irq_pick_free_entry(&mask, IRQ_SOURCE_IRQ, ANY_CPU, &port->irq);

        port->irqflags = 0;
        port->uartclk = old_port->baud_base * 16;
        port->flags = old_port->flags;
        // port->hub6 = 0;
        port->membase = old_port->iomem_base;
        port->iotype = old_port->io_type;
        port->regshift = old_port->iomem_reg_shift;

        if (share_irqs)
        {
            port->irqflags |= HANDLER_SHAREABLE;
        }

        // if (serial8250_isa_config != NULL)
        // {
        //     serial8250_isa_config(i, &up->port, &up->capabilities);
        // }
    }

    for (i = 0; i < nr_uarts; i++)
    {
        serial8250_core_setup_port(i, NULL);
    }
}

void serial8250_platform_isa_init_ports()
{
    __serial8250_isa_init_ports();
}

static int serial8250_probe_platform(platform_device_t *dev, plat_serial8250_port_t *p)
{
    status_t ret;
    int i;

    uart_8250_port_t *uart = kzmalloc(sizeof(uart_8250_port_t));
    if (!uart)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED, "");
    }

    for (i = 0; p && p->flags != 0; p++, i++)
    {
        uart->port.iobase = p->iobase;
        uart->port.membase = p->membase;
        uart->port.irq = p->irq;
        uart->port.irqflags = p->irqflags;
        uart->port.uartclk = p->uartclk;
        uart->port.regshift = p->regshift;
        uart->port.iotype = p->iotype;
        uart->port.flags = p->flags;
        uart->port.mapbase = p->mapbase;
        uart->port.mapsize = p->mapsize;
        // uart->port.hub6 = p->hub6;
        uart->port.has_sysrq = p->has_sysrq;
        uart->port.private_data = p->private_data;
        uart->port.type = p->type;
        uart->bugs = p->bugs;
        uart->port.serial_in = p->serial_in;
        uart->port.serial_out = p->serial_out;
        uart->dl_read = p->dl_read;
        uart->dl_write = p->dl_write;
        uart->port.handle_irq = p->handle_irq;
        uart->port.handle_break = p->handle_break;
        uart->port.set_termios = p->set_termios;
        uart->port.set_ldisc = p->set_ldisc;
        uart->port.get_mctrl = p->get_mctrl;
        // uart->port.pm = p->pm;
        uart->port.dev = &dev->dev;

        if (share_irqs)
        {
            uart->port.irqflags |= HANDLER_SHAREABLE;
        }

        ret = serial8250_core_register_8250_port(uart);
        if (ret < 0)
        {
            pr_err("unable to register port at index %d "
                   "(IO%lx MEM%llx IRQ%d): %d",
                   i,
                   p->iobase, (unsigned long long)p->mapbase,
                   p->irq, ret);
        }
    }
    return 0;
}

status_t serial8250_probe(platform_device_t *pdev)
{
    ENTER_FUNC("%p", pdev);
    plat_serial8250_port_t *p;

    p = dev_get_platdata(pdev);
    if (p)
    {
        return serial8250_probe_platform(pdev, p);
    }

    return KERRNO_SUCCESS;
}

platform_driver_t serial8250_isa_driver = {
    .probe = serial8250_probe,
    // .remove = serial8250_remove,
    // .suspend = serial8250_suspend,
    // .resume = serial8250_resume,
    .driver = {
        .name = "serial8250",
        // .acpi_match_table = acpi_platform_serial_table,
        // .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

platform_device_t *serial8250_isa_devs;

__init status_t serial8250_platform_init()
{
    status_t ret;

    if (nr_uarts == 0)
    {
        KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "");
    }

    serial8250_platform_isa_init_ports();

    pr_info("serial: 8250/16550 driver, %d ports, IRQ sharing %s",
            nr_uarts, share_irqs ? "enabled" : "disabled");

    serial8250_reg.nr = UART_NR;
    ret = uart_core_register_driver(&serial8250_reg);

    if (ret)
    {
        LOG_RETURN(ret);
    }

    ret = platform_device_alloc("serial8250", PLAT8250_DEV_LEGACY, &serial8250_isa_devs);
    if (ret)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED, "");
    }

    serial8250_isa_devs->dev.class = &serial_class;

    ret = platform_device_add(serial8250_isa_devs);
    KERRNO_CHECK_AND_RETURN(ret);

    serial8250_core_register_ports(&serial8250_reg, &serial8250_isa_devs->dev);

    ret = platform_register_driver(&serial8250_isa_driver);
    if (ret == 0)
    {
        return ret;
    }
    FUNC_NOT_IMPLEMENTED();
    return ret;
}

MODULE_INIT(serial8250_platform_init);
