/*
 * File: serial_core.c
 * File Created: 16 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/serial/serial_core.h"
#include "drivers/serial/serial_base.h"
#include "drivers/serial/serial_uart_drv.h"
#include "drivers/serial/serial_core_types.h"

#include "drivers/tty/tty.h"

#include "sync/mutex.h"

#include "kerrno.h"
#include "memory.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

#include <defs.h>

#define MODULE "serial-core"

mutex_t port_mutex;

static const char *uart_get_mmio_width(uart_port_t *port)
{
    switch (port->iotype)
    {
        case SERIAL_IO_TYPE_MEM16 :
            return "16";
        case SERIAL_IO_TYPE_MEM32 :
        case SERIAL_IO_TYPE_MEM32BE :
            return "32";
        case SERIAL_IO_TYPE_AU :
        case SERIAL_IO_TYPE_MEM :
        default :
            return "";
    }
}

bool uart_iotype_mmio(serial_io_type_t iotype)
{
    switch (iotype)
    {
        case SERIAL_IO_TYPE_MEM :
        case SERIAL_IO_TYPE_MEM32 :
        case SERIAL_IO_TYPE_AU :
        case SERIAL_IO_TYPE_TSI :
        case SERIAL_IO_TYPE_MEM32BE :
        case SERIAL_IO_TYPE_MEM16 :
            return true;
        default :
            return false;
    }
}

bool uart_iotype_io(serial_io_type_t iotype)
{
    switch (iotype)
    {
        case SERIAL_IO_TYPE_IO_PORT :
        case SERIAL_IO_TYPE_HUB6 :
            return true;
        default :
            return false;
    }
}

void uart_core_get_ioinfos(uart_port_t *port, char *buf, size_t size)
{
    int end = 0;
    if (uart_iotype_mmio(port->iotype))
    {
        end = scnprintf(buf, size, " MMIO%s:%pa", uart_get_mmio_width(port), &port->mapbase);
    }
    else if (uart_iotype_io(port->iotype))
    {
        if (port->iotype == SERIAL_IO_TYPE_IO_PORT)
        {
            end = scnprintf(buf, size, " I/O:0x%lx", port->iobase);
        }
        else if (port->iotype == SERIAL_IO_TYPE_HUB6)
        {
            end = scnprintf(buf, size, " I/O:0x%lx, offset 0x%x", port->iobase, /* port->hub6 */ 0);
        }
    }
    buf[end] = '\0';
}

void uart_core_get_baud_rate(uart_port_t *port, termios_t *termios, const termios_t *old, uint32_t min, uint32_t max, speed_t *out)
{
    // ENTER_FUNC("%p, %p, %p, %u, %u, %p", port, termios, old, min, max, out);
    uint32_t try;
    speed_t baud;
    speed_t altbaud;
    upf_t flags = port->flags & UPF_SPD_MASK;

    switch (flags)
    {
        case UPF_SPD_HI :
            altbaud = 57600;
            break;
        case UPF_SPD_VHI :
            altbaud = 115200;
            break;
        case UPF_SPD_SHI :
            altbaud = 230400;
            break;
        case UPF_SPD_WARP :
            altbaud = 460800;
            break;
        default :
            altbaud = 38400;
            break;
    }

    for (try = 0; try < 2; try++)
    {
        baud = tty_termios_baud_rate(termios);

        if (try == 0 && baud == 38400)
        {
            baud = altbaud;
        }

        if (baud == 0)
        {
            *out = max(min, 9600);
            return;
        }

        if (baud >= min && baud <= max)
        {
            *out = baud;
            return;
        }

        termios->c_cflag &= ~CBAUD;
        if (old)
        {
            baud = tty_termios_baud_rate(old);
            tty_termios_encode_baud_rate(termios, baud, baud);
            old = NULL;
            continue;
        }

        if (baud <= min)
        {
            baud = min + 1;
        }
        else
        {
            baud = max - 1;
        }

        tty_termios_encode_baud_rate(termios, baud, baud);
    }

    KERNEL_PANIC(MODULE, "This should never happen");
}

void uart_core_get_divisor(uart_port_t *port, speed_t baud, uint32_t *out)
{
    uint32_t quot;

    /*
     * Old custom speed handling.
     */
    if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST)
    {
        quot = port->custom_divisor;
    }
    else
    {
        quot = DIV_ROUND_CLOSEST(port->uartclk, 16 * baud);
    }

    *out = quot;
}

#if CONFIG_SERIAL_CORE_CONSOLE == 1

status_t uart_core_console_write(struct uart_port *port, const char *s, size_t count, status_t (*putchar)(uart_port_t *, unsigned char))
{
    size_t i;

    for (i = 0; i < count; i++, s++)
    {
        if (*s == '\n')
        {
            putchar(port, '\r');
        }
        putchar(port, *s);
    }
    return KERRNO_SUCCESS;
}

