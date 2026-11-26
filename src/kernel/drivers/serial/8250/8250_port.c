/*
 * File: 8250_port.c
 * File Created: 18 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/8250/8250_port.h"
#include "drivers/serial/8250/8250.h"
#include "drivers/serial/serial_port.h"
#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial_reg.h"
#include "drivers/serial/serial.h"

#include "drivers/tty/tty.h"

#include "debug/debug.h"

#include "asm/io_arch.h"

#include "string.h"

#include <config.h>

#define MODULE "8250-port"

typedef struct serial8250_config
{
    const char *name;
    unsigned short fifo_size;
    unsigned short tx_loadsz;
    unsigned char fcr;
    // unsigned char	rxtrig_bytes[UART_FCR_R_TRIG_MAX_STATE];
    unsigned int flags;
} serial8250_config_t;

static const serial8250_config_t uart_config[] = {
    [PORT_UNKNOWN] = {
        .name = "unknown",
        .fifo_size = 1,
        .tx_loadsz = 1,
    },
    [PORT_8250] = {
        .name = "8250",
        .fifo_size = 1,
        .tx_loadsz = 1,
    },
    [PORT_16450] = {
        .name = "16450",
        .fifo_size = 1,
        .tx_loadsz = 1,
    },
    [PORT_16550] = {
        .name = "16550",
        .fifo_size = 1,
        .tx_loadsz = 1,
    },
    [PORT_16550A] = {
        .name = "16550A",
        .fifo_size = 16,
        .tx_loadsz = 16,
        // .fcr = UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
        // .rxtrig_bytes = {1, 4, 8, 14},
        // .flags = UART_CAP_FIFO,
    },
};

/* Uart divisor latch read */
static status_t default_serial_dl_read(uart_8250_port_t *up, uint32_t *out)
{
	/* Assign these in pieces to truncate any bits above 7.  */
	uint32_t dll = 0;
    serial_in(up, UART_DLL, &dll);
	uint32_t dlm = 0;
    serial_in(up, UART_DLM, &dlm);

	*out = dll | dlm << 8;
    return KERRNO_SUCCESS;
}

/* Uart divisor latch write */
static status_t default_serial_dl_write(uart_8250_port_t *up, uint32_t value)
{
	serial_out(up, UART_DLL, value & 0xff);
	return serial_out(up, UART_DLM, value >> 8 & 0xff);
}

#ifdef CONFIG_HAS_IOPORT
static status_t io_serial_in(uart_port_t *p, off_t offset, uint32_t *data_out)
{
    offset = offset << p->regshift;
    *data_out = inb(p->iobase + offset);
    return KERRNO_SUCCESS;
}

static status_t io_serial_out(uart_port_t *p, off_t offset, uint32_t value)
{
    offset = offset << p->regshift;
    outb(value, p->iobase + offset);
    return KERRNO_SUCCESS;
}
#endif

status_t no_serial_in(uart_port_t *p, off_t offset, uint32_t *data_out)
{
    *data_out = ~0u;
    return KERRNO_SUCCESS;
}

status_t no_serial_out(uart_port_t *p, off_t offset, uint32_t value)
{
    return KERRNO_SUCCESS;
}

