/*
 * File: serial_core.h
 * File Created: 16 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "console/consolet.h"
#include "drivers/tty/ttyt.h"
#include "drivers/tty/tty_flags.h"
#include "drivers/serial/serial_uart_drv.h"
#include "drivers/serial/serial_port.h"

#ifdef CONFIG_SERIAL_CORE_CONSOLE
#define uart_console(port)                                \
    ((port)->cons && (port)->cons->index == (port)->line)
#else
#define uart_console(port) ({ (void)port; 0; })
#endif

#ifdef CONFIG_HAS_IOPORT
#define UPF_FOURPORT ((upf_t)ASYNC_FOURPORT /* 1  */)
#else
#define UPF_FOURPORT 0
#endif
#define UPF_SAK                     ((upf_t)ASYNC_SAK /* 2  */)
#define UPF_SPD_HI                  ((upf_t)ASYNC_SPD_HI /* 4  */)
#define UPF_SPD_VHI                 ((upf_t)ASYNC_SPD_VHI /* 5  */)
#define UPF_SPD_CUST                ((upf_t)ASYNC_SPD_CUST /* 0x0030 */)
#define UPF_SPD_WARP                ((upf_t)ASYNC_SPD_WARP /* 0x1010 */)
#define UPF_SPD_MASK                ((upf_t)ASYNC_SPD_MASK /* 0x1030 */)
#define UPF_SKIP_TEST               ((upf_t)ASYNC_SKIP_TEST /* 6  */)
#define UPF_AUTO_IRQ                ((upf_t)ASYNC_AUTO_IRQ /* 7  */)
#define UPF_HARDPPS_CD              ((upf_t)ASYNC_HARDPPS_CD /* 11 */)
#define UPF_SPD_SHI                 ((upf_t)ASYNC_SPD_SHI /* 12 */)
#define UPF_LOW_LATENCY             ((upf_t)ASYNC_LOW_LATENCY /* 13 */)
#define UPF_BUGGY_UART              ((upf_t)ASYNC_BUGGY_UART /* 14 */)
#define UPF_MAGIC_MULTIPLIER        ((upf_t)ASYNC_MAGIC_MULTIPLIER /* 16 */)

#define UART_ENABLE_MS(port, cflag) ((port)->flags & UPF_HARDPPS_CD || \
                                     (cflag) & CRTSCTS ||              \
                                     !((cflag) & CLOCAL))

bool uart_iotype_mmio(serial_io_type_t iotype);
bool uart_iotype_io(serial_io_type_t iotype);

status_t uart_core_console_write(struct uart_port *port, const char *s, size_t count, status_t (*putchar)(uart_port_t *, unsigned char));
status_t uart_core_parse_earlycon(char *p, serial_io_type_t *iotype, paddr_t *addr, char **options);
void uart_core_get_ioinfos(uart_port_t *port, char *buf, size_t size);
void uart_core_get_baud_rate(uart_port_t *port, termios_t *termios, const termios_t *old, uint32_t min, uint32_t max, speed_t *out);
void uart_core_get_divisor(uart_port_t *port, speed_t baud, uint32_t *out);
void uart_core_parse_options(const char *options, uint32_t *baud, int *parity, int *bits, int *flow);
status_t uart_core_console_device(console_t *co, int *index, tty_driver_t **out_drv);
bool uart_core_match_port(const uart_port_t *port1, const uart_port_t *port2);
status_t uart_core_register_driver(uart_driver_t *uart_drv);
status_t uart_core_set_options(uart_port_t *port, console_t *co, int baud, int parity, int bits, int flow);

status_t serial_core_register_port(uart_driver_t *uart_drv, uart_port_t *uart_port);

// serial_port.c

status_t uart_add_one_port(uart_driver_t *drv, uart_port_t *port);
void uart_remove_one_port(uart_driver_t *drv, uart_port_t *port);
status_t serial_base_port_init();

// serial_ctrl.c

status_t serial_ctrl_register_port(uart_driver_t *drv, uart_port_t *port);
void serial_ctrl_unregister_port(uart_driver_t *drv, uart_port_t *port);

status_t serial_base_ctrl_init();
status_t serial_base_ctrl_exit();
