/*
 * File: 8250_core_types.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/serial/serial_core_types.h"

#include "resource/resourcet.h"

#include <types.h>

typedef struct old_serial_port
{
    uint32_t uart;
    uint32_t baud_base;
    kernel_irq_t irq;
    upf_t flags;
    serial_io_type_t io_type;
    arch_port_t port;
    paddr_t iomem_base;
    uint16_t iomem_reg_shift;
} old_serial_port_t;

struct uart_8250_port;

typedef struct uart_8250_ops
{
    int (*setup_irq)(struct uart_8250_port *);
    void (*release_irq)(struct uart_8250_port *);
    void (*setup_timer)(struct uart_8250_port *);
} uart_8250_ops_t;

typedef struct uart_8250_port
{
    uart_port_t port;
    uint32_t capabilities;  /* port capabilities */
    uint16_t bugs;          /* port bugs */
    unsigned int tx_loadsz; /* transmit fifo load size */
    uint32_t acr;
    uint32_t fcr;
    uint32_t ier;
    uint32_t lcr;
    uint32_t mcr;
    serial_io_type_t cur_iotype; /* Running I/O type */
    unsigned int rpm_tx_active;
    unsigned char canary;        /* non-zero during system sleep if no_console_suspend */
    unsigned char probe;

#define LSR_SAVE_FLAGS UART_LSR_BRK_ERROR_BITS
    uint16_t lsr_saved_flags;
    uint16_t lsr_save_mask;

    bool console_line_ended;

    /* Allow queuing irq_work for MSR handling */
    bool console_msr_work_allow;

    unsigned char msr_saved_flags;
    // struct irq_work console_msr_work;

    // struct uart_8250_dma *dma;
    const uart_8250_ops_t *ops;

    status_t (*dl_read)(struct uart_8250_port *uart_8250_p, uint32_t *out_data);
    status_t (*dl_write)(struct uart_8250_port *uart_8250_p, uint32_t value);
} uart_8250_port_t;

typedef struct plat_serial8250_port
{
    arch_port_t iobase;      /* io base address */
    vaddr_t membase;         /* ioremap cookie or NULL */
    resource_size_t mapbase; /* resource base */
    resource_size_t mapsize;
    uint32_t uartclk;        /* UART clock rate */
    kernel_irq_t irq;        /* interrupt number */
    uint32_64_t irqflags;    /* request_irq flags */
    void *private_data;
    uint8_t regshift;        /* register shift */
    serial_io_type_t iotype; /* UPIO_* */
    // uint8_t hub6;
    uint8_t has_sysrq; /* supports magic SysRq */
    uint32_t type;     /* If UPF_FIXED_TYPE */
    upf_t flags;       /* UPF_* flags */
    uint16_t bugs;     /* port bugs */

    status_t (*dl_read)(uart_8250_port_t *uart_8250_p, uint32_t *out_data);
    status_t (*dl_write)(uart_8250_port_t *uart_8250_p, uint32_t value);

    status_t (*serial_in)(uart_port_t *port, off_t offset, uint32_t *data_out);
    status_t (*serial_out)(uart_port_t *port, off_t offset, uint32_t val);
    status_t (*set_termios)(uart_port_t *port, termios_t *termios, const termios_t *old);
    status_t (*set_ldisc)(uart_port_t *port, termios_t *);
    status_t (*get_mctrl)(uart_port_t *port, uint32_t *out_mctrl);
    status_t (*handle_irq)(uart_port_t *port);
    // void (*pm)(struct uart_port *, unsigned int state, unsigned old);
    status_t (*handle_break)(uart_port_t *);
} plat_serial8250_port_t;