void uart_core_parse_options(const char *options, uint32_t *baud, int *parity, int *bits, int *flow)
{
    const char *s = options;

    strtoul(s, 10, baud);
    while (*s >= '0' && *s <= '9')
    {
        s++;
    }
    if (*s)
    {
        *parity = *s++;
    }
    if (*s)
    {
        *bits = *s++ - '0';
    }
    if (*s)
    {
        *flow = *s;
    }
}

status_t uart_core_parse_earlycon(char *p, serial_io_type_t *iotype, paddr_t *addr, char **options)
{
    if (strncmp(p, "mmio,", 5) == 0)
    {
        *iotype = SERIAL_IO_TYPE_MEM;
        p += 5;
    }
    else if (strncmp(p, "mmio16,", 7) == 0)
    {
        *iotype = SERIAL_IO_TYPE_MEM16;
        p += 7;
    }
    else if (strncmp(p, "mmio32,", 7) == 0)
    {
        *iotype = SERIAL_IO_TYPE_MEM32;
        p += 7;
    }
    else if (strncmp(p, "mmio32be,", 9) == 0)
    {
        *iotype = SERIAL_IO_TYPE_MEM32BE;
        p += 9;
    }
    else if (strncmp(p, "mmio32native,", 13) == 0)
    {
        *iotype = IS_ENABLED(CONFIG_CPU_BIG_ENDIAN) ? SERIAL_IO_TYPE_MEM32BE : SERIAL_IO_TYPE_MEM32;
        p += 13;
    }
    else if (strncmp(p, "io,", 3) == 0)
    {
        *iotype = SERIAL_IO_TYPE_IO_PORT;
        p += 3;
    }
    else if (strncmp(p, "0x", 2) == 0)
    {
        *iotype = SERIAL_IO_TYPE_MEM;
    }
    else
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "");
    }

    strtoull(p, 0, (uint64_t *)addr);
    p = strchr(p, ',');
    if (p)
    {
        p++;
    }

    *options = p;
    return KERRNO_SUCCESS;
}

status_t uart_core_set_options(uart_port_t *port, console_t *co, int baud, int parity, int bits, int flow)
{
    ENTER_FUNC("%p, %p, %u, %u, %u, %u", port, co, baud, parity, bits, flow);
    if (/* !uart_console_registered_locked(port) && */ !port->console_reinit)
    {
        // uart_port_spin_lock_init(port);
    }

    termios_t termios;
    memset(&termios, 0, sizeof(termios_t));

    termios.c_cflag |= CREAD | HUPCL | CLOCAL;
    tty_termios_encode_baud_rate(&termios, baud, baud);

    if (bits == 7)
    {
        termios.c_cflag |= CS7;
    }
    else
    {
        termios.c_cflag |= CS8;
    }

    switch (parity)
    {
        case 'o' :
        case 'O' :
            termios.c_cflag |= PARODD;
            FALLTHROUGH;
        case 'e' :
        case 'E' :
            termios.c_cflag |= PARENB;
            break;
    }

    if (flow == 'r')
    {
        termios.c_cflag |= CRTSCTS;
    }

    port->mctrl |= TIOCM_DTR;

    termios_t dummy;
    port->ops->set_termios(port, &termios, &dummy);
    trace_debug(MODULE, "here1");
    if (flow == 'r' && (termios.c_cflag & CRTSCTS))
    {
        uart_port_lock_irqsave(port);
        port->status |= UPSTAT_CTS_ENABLE;
        uart_port_unlock_irqsave(port);
    }

    if (co)
    {
        co->cflag = termios.c_cflag;
        co->ispeed = termios.c_ispeed;
        co->ospeed = termios.c_ospeed;
    }

    return KERRNO_SUCCESS;
}

#endif

static const tty_ops_t uart_ops = {
    // .install	= uart_install,
    // .open		= uart_open,
    // .close		= uart_close,
    // .write		= uart_write,
    // .put_char	= uart_put_char,
    // .flush_chars	= uart_flush_chars,
    // .write_room	= uart_write_room,
    // .chars_in_buffer= uart_chars_in_buffer,
    // .flush_buffer	= uart_flush_buffer,
    // .ioctl		= uart_ioctl,
    // .throttle	= uart_throttle,
    // .unthrottle	= uart_unthrottle,
    // .send_xchar	= uart_send_xchar,
    // .set_termios	= uart_set_termios,
    // .set_ldisc	= uart_set_ldisc,
    // .stop		= uart_stop,
    // .start		= uart_start,
    // .hangup		= uart_hangup,
    // .break_ctl	= uart_break_ctl,
    // .wait_until_sent= uart_wait_until_sent,
    // #ifdef CONFIG_PROC_FS
    // .proc_show	= uart_proc_show,
    // #endif
    // .tiocmget	= uart_tiocmget,
    // .tiocmset	= uart_tiocmset,
    // .set_serial	= uart_set_info_user,
    // .get_serial	= uart_get_info_user,
    // .get_icount	= uart_get_icount,
    // #ifdef CONFIG_CONSOLE_POLL
    // 	.poll_init	= uart_poll_init,
    // 	.poll_get_char	= uart_poll_get_char,
    // 	.poll_put_char	= uart_poll_put_char,
    // #endif
};