static void set_io_from_upio(uart_port_t *p)
{
    uart_8250_port_t *up = to_uart_8250_port(p);

    up->dl_read = default_serial_dl_read;
    up->dl_write = default_serial_dl_write;

    switch (p->iotype)
    {
            // #ifdef CONFIG_HAS_IOPORT
            //         case UPIO_HUB6 :
            //             p->serial_in = hub6_serial_in;
            //             p->serial_out = hub6_serial_out;
            //             break;
            // #endif

            //         case UPIO_MEM :
            //             p->serial_in = mem_serial_in;
            //             p->serial_out = mem_serial_out;
            //             break;

            //         case UPIO_MEM16 :
            //             p->serial_in = mem16_serial_in;
            //             p->serial_out = mem16_serial_out;
            //             break;

            //         case UPIO_MEM32 :
            //             p->serial_in = mem32_serial_in;
            //             p->serial_out = mem32_serial_out;
            //             break;

            //         case UPIO_MEM32BE :
            //             p->serial_in = mem32be_serial_in;
            //             p->serial_out = mem32be_serial_out;
            //             break;
            // #ifdef CONFIG_HAS_IOPORT
        case SERIAL_IO_TYPE_IO_PORT :
            p->serial_in = io_serial_in;
            p->serial_out = io_serial_out;
            break;
            // #endif
            // case UPIO_AU :
            //     break;
        default :
            if (p->iotype != SERIAL_IO_TYPE_IO_PORT || p->iobase)
            {
                trace_warn(MODULE, "Unsupported UART type 0x%x", p->iotype);
            }
            p->serial_in = no_serial_in;
            p->serial_out = no_serial_out;
    }

    up->cur_iotype = p->iotype;
    // p->handle_irq = serial8250_default_handle_irq;
}

void serial8250_do_set_mctrl(uart_port_t *port, uart_8250_port_t *up, uint32_t mctrl)
{
    uint8_t mcr;

    mcr = serial8250_TIOCM_to_MCR(mctrl);

    mcr |= up->mcr;

    serial8250_out_MCR(up, mcr);
}

static void serial8250_set_mctrl(uart_port_t *port, uart_8250_port_t *up, uint32_t mctrl)
{
    // ENTER_FUNC("%p, %p, %u", port, up, mctrl);
    if (port->set_mctrl)
    {
        port->set_mctrl(port, mctrl);
    }
    else
    {
        serial8250_do_set_mctrl(port, up, mctrl);
    }
}

