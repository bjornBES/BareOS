/*
 * File: 8250_early.c
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial.h"
#include "console/earlycon.h"
#include "drivers/serial/8250/8250.h"

#include "asm/io_arch.h"

#include "math.h"

#include "kerrno.h"

static unsigned int serial8250_early_in(uart_port_t *port, int offset)
{
    offset <<= port->regshift;

    switch (port->iotype)
    {
        case SERIAL_IO_TYPE_MEM :
            // return readb(port->membase + offset);
        case SERIAL_IO_TYPE_MEM16 :
            // return readw(port->membase + offset);
        case SERIAL_IO_TYPE_MEM32 :
            // return readl(port->membase + offset);
        case SERIAL_IO_TYPE_MEM32BE :
            // return ioread32be(port->membase + offset);
#ifdef CONFIG_HAS_IOPORT
        case SERIAL_IO_TYPE_IO_PORT :
            return inb(port->iobase + offset);
#endif
        default :
            return 0;
    }
}

static void serial8250_early_out(uart_port_t *port, int offset, int value)
{
    offset <<= port->regshift;

    switch (port->iotype)
    {
        case SERIAL_IO_TYPE_MEM :
            // writeb(value, port->membase + offset);
            break;
        case SERIAL_IO_TYPE_MEM16 :
            // writew(value, port->membase + offset);
            break;
        case SERIAL_IO_TYPE_MEM32 :
            // writel(value, port->membase + offset);
            break;
        case SERIAL_IO_TYPE_MEM32BE :
            // iowrite32be(value, port->membase + offset);
            break;
#ifdef CONFIG_HAS_IOPORT
        case SERIAL_IO_TYPE_IO_PORT :
            outb(value, port->iobase + offset);
            break;
#endif
        default :
            break;
    }
}

static status_t serial_putc(uart_port_t *port, unsigned char c)
{
    unsigned int status;

    serial8250_early_out(port, UART_TX, c);

    for (;;)
    {
        status = serial8250_early_in(port, UART_LSR);
        if (uart_lsr_tx_empty(status))
        {
            break;
        }
    }
    return KERRNO_SUCCESS;
}

static status_t early_serial8250_write(console_t *console, console_message_t *console_msg, const char *s, size_t count)
{
    earlycon_device_t *device = console->data;
    uart_port_t *port = &device->port;
    
    uart_core_console_write(port, s, count, serial_putc);
    return KERRNO_SUCCESS;
}

static void __init init_port(earlycon_device_t *device)
{
    uart_port_t *port = &device->port;
    uint32_t divisor;
    unsigned char c;
    uint32_t ier;

    serial8250_early_out(port, UART_LCR, UART_LCR_WLEN8);     /* 8n1 */
    ier = serial8250_early_in(port, UART_IER);
    serial8250_early_out(port, UART_IER, ier & UART_IER_UUE); /* no interrupt */
    serial8250_early_out(port, UART_FCR, 0);                  /* no fifo */
    serial8250_early_out(port, UART_MCR, UART_MCR_DTR | UART_MCR_RTS);

    if (port->uartclk)
    {
        divisor = DIV_ROUND_CLOSEST(port->uartclk, 16 * device->baud);
        c = serial8250_early_in(port, UART_LCR);
        serial8250_early_out(port, UART_LCR, c | UART_LCR_DLAB);
        serial8250_early_out(port, UART_DLL, divisor & 0xFF);
        serial8250_early_out(port, UART_DLM, (divisor >> 8) & 0xFF);
        serial8250_early_out(port, UART_LCR, c & ~UART_LCR_DLAB);
    }
}

int __init early_serial8250_setup(earlycon_device_t *device, const char *options)
{
    if (!(device->port.membase || device->port.iobase))
    {
        return KERRNO_DEV_NOT_FOUND;
    }

    if (!device->baud)
    {
        uart_port_t *port = &device->port;
        uint32_t ier;

        ier = serial8250_early_in(port, UART_IER);
        serial8250_early_out(port, UART_IER, ier & UART_IER_UUE);
    }
    else
    {
        init_port(device);
    }

    device->con->write = early_serial8250_write;
    // device->con->read = early_serial8250_read;
    return 0;
}

EARLYCON_DECLARE(uart8250, early_serial8250_setup);