status_t uart_core_register_driver(uart_driver_t *uart_drv)
{
    uart_drv->state = kzmalloc(sizeof(uart_state_t) * uart_drv->nr);
    if (uart_drv->state)
    {
        KERRNO_RETURN(KERRNO_ALLOC_FAILED);
    }

    tty_driver_t *tty_drv;
    status_t ret = tty_alloc_driver(uart_drv->nr, TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS, &tty_drv);
    if (ret)
    {
        KERNEL_PANIC(MODULE, "Couldn't allocate uart driver");
    }

    uart_drv->tty_driver = tty_drv;

    tty_drv->driver_name = uart_drv->driver_name;
    tty_drv->name = uart_drv->dev_name;
    tty_drv->device = 2;
    tty_drv->major = uart_drv->major;
    tty_drv->minor_start = uart_drv->minor;
    tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
    tty_drv->subtype = SERIAL_TYPE_NORMAL;
    tty_drv->init_termios = tty_std_termios;
    tty_drv->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    tty_drv->init_termios.c_ispeed = tty_drv->init_termios.c_ospeed = 9600;
    tty_drv->driver_state = uart_drv;
    tty_set_operations(tty_drv, &uart_ops);

    ret = tty_register_driver(tty_drv, NULL);
    if (!ret)
    {
        return ret;
    }
    KERNEL_PANIC(MODULE, "Couldn't register uart driver");
    return ret;
}

status_t uart_core_console_device(console_t *co, int *index, tty_driver_t **out_drv)
{
    ENTER_FUNC("%p, %p, %p", co, index, out_drv);
    uart_driver_t *p = co->data;
    *index = co->index;
    *out_drv = p->tty_driver;
    return KERRNO_SUCCESS;
}

INTERNAL serial_ctrl_device_t *serial_core_get_ctrl_dev(serial_port_device_t *port_dev)
{
    device_t *dev = &port_dev->dev;

    return to_serial_base_ctrl_device(dev->parent);
}

serial_ctrl_device_t *serial_core_ctrl_find(uart_driver_t *drv, device_t *phys_dev, int ctrl_id)
{
    ENTER_FUNC("%p, %p, 0x%x", drv, phys_dev, ctrl_id);
    uart_state_t *state;
    int i;

    for (i = 0; i < drv->nr; i++)
    {
        state = drv->state + i;
        if (!state->uart_port || !state->uart_port->port_dev)
        {
            continue;
        }

        if (state->uart_port->dev == phys_dev &&
            state->uart_port->ctrl_id == ctrl_id)
        {
            return serial_core_get_ctrl_dev(state->uart_port->port_dev);
        }
    }

    return NULL;
}

INTERNAL status_t serial_core_ctrl_device_add(uart_port_t *uart_port, serial_ctrl_device_t **out)
{
    return serial_base_ctrl_add(uart_port, uart_port->dev, out);
}

INTERNAL status_t serial_core_port_device_add(serial_ctrl_device_t *ctrl_dev, uart_port_t *uart_port)
{
    serial_port_device_t *port_dev = NULL;

    status_t ret = 0;

    ret = serial_base_port_add(uart_port, ctrl_dev, &port_dev);
    if (ret)
    {
        return ret;
    }

    uart_port->port_dev = port_dev;

    return KERRNO_SUCCESS;
}

status_t serial_core_register_port(uart_driver_t *uart_drv, uart_port_t *uart_port)
{
    ENTER_FUNC("%p, %p", uart_drv, uart_port)
    uart_port->flags |= UPF_DEAD;

    status_t ret = 0;
    serial_ctrl_device_t *ctrl_dev = serial_core_ctrl_find(uart_drv, uart_port->dev, uart_port->ctrl_id);
    if (!ctrl_dev)
    {
        ret = serial_core_ctrl_device_add(uart_port, &ctrl_dev);
        KERRNO_CHECK_AND_RETURN(ret);
    }
    
    ret = serial_core_port_device_add(ctrl_dev, uart_port);
    KERRNO_CHECK_AND_RETURN(ret);

    return KERRNO_SUCCESS;
}

