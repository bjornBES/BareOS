/*
 * File: serial_core_types.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "serial_io_types.h"

#include "idalloc/ida/idat.h"

#include "dev/devicet.h"

#include "drivers/tty/tty_core_types.h"

#include "resource/resourcet.h"

#include "console/consolet.h"

#include <types.h>

typedef uint64_t upf_t;
typedef uint32_t upstat_t;

typedef struct uart_icount
{
    uint32_t cts;
    uint32_t dsr;
    uint32_t rng;
    uint32_t dcd;
    uint32_t rx;
    uint32_t tx;
    uint32_t frame;
    uint32_t overrun;
    uint32_t parity;
    uint32_t brk;
    uint32_t buf_overrun;
} uart_icount_t;

typedef struct serial_ctrl_device
{
    device_t dev;
    ida_t port_ida;
} serial_ctrl_device_t;

#define to_serial_base_ctrl_device(d) container_of((d), serial_ctrl_device_t, dev)

typedef struct serial_port_device
{
    device_t dev;
    struct uart_port *port;
    unsigned int tx_enabled : 1;
} serial_port_device_t;

#define to_serial_base_port_device(d) container_of((d), serial_port_device_t, dev)

typedef struct serial_struct
{
    int type;
    int line;
    arch_port_t port;
    kernel_irq_t irq;
    int flags;
    int xmit_fifo_size;
    int custom_divisor;
    int baud_base;
    unsigned short close_delay;
    serial_io_type_t io_type;
    char reserved_char[1];
    int hub6;
    unsigned short closing_wait;  /* time to wait before closing */
    unsigned short closing_wait2; /* no longer used... */
    unsigned char *iomem_base;
    unsigned short iomem_reg_shift;
    unsigned int port_high;
    unsigned long iomap_base; /* cookie passed into ioremap */
} serial_struct_t;

typedef struct uart_state
{
    // struct tty_port		port;

    // enum uart_pm_state	pm_state;

    refcount_t refcount;
    // wait_queue_head_t	remove_wait;
    struct uart_port *uart_port;
} uart_state_t;

typedef struct uart_ops
{
    status_t (*tx_empty)(struct uart_port *);
    status_t (*set_mctrl)(struct uart_port *, uint32_t mctrl);
    status_t (*get_mctrl)(struct uart_port *, uint32_t *out_mctrl);
    status_t (*stop_tx)(struct uart_port *);
    status_t (*start_tx)(struct uart_port *);
    status_t (*throttle)(struct uart_port *);
    status_t (*unthrottle)(struct uart_port *);
    status_t (*send_xchar)(struct uart_port *, char ch);
    status_t (*stop_rx)(struct uart_port *);
    status_t (*start_rx)(struct uart_port *);
    status_t (*enable_ms)(struct uart_port *);
    status_t (*break_ctl)(struct uart_port *, int ctl);
    status_t (*startup)(struct uart_port *);
    status_t (*shutdown)(struct uart_port *);
    status_t (*flush_buffer)(struct uart_port *);
    status_t (*set_termios)(struct uart_port *, termios_t *new, const termios_t *old);
    status_t (*set_ldisc)(struct uart_port *, termios_t *);
    // status_t (*pm)(struct uart_port *, uint32_t state, uint32_t oldstate);
    status_t (*type)(struct uart_port *, const char **out);
    status_t (*release_port)(struct uart_port *);
    status_t (*request_port)(struct uart_port *);
    status_t (*config_port)(struct uart_port *, int);
    status_t (*verify_port)(struct uart_port *, struct serial_struct *);
#ifdef CONFIG_CONSOLE_POLL
    status_t (*poll_init)(struct uart_port *);
    status_t (*poll_put_char)(struct uart_port *, unsigned char);
    status_t (*poll_get_char)(struct uart_port *);
#endif
} uart_ops_t;

typedef struct uart_port
{
    spinlock_t lock;    /* port lock */
    arch_port_t iobase; /* in/out[bwl] */
    vaddr_t membase;    /* read/write[bwl] */

    status_t (*serial_in)(struct uart_port *port, off_t offset, uint32_t *data_out);
    status_t (*serial_out)(struct uart_port *port, off_t offset, uint32_t val);
    status_t (*set_termios)(struct uart_port *port, termios_t *termios, const termios_t *old);
    status_t (*set_ldisc)(struct uart_port *port, termios_t *);
    status_t (*get_mctrl)(struct uart_port *port, uint32_t *);
    status_t (*set_mctrl)(struct uart_port *port, uint32_t);
    status_t (*get_divisor)(struct uart_port *port, speed_t baud, uint32_t *frac, uint32_t *out);
    status_t (*set_divisor)(struct uart_port *port, speed_t baud, uint32_t quot, uint32_t quot_frac);
    status_t (*get_rxtrig)(struct uart_port *port);
    status_t (*set_rxtrig)(struct uart_port *port, uint8_t bytes);
    status_t (*startup)(struct uart_port *port);
    status_t (*shutdown)(struct uart_port *port);
    status_t (*throttle)(struct uart_port *port);
    status_t (*unthrottle)(struct uart_port *port);
    status_t (*break_ctl)(struct uart_port *port, int break_state);
    status_t (*handle_irq)(struct uart_port *port);
    // void (*pm)(struct uart_port *port, unsigned int state, unsigned old);
    status_t (*handle_break)(struct uart_port *port);

    id_t ctrl_id;  /* optional serial core controller id */
    id_t port_id;  /* optional serial core port id */
    kernel_irq_t irq;  /* irq number */
    uint64_t irqflags; /* irq flags  */
    uint32_t uartclk;  /* base uart clock */
    uint32_t fifosize; /* tx fifo size */
    uint8_t x_char;    /* xon/xoff char */
    uint8_t regshift;  /* reg offset shift */

    serial_io_type_t iotype;

    uint32_t read_status_mask;   /* driver specific */
    uint32_t ignore_status_mask; /* driver specific */
    uart_state_t *state;         /* pointer to parent state */
    uart_icount_t icount;        /* statistics */

    console_t *cons;             /* struct console, if any */
    /* flags must be updated while holding port mutex */
    upf_t flags;

    /*
     * Must hold termios_rwsem, port mutex and port lock to change;
     * can hold any one lock to read.
     */
    upstat_t status;

    bool hw_stopped;     /* sw-assisted CTS flow state */
    bool cons_flow;      /* user specified console flow control */
    uint32_t mctrl;      /* current modem ctrl settings */
    uint32_t frame_time; /* frame timing in ns */
    uint32_t type;       /* port type */
    const uart_ops_t *ops;
    uint32_t custom_divisor;
    uint32_t line;                  /* port index */
    uint32_t minor;
    resource_size_t mapbase;        /* for ioremap */
    resource_size_t mapsize;
    device_t *dev;                  /* serial port physical parent device */
    serial_port_device_t *port_dev; /* serial core port device */

    uint32_64_t sysrq;              /* sysrq timeout */
    uint8_t sysrq_ch;               /* char for sysrq */
    uint8_t has_sysrq;
    uint8_t sysrq_seq;              /* index in sysrq_toggle_seq */

    uint8_t suspended;
    uint8_t console_reinit;
    const char *name;   /* port name */
    void *private_data; /* generic platform data pointer */
} uart_port_t;

typedef struct uart_driver
{
    const char *driver_name;
    const char *dev_name;
    int major;
    int minor;
    int nr;
    console_t *cons;

    uart_state_t *state;
    tty_driver_t *tty_driver;
} uart_driver_t;
