/*
 * File: 8250.h
 * File Created: 16 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "8250t.h"
#include "8250_port.h"
#include "drivers/serial/serial_reg.h"
#include "drivers/serial/serial_uart_drv.h"

#include <config.h>

#define UART_NR CONFIG_SERIAL_8250_NR_UARTS

extern unsigned int nr_uarts;
extern uart_driver_t serial8250_reg;

enum {
	PLAT8250_DEV_LEGACY = -1,
	PLAT8250_DEV_PLATFORM,
	PLAT8250_DEV_PLATFORM1,
	PLAT8250_DEV_PLATFORM2,
	PLAT8250_DEV_FOURPORT,
	PLAT8250_DEV_ACCENT,
	PLAT8250_DEV_BOCA,
	PLAT8250_DEV_EXAR_ST16C554,
	PLAT8250_DEV_HUB6,
	PLAT8250_DEV_AU1X00,
	PLAT8250_DEV_SM501,
};

static inline status_t serial_in(uart_8250_port_t *up, off_t offset, uint32_t *out)
{
	return up->port.serial_in(&up->port, offset, out);
}

static inline status_t serial_out(uart_8250_port_t *up, off_t offset, uint32_t value)
{
	return up->port.serial_out(&up->port, offset, value);
}

static inline uint8_t serial8250_TIOCM_to_MCR(uint32_t tiocm)
{
    uint8_t mcr = 0;

    if (tiocm & TIOCM_RTS)
    {
        mcr |= UART_MCR_RTS;
    }
    if (tiocm & TIOCM_DTR)
    {
        mcr |= UART_MCR_DTR;
    }
    if (tiocm & TIOCM_OUT1)
    {
        mcr |= UART_MCR_OUT1;
    }
    if (tiocm & TIOCM_OUT2)
    {
        mcr |= UART_MCR_OUT2;
    }
    if (tiocm & TIOCM_LOOP)
    {
        mcr |= UART_MCR_LOOP;
    }

    return mcr;
}

static inline uint32_t serial8250_MSR_to_TIOCM(uint32_t msr)
{
    uint32_t tiocm = 0;

    if (msr & UART_MSR_DCD)
    {
        tiocm |= TIOCM_CAR;
    }
    if (msr & UART_MSR_RI)
    {
        tiocm |= TIOCM_RNG;
    }
    if (msr & UART_MSR_DSR)
    {
        tiocm |= TIOCM_DSR;
    }
    if (msr & UART_MSR_CTS)
    {
        tiocm |= TIOCM_CTS;
    }

    return tiocm;
}

void serial8250_out_MCR(uart_8250_port_t *up, uint32_t value);

// 8250_platform.c

void serial8250_platform_isa_init_ports();

// 8250_core.c

uart_8250_port_t *serial8250_core_get_port(int line);
status_t serial8250_core_setup_port(int index, uart_8250_port_t **out);
void serial8250_core_register_ports(uart_driver_t *drv, device_t *dev);
status_t serial8250_core_register_8250_port(const uart_8250_port_t *up);

// 8250_port.c

status_t serial8250_port_console_setup(uart_port_t *port, char *options, bool probe);
void serial8250_port_init_port(uart_8250_port_t *up);
void serial8250_port_set_defaults(uart_8250_port_t *up);