static void serial8250_set_fcr(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{
    // ENTER_FUNC("%p, %p, %p", port, up, termios);
    bool is_16750 = port->type == PORT_16750;

    if (is_16750)
    {
        serial_port_out(port, UART_FCR, up->fcr);
    }

    serial_port_out(port, UART_LCR, up->lcr);

    if (is_16750)
    {
        return;
    }

    if (up->fcr & UART_FCR_ENABLE_FIFO)
    {
        serial_port_out(port, UART_FCR, UART_FCR_ENABLE_FIFO);
    }

    serial_port_out(port, UART_FCR, up->fcr);
}

void serial8250_do_set_divisor(uart_port_t *port, uart_8250_port_t *up, speed_t baud, uint32_t quot)
{
    // ENTER_FUNC("%p, %p, %u, %u", port, up, baud, quot);
    // if (is_omap1510_8250(up))
    // {
    //     if (baud == 115200)
    //     {
    //         quot = 1;
    //         serial_port_out(port, UART_OMAP_OSC_12M_SEL, 1);
    //     }
    //     else
    //     {
    //         serial_port_out(port, UART_OMAP_OSC_12M_SEL, 0);
    //     }
    // }

    /*
     * For NatSemi, switch to bank 2 not bank 1, to avoid resetting EXCR2,
     * otherwise just set DLAB
     */
    if (up->capabilities & UART_NATSEMI)
    {
        serial_port_out(port, UART_LCR, 0xE0);
    }
    else
    {
        serial_port_out(port, UART_LCR, up->lcr | UART_LCR_DLAB);
    }

    serial_dl_write(up, quot);
}

static void serial8250_set_divisor(uart_port_t *port, uart_8250_port_t *up, speed_t baud, uint32_t quot, uint32_t quot_frac)
{
    // ENTER_FUNC("%p, %p, %u, %u, %u", port, up, baud, quot, quot_frac);
    if (port->set_divisor)
    {
        port->set_divisor(port, baud, quot, quot_frac);
    }
    else
    {
        serial8250_do_set_divisor(port, up, baud, quot);
    }
}

static void serial8250_set_efr(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{
    uint8_t efr_reg = UART_EFR;
    uint8_t efr = 0;

    if (!(up->capabilities & UART_CAP_EFR))
    {
        return;
    }

    if (termios->c_cflag & CRTSCTS)
    {
        efr |= UART_EFR_CTS;
    }

    if (port->flags & UPF_EXAR_EFR)
    {
        efr_reg = UART_XR_EFR;
    }

    serial_port_out(port, UART_LCR, UART_LCR_CONF_MODE_B);
    serial_port_out(port, efr_reg, efr);
}

static void serial8250_set_ier(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{
    /* CTS flow control flag and modem status interrupts */
    up->ier &= ~UART_IER_MSI;
    if (!(up->bugs & UART_BUG_NOMSR) && UART_ENABLE_MS(&up->port, termios->c_cflag))
    {
        up->ier |= UART_IER_MSI;
    }
    if (up->capabilities & UART_CAP_UUE)
    {
        up->ier |= UART_IER_UUE;
    }
    if (up->capabilities & UART_CAP_RTOIE)
    {
        up->ier |= UART_IER_RTOIE;
    }

    serial_port_out(port, UART_IER, up->ier);
}

static void serial8250_set_errors_and_ignores(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{

    port->read_status_mask = UART_LSR_OE | UART_LSR_DR;
    if (termios->c_iflag & INPCK)
    {
        port->read_status_mask |= UART_LSR_FE | UART_LSR_PE;
    }
    if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
    {
        port->read_status_mask |= UART_LSR_BI;
    }

    /* Characters to ignore */
    port->ignore_status_mask = 0;
    if (termios->c_iflag & IGNPAR)
    {
        port->ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
    }
    if (termios->c_iflag & IGNBRK)
    {
        port->ignore_status_mask |= UART_LSR_BI;

        if (termios->c_iflag & IGNPAR)
        {
            port->ignore_status_mask |= UART_LSR_OE;
        }
    }

    if ((termios->c_cflag & CREAD) == 0)
    {
        port->ignore_status_mask |= UART_LSR_DR;
    }
}

static void serial8250_set_afe(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{
    if (!(up->capabilities & UART_CAP_AFE))
    {
        return;
    }

    up->mcr &= ~UART_MCR_AFE;
    if (termios->c_cflag & CRTSCTS)
    {
        up->mcr |= UART_MCR_AFE;
    }
}

static void serial8250_set_trigger_for_slow_speed(uart_port_t *port, uart_8250_port_t *up, termios_t *termios, unsigned int baud)
{
    // ENTER_FUNC("%p, %p, %p, %u", port, up, termios, baud);
    if (!(up->capabilities & UART_CAP_FIFO))
    {
        return;
    }
    if (port->fifosize <= 1)
    {
        return;
    }
    if (baud >= 2400)
    {
        return;
    }
    // if (up->dma)
    // {
    //     return;
    // }

    up->fcr &= ~UART_FCR_TRIGGER_MASK;
    up->fcr |= UART_FCR_TRIGGER_1;
}

static void serial8250_do_get_divisor(uart_port_t *port, uart_8250_port_t *up, unsigned int baud, uint32_t *out)
{
    // ENTER_FUNC("%p, %p, %u, %p", port, up, baud, out);
    upf_t magic_multiplier = port->flags & UPF_MAGIC_MULTIPLIER;
    uint32_t quot;

    if (magic_multiplier && baud >= port->uartclk / 6)
    {
        quot = 0x8001;
    }
    else if (magic_multiplier && baud >= port->uartclk / 12)
    {
        quot = 0x8002;
    }
    else
    {
        uart_core_get_divisor(port, baud, &quot);
    }

    if (up->bugs & UART_BUG_QUOT && (quot & 0xFF) == 0)
    {
        quot++;
    }

    *out = quot;
}

static status_t serial8250_get_divisor(uart_port_t *port, uart_8250_port_t *up, speed_t baud, uint32_t *frac, uint32_t *out)
{
    // ENTER_FUNC("%p, %p, %u, %p, %p", port, up, baud, frac, out);
    if (port->get_divisor)
    {
        return port->get_divisor(port, baud, frac, out);
    }

    serial8250_do_get_divisor(port, up, baud, out);
    return KERRNO_SUCCESS;
}

static void serial8250_get_baud_rate(uart_port_t *port, uart_8250_port_t *up, termios_t *termios, const termios_t *old, speed_t *out)
{
    unsigned int tolerance = port->uartclk / 100;
    unsigned int min;
    unsigned int max;

    if (port->flags & UPF_MAGIC_MULTIPLIER)
    {
        min = port->uartclk / 16 / UART_DIV_MAX >> 1;
        max = (port->uartclk + tolerance) / 4;
    }
    else
    {
        min = port->uartclk / 16 / UART_DIV_MAX;
        max = (port->uartclk + tolerance) / 16;
    }

    uart_core_get_baud_rate(port, termios, old, min, max, out);
}

static void serial8250_compute_lcr(uart_8250_port_t *up, tcflag_t c_cflag, uint8_t *out)
{
    uint8_t lcr = UART_LCR_WLEN(tty_get_char_size(c_cflag));

    if (c_cflag & CSTOPB)
    {
        lcr |= UART_LCR_STOP;
    }
    if (c_cflag & PARENB)
    {
        lcr |= UART_LCR_PARITY;
    }
    if (!(c_cflag & PARODD))
    {
        lcr |= UART_LCR_EPAR;
    }
    if (c_cflag & CMSPAR)
    {
        lcr |= UART_LCR_SPAR;
    }

    *out = lcr;
}

static void serial8250_set_mini(uart_port_t *port, uart_8250_port_t *up, termios_t *termios)
{
    if (!(up->capabilities & UART_CAP_MINI))
    {
        return;
    }

    termios->c_cflag &= ~(CSTOPB | PARENB | PARODD | CMSPAR);

    tcflag_t csize = termios->c_cflag & CSIZE;
    if (csize == CS5 || csize == CS6)
    {
        termios->c_cflag &= ~CSIZE;
        termios->c_cflag |= CS7;
    }
}

status_t serial8250_do_set_termios(uart_port_t *port, termios_t *termios, const termios_t *old)
{
    ENTER_FUNC("%p, %p, %p", port, termios, old);
    uart_8250_port_t *up = to_uart_8250_port(port);
    speed_t baud;
    uint32_t quot;
    uint32_t frac = 0;
    uint8_t lcr;

    serial8250_set_mini(port, up, termios);
    serial8250_compute_lcr(up, termios->c_cflag, &lcr);
    serial8250_get_baud_rate(port, up, termios, old, &baud);
    serial8250_get_divisor(port, up, baud, &frac, &quot);

    {
        uart_port_lock_irqsave(port);

        up->lcr = lcr;
        serial8250_set_trigger_for_slow_speed(port, up, termios, baud);
        serial8250_set_afe(port, up, termios);
        // uart_update_timeout(port, termios->c_cflag, baud);
        serial8250_set_errors_and_ignores(port, up, termios);
        serial8250_set_ier(port, up, termios);
        serial8250_set_efr(port, up, termios);
        serial8250_set_divisor(port, up, baud, quot, frac);
        serial8250_set_fcr(port, up, termios);
        if (uart_console(port) && termios->c_cflag & CRTSCTS)
        {
            port->mctrl |= TIOCM_RTS;
        }
        serial8250_set_mctrl(port, up, port->mctrl);
        uart_port_unlock_irqsave(port);
    }

    /* Don't rewrite B0 */
    if (tty_termios_baud_rate(termios))
    {
        tty_termios_encode_baud_rate(termios, baud, baud);
    }
    return KERRNO_SUCCESS;
}

status_t serial8250_set_termios(uart_port_t *port, termios_t *termios, const termios_t *old)
{
    if (port->set_termios)
    {
        return port->set_termios(port, termios, old);
    }
    else
    {
        return serial8250_do_set_termios(port, termios, old);
    }
}

const uart_ops_t serial8250_pops = {
    // .tx_empty	= serial8250_tx_empty,
    // .set_mctrl	= serial8250_set_mctrl,
    // .get_mctrl	= serial8250_get_mctrl,
    // .stop_tx	= serial8250_stop_tx,
    // .start_tx	= serial8250_start_tx,
    // .throttle	= serial8250_throttle,
    // .unthrottle	= serial8250_unthrottle,
    // .stop_rx	= serial8250_stop_rx,
    // .enable_ms	= serial8250_enable_ms,
    // .break_ctl	= serial8250_break_ctl,
    // .startup	= serial8250_startup,
    // .shutdown	= serial8250_shutdown,
    // .flush_buffer	= serial8250_flush_buffer,
    .set_termios = serial8250_set_termios,
// .set_ldisc	= serial8250_set_ldisc,
// .pm		= serial8250_pm,
// .type		= serial8250_type,
// .release_port	= serial8250_release_port,
// .request_port	= serial8250_request_port,
// .config_port	= serial8250_config_port,
// .verify_port	= serial8250_verify_port,
#ifdef CONFIG_CONSOLE_POLL
// .poll_get_char = serial8250_get_poll_char,
// .poll_put_char = serial8250_put_poll_char,
#endif
};

INTERNAL uint32_t probe_baud(struct uart_port *port)
{

    uint32_t lcr;
    serial_port_in(port, UART_LCR, &lcr);

    serial_port_out(port, UART_LCR, lcr | UART_LCR_DLAB);

    uint32_t dll;
    serial_port_in(port, UART_DLL, &dll);

    uint32_t dlm;
    serial_port_in(port, UART_DLM, &dlm);

    serial_port_out(port, UART_LCR, lcr);

    uint32_t quot = (dlm << 8) | dll;
    return (port->uartclk / 16) / quot;
}

status_t serial8250_port_console_setup(uart_port_t *port, char *options, bool probe)
{
    ENTER_FUNC("%p, %p, %s", port, options, probe BOOL_TO_STRING);
    uart_8250_port_t *up = to_uart_8250_port(port);
    uint32_t baud = 9600;
    int bits = 8;
    int parity = 'n';
    int flow = 'n';
    status_t ret;

    if (!port->iobase && !port->membase)
    {
        KERRNO_RETURN(KERRNO_DEV_NOT_FOUND, "");
    }

    // up->console_line_ended = true;
    // up->console_msr_work_allow = true;
    // init_irq_work(&up->console_msr_work, console_msr_handler);

    if (options)
    {
        uart_core_parse_options(options, &baud, &parity, &bits, &flow);
    }
    else if (probe)
    {
        baud = probe_baud(port);
    }

    ret = uart_core_set_options(port, port->cons, baud, parity, bits, flow);
    if (ret)
    {
        return ret;
    }

    /* Track user-specified console flow control. */
    uart_port_set_cons_flow_enabled(port, flow == 'r');

    if (port->dev)
    {
        // pm_runtime_get_sync(port->dev);
    }

    return 0;
}

void serial8250_port_init_port(uart_8250_port_t *up)
{
    uart_port_t *port = &up->port;

    // spin_lock_init(&port->lock);
    port->ctrl_id = 0;
    // port->pm = NULL;
    port->ops = &serial8250_pops;
    // port->has_sysrq = IS_ENABLED(CONFIG_SERIAL_8250_CONSOLE);

    up->cur_iotype = SERIAL_IO_TYPE_UNKNOWN;
}

void serial8250_port_set_defaults(uart_8250_port_t *up)
{
    uart_port_t *port = &up->port;

    if (up->port.flags & UPF_FIXED_TYPE)
    {
        uint32_t type = up->port.type;

        if (!up->port.fifosize)
        {
            up->port.fifosize = uart_config[type].fifo_size;
        }
        if (!up->tx_loadsz)
        {
            up->tx_loadsz = uart_config[type].tx_loadsz;
        }
        if (!up->capabilities)
        {
            up->capabilities = uart_config[type].flags;
        }
    }

    set_io_from_upio(port);

    /* default dma handlers */
    // if (up->dma)
    // {
    //     if (!up->dma->tx_dma)
    //     {
    //         up->dma->tx_dma = serial8250_tx_dma;
    //     }
    //     if (!up->dma->rx_dma)
    //     {
    //         up->dma->rx_dma = serial8250_rx_dma;
    //     }
    // }
}
