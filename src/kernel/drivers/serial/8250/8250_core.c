/*
 * File: 8250_core.c
 * File Created: 16 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/8250/8250.h"
#include "drivers/serial/8250/8250_port.h"

#include "drivers/serial/serial.h"
#include "drivers/serial/serial_uart_drv.h"
#include "drivers/serial/serial_core.h"

#include "drivers/tty/tty.h"

#include "console/console.h"

#include "drivers/base/classt.h"

#include "sync/mutex.h"

#include "string.h"
#include "math.h"
#include "memory.h"
#include "module.h"
#include "kerrno.h"

#include <config.h>

#define MODULE "8250-core"

static uart_8250_port_t serial8250_ports[UART_NR];

uart_driver_t serial8250_reg;

void serial8250_out_MCR(uart_8250_port_t *up, uint32_t value)
{
    serial_out(up, UART_MCR, value);

    // if (up->gpios)
    // {
    //     mctrl_gpio_set(up->gpios, serial8250_MCR_to_TIOCM(value));
    // }
}

uart_8250_port_t *serial8250_core_get_port(int line)
{
    return &serial8250_ports[line];
}

status_t serial8250_core_setup_port(int index, uart_8250_port_t **out)
{
    ENTER_FUNC("%u, %p", index, out);
    uart_8250_port_t *up;

    if (index >= UART_NR)
    {
        if (out)
        {
            *out = NULL;
        }
        KERRNO_RETURN(KERRNO_UNSUCCESS, "");
    }

    up = &serial8250_ports[index];
    up->port.line = index;
    up->port.port_id = index;

    serial8250_port_init_port(up);
    // if (!univ8250_port_base_ops)
    // {
    //     univ8250_port_base_ops = up->port.ops;
    // }
    // up->port.ops = &univ8250_port_ops;

    // timer_setup(&up->timer, serial8250_timeout, 0);

    // up->ops = &univ8250_driver_ops;

    serial8250_port_set_defaults(up);

    if (out)
    {
        *out = up;
    }
    return KERRNO_SUCCESS;
}

__init void serial8250_core_register_ports(uart_driver_t *drv, device_t *dev)
{
    ENTER_FUNC("%p, %p", drv, dev);
    int i;

    for (i = 0; i < nr_uarts; i++)
    {
        uart_8250_port_t *up = &serial8250_ports[i];

        if (up->port.type == PORT_8250_CIR)
        {
            continue;
        }

        if (up->port.dev)
        {
            continue;
        }

        up->port.dev = dev;

        // if (uart_console_registered(&up->port))
        // {
        //     pm_runtime_get_sync(up->port.dev);
        // }

        status_t ret = uart_add_one_port(drv, &up->port);
    }
}

#ifdef CONFIG_SERIAL_8250_CONSOLE

status_t univ8250_console_setup(console_t *co, char *options)
{
    ENTER_FUNC("%p, %p", co, options);
    status_t ret = KERRNO_SUCCESS;

    if (co->index < 0 || co->index >= UART_NR)
    {
        co->index = 0;
    }

    uart_8250_port_t *uart_8250_port;
    for (int i = nr_uarts; i <= co->index; i++)
    {
        ret = serial8250_core_setup_port(i, &uart_8250_port);
        if (!uart_8250_port || ret)
        {
            KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "");
        }
        nr_uarts++;
    }

    uart_port_t *uart_port = &serial8250_ports[co->index].port;
    uart_port_set_cons(uart_port, co);

    ret = serial8250_port_console_setup(uart_port, options, false);
    if (ret != 0)
    {
        uart_port_set_cons(uart_port, NULL);
    }
    return ret;
}

console_t univ8250_console = {
    .name = "ttyS0",
    // .write_atomic	= univ8250_console_write_atomic,
    // .write_thread	= univ8250_console_write_thread,
    // .device_lock	= univ8250_console_device_lock,
    // .device_unlock	= univ8250_console_device_unlock,
    .device = uart_core_console_device,
    .setup = univ8250_console_setup,
    // .exit		= univ8250_console_exit,
    // .match		= univ8250_console_match,
    .flags = CON_PRINTBUFFER | CON_ANYTIME | CON_NBCON,
    .index = -1,
    .data = &serial8250_reg,
};

status_t univ8250_console_init()
{
    if (nr_uarts == 0)
    {
        KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "");
    }

    // serial8250_platform_isa_init_ports();
    status_t ret = console_register(&univ8250_console);
    return ret;
}

CONSOLE_INITCALL(univ8250_console_init);

#define SERIAL8250_CONSOLE (&univ8250_console)
#else
#define SERIAL8250_CONSOLE NULL
#endif

uart_driver_t serial8250_reg = {
    .driver_name = "serial_8250",
    .dev_name = "ttyS",
    .minor = 64,
    .cons = SERIAL8250_CONSOLE,
};

status_t serial8250_init()
{
    serial8250_reg.major = tty_class.type;
    serial8250_reg.nr = UART_NR;
    return KERRNO_SUCCESS;
}

SUBSYS_INITCALL(serial8250_init);

bool uart_core_match_port(const uart_port_t *port1, const uart_port_t *port2)
{
    if (port1->iotype != port2->iotype)
    {
        return false;
    }
    else if (port1->iotype == SERIAL_IO_TYPE_IO_PORT)
    {
        return port1->iobase == port2->iobase;
    }
    else if (port1->iotype == SERIAL_IO_TYPE_HUB6)
    {
        // return hub6_match_port(port1, port2);
        FUNC_NOT_IMPLEMENTED();
    }
    else if (uart_iotype_mmio(port1->iotype))
    {
        return port1->mapbase == port2->mapbase;
    }
    else if (port1->iotype == SERIAL_IO_TYPE_BUS)
    {
        return true;
    }
    return false;
}

mutex_t serial_mutex;

static uart_8250_port_t *serial8250_find_match_or_unused(const uart_port_t *port)
{
    int i;

    for (i = 0; i < nr_uarts; i++)
    {
        if (uart_core_match_port(&serial8250_ports[i].port, port))
        {
            return &serial8250_ports[i];
        }
    }

    i = port->line;
    if (i < nr_uarts && serial8250_ports[i].port.type == PORT_UNKNOWN && serial8250_ports[i].port.iobase == 0)
    {
        return &serial8250_ports[i];
    }
    for (i = 0; i < nr_uarts; i++)
    {
        if (serial8250_ports[i].port.type == PORT_UNKNOWN && serial8250_ports[i].port.iobase == 0)
        {
            return &serial8250_ports[i];
        }
    }

    for (i = 0; i < nr_uarts; i++)
    {
        if (serial8250_ports[i].port.type == PORT_UNKNOWN)
        {
            return &serial8250_ports[i];
        }
    }

    return NULL;
}

status_t serial8250_core_register_8250_port(const uart_8250_port_t *up)
{
    ENTER_FUNC("%p", up);
    uart_8250_port_t *uart;
    bool cons_flow;

    if (up->port.uartclk == 0)
    {
        return -EINVAL;
    }

    mutex_lock(&serial_mutex);

    status_t ret = 0;

    uart = serial8250_find_match_or_unused(&up->port);
    if (!uart)
    {
        /*
         * If the port is past the initial isa ports, initialize a new
         * port and increment nr_uarts accordingly.
         */
        ret = serial8250_core_setup_port(nr_uarts, &uart);
        if (ret)
        {
            LOG_RETURN(ret);
        }
        if (!uart)
        {
            KERRNO_RETURN(KERRNO_DEVICE_FULL, "");
        }
        nr_uarts++;
    }

    /* Check if it is CIR already. We check this below again, see there why. */
    if (uart->port.type == PORT_8250_CIR)
    {
        KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "");
    }

    /* Preserve specified console flow control. */
    cons_flow = uart_port_cons_flow_enabled(&uart->port);

    if (uart->port.dev)
    {
        uart_remove_one_port(&serial8250_reg, &uart->port);
    }

    uart->port.ctrl_id = up->port.ctrl_id;
    uart->port.port_id = up->port.port_id;
    uart->port.iobase = up->port.iobase;
    uart->port.membase = up->port.membase;
    uart->port.irq = up->port.irq;
    uart->port.irqflags = up->port.irqflags;
    uart->port.uartclk = up->port.uartclk;
    uart->port.fifosize = up->port.fifosize;
    uart->port.regshift = up->port.regshift;
    uart->port.iotype = up->port.iotype;
    uart->port.flags = up->port.flags | UPF_BOOT_AUTOCONF;
    uart->bugs = up->bugs;
    uart->port.mapbase = up->port.mapbase;
    uart->port.mapsize = up->port.mapsize;
    uart->port.private_data = up->port.private_data;
    uart->tx_loadsz = up->tx_loadsz;
    uart->capabilities = up->capabilities;
    uart->port.throttle = up->port.throttle;
    uart->port.unthrottle = up->port.unthrottle;
    // uart->port.rs485_config = up->port.rs485_config;
    // uart->port.rs485_supported = up->port.rs485_supported;
    // uart->port.rs485 = up->port.rs485;
    // uart->rs485_start_tx = up->rs485_start_tx;
    // uart->rs485_stop_tx = up->rs485_stop_tx;
    uart->lsr_save_mask = up->lsr_save_mask;
    // uart->dma = up->dma;

    uart_port_set_cons_flow_enabled(&uart->port, uart_port_cons_flow_enabled(&up->port) | cons_flow);

    /* Take tx_loadsz from fifosize if it wasn't set separately */
    if (uart->port.fifosize && !uart->tx_loadsz)
    {
        uart->tx_loadsz = uart->port.fifosize;
    }

    if (up->port.dev)
    {
        uart->port.dev = up->port.dev;
        // ret = uart_get_rs485_mode(&uart->port);
        // if (ret)
        // {
        //     goto err;
        // }
    }

    if (up->port.flags & UPF_FIXED_TYPE)
    {
        uart->port.type = up->port.type;
    }

    // if (uart->port.dev && !has_acpi_companion(uart->port.dev))
    // {
    //     struct mctrl_gpios *gpios = mctrl_gpio_init(&uart->port, 0);
    //     if (IS_ERR(gpios))
    //     {
    //         ret = PTR_ERR(gpios);
    //         goto err;
    //     }
    //     else
    //     {
    //         uart->gpios = gpios;
    //     }
    // }

    serial8250_port_set_defaults(uart);

    /* Possibly override default I/O functions.  */
    if (up->port.serial_in)
    {
        uart->port.serial_in = up->port.serial_in;
    }
    if (up->port.serial_out)
    {
        uart->port.serial_out = up->port.serial_out;
    }
    if (up->port.handle_irq)
    {
        uart->port.handle_irq = up->port.handle_irq;
    }

    if (up->port.set_termios)
    {
        uart->port.set_termios = up->port.set_termios;
    }
    if (up->port.set_ldisc)
    {
        uart->port.set_ldisc = up->port.set_ldisc;
    }
    if (up->port.get_mctrl)
    {
        uart->port.get_mctrl = up->port.get_mctrl;
    }
    if (up->port.set_mctrl)
    {
        uart->port.set_mctrl = up->port.set_mctrl;
    }
    if (up->port.get_divisor)
    {
        uart->port.get_divisor = up->port.get_divisor;
    }
    if (up->port.set_divisor)
    {
        uart->port.set_divisor = up->port.set_divisor;
    }
    if (up->port.get_rxtrig)
    {
        uart->port.get_rxtrig = up->port.get_rxtrig;
    }
    if (up->port.set_rxtrig)
    {
        uart->port.set_rxtrig = up->port.set_rxtrig;
    }
    if (up->port.startup)
    {
        uart->port.startup = up->port.startup;
    }
    if (up->port.shutdown)
    {
        uart->port.shutdown = up->port.shutdown;
    }
    if (up->port.break_ctl)
    {
        uart->port.break_ctl = up->port.break_ctl;
    }
    // if (up->port.pm)
    // {
    //     uart->port.pm = up->port.pm;
    // }
    if (up->port.handle_break)
    {
        uart->port.handle_break = up->port.handle_break;
    }
    if (up->dl_read)
    {
        uart->dl_read = up->dl_read;
    }
    if (up->dl_write)
    {
        uart->dl_write = up->dl_write;
    }

    /* Check the type (again)! It might have changed by the port.type assignment above. */
    if (uart->port.type != PORT_8250_CIR)
    {
        // if (uart_console_registered(&uart->port))
        // {
        //     pm_runtime_get_sync(uart->port.dev);
        // }

        // if (serial8250_isa_config != NULL)
        // {
        //     serial8250_isa_config(0, &uart->port,
        //                           &uart->capabilities);
        // }

        // serial8250_apply_quirks(uart);
        ret = uart_add_one_port(&serial8250_reg, &uart->port);
        if (ret)
        {
            goto err;
        }

        ret = uart->port.line;
    }
    else
    {
        pr_info("skipping CIR port at 0x%lx / 0x%llx, IRQ %d",
                uart->port.iobase,
                (unsigned long long)uart->port.mapbase,
                uart->port.irq);

        ret = 0;
    }

    if (!uart->lsr_save_mask)
    {
        uart->lsr_save_mask = LSR_SAVE_FLAGS; /* Use default LSR mask */
    }

    // if (up->overrun_backoff_time_ms > 0)
    // {
    //     uart->overrun_backoff_time_ms =
    //         up->overrun_backoff_time_ms;
    //     INIT_DELAYED_WORK(&uart->overrun_backoff,
    //                       serial_8250_overrun_backoff_work);
    // }
    // else
    // {
    //     uart->overrun_backoff_time_ms = 0;
    // }

    return ret;

    err:
    uart->port.dev = NULL;
    return ret;
}

