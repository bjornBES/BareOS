/*
 * File: UART.c
 * File Created: 04 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "UART.h"
#include "stdio.h"

#include <types.h>
#include "device/device.h"
#include "drivers/IO/tty/tty_types.h"

#include "kernel/io.h"
#include "kernel/memory.h"

typedef struct UART_dev_priv
{
    uint16_t port;
} UART_dev_priv_t;

int UART_serial_received(uint16_t port)
{
    return inb(port + UART_LINE_STATUS_REGISTER) & 1;
}

int UART_is_transmit_empty(uint16_t port)
{
    return inb(port + UART_LINE_STATUS_REGISTER) & 0x20;
}

uint8_t UART_read(uint16_t port, UART_port off)
{
    while (UART_serial_received(port) == 0);

    return inb(port + off);
}

void UART_write(uint16_t port, UART_port off, uint8_t data)
{
    while (UART_is_transmit_empty(port) == 0);

    outb(port + off, data);
}

size_t UART_write_data(void *buffer, off_t offset, size_t count, device_t *device)
{
    uint8_t *data = (uint8_t *)buffer;
    UART_dev_priv_t *priv = device->priv;
    for (size_t i = 0; i < count; i++)
    {
        UART_write(priv->port, UART_DATA, data[i]);
    }
    return count;
}

void UART_write_buf(device_t *device, const uint8_t *buffer, size_t count)
{
    UART_write_data((void*)buffer, 0, count, device);
}

void UART_write_char(device_t *device, char c)
{
    UART_dev_priv_t *priv = device->priv;
    UART_write(priv->port, UART_DATA, c);
}

void UART_write_str(uint16_t port, char *data)
{
    while (*data)
    {
        UART_write(port, UART_DATA, *data);
        data++;
    }
}

void UART_write_fstr(uint16_t port, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char str[1024];
    vsprintf(str, format, args);
    UART_write_str(port, str);
    va_end(args);
}

void uart_get_ops(device_t *dev, tty_dev_ops_t *ops)
{
    ops->write_char = UART_write_char;
    ops->write_buf = UART_write_buf;
}

int UART_init(uint16_t port)
{
    UART_write(port, UART_INTERRUPT_ENABLE, 0x00);       // Disable all interrupts
    UART_write(port, UART_LINE_CONTROL_REGISTER, 0x80);  // Enable DLAB (set baud rate divisor)
    UART_write(port, UART_DLAB_LSB, 0x03);               // Set divisor to 3 (lo byte) 38400 baud
    UART_write(port, UART_DLAB_MSB, 0x00);               // (hi byte)
    UART_write(port, UART_LINE_CONTROL_REGISTER, 0x03);  // 8 bits, no parity, one stop bit
    UART_write(port, UART_FIFO_CONTROL_REGISTER, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x0B); // IRQs enabled, RTS/DSR set
    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x1E); // Set in loopback mode, test the serial chip
    UART_write(port, UART_DATA, 0xAE);                   // Test serial chip (send byte 0xAE and check if serial returns same byte)

    if (UART_read(port, UART_DATA) != 0xAE)
    {
        return RETURN_FAILED;
    }

    UART_write(port, UART_MODEM_CONTROL_REGISTER, 0x0F);

    device_t *dev = malloc(sizeof(device_t));
    dev->class_name = "uart";
    dev->type = DEVICE_BLOCK;
    dev->flags = DEVICE_FLAG_WRITABLE | DEVICE_FLAG_BLOCKDEV;
    dev->write = UART_write_data;
    dev->tty_ops = uart_get_ops;

    UART_dev_priv_t *priv = malloc(sizeof(UART_dev_priv_t));
    priv->port = port;
    dev->priv = priv;

    device_register(dev);

    return RETURN_GOOD;
}
