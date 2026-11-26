/*
 * File: earlycon.c
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "console/earlycon.h"
#include "console/console.h"

#include "drivers/serial/serial_core.h"

#include "asm/serial_port_arch.h"

#include "cmdline/cmdline.h"

#include "math.h"
#include "string.h"
#include "kerrno.h"
#include "init.h"

#define MODULE "earlycon"

static console_t early_con = {
    .name = "uart",
    .flags = CON_PRINTBUFFER | CON_BOOT,
    .index = 0,
};

static earlycon_device_t early_console_dev = {
    .con = &early_con,
};

extern const earlycon_id_t earlycon_table_start[];
extern const earlycon_id_t earlycon_table_end[];

__init static void *earlycon_map(paddr_t paddr, size_t size)
{
    /*     void *base;
    #ifdef CONFIG_FIX_EARLYCON_MEM
        set_fixmap_io(FIX_EARLYCON_MEM_BASE, paddr & PAGE_MASK);
        base = (void __iomem *)__fix_to_virt(FIX_EARLYCON_MEM_BASE);
        base += paddr & ~PAGE_MASK;
    #else
        base = ioremap(paddr, size);
    #endif
        if (!base)
        {
            pr_err("%s: Couldn't map %pa", __func__, &paddr);
        }

        return base; */
        return NULL;
}

__init static void earlycon_init(earlycon_device_t *device, const char *name)
{
    console_t *earlycon = device->con;
    const char *s;
    // size_t len;

    /* scan backwards from end of string for first non-numeral */
    for (s = name + strlen(name); s > name && s[-1] >= '0' && s[-1] <= '9'; s--);
    if (*s)
    {
        strtoul(s, 10, (uint32_t *)&earlycon->index);
    }
    // len = s - name;
    strncpy(earlycon->name, name, sizeof(earlycon->name));
    earlycon->data = &early_console_dev;
}

__init static void earlycon_print_info(earlycon_device_t *device)
{
    struct console *earlycon = device->con;
    struct uart_port *port = &device->port;
    char ioinfos[64];

    uart_core_get_ioinfos(port, ioinfos, sizeof(ioinfos));

    pr_info("%s %d %s (options '%s')", earlycon->name, earlycon->index, ioinfos, device->options);
    pr_info("%s %d %s (options '%s')", earlycon->name, earlycon->index, ioinfos, device->options);
}

__init static status_t parse_options(earlycon_device_t *device, char *options)
{
    uart_port_t *port = &device->port;
    int length;
    paddr_t addr;

    if (uart_core_parse_earlycon(options, &port->iotype, &addr, &options))
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "");
    }

    switch (port->iotype)
    {
        case SERIAL_IO_TYPE_MEM :
            port->mapbase = addr;
            break;
        case SERIAL_IO_TYPE_MEM16 :
            port->regshift = 1;
            port->mapbase = addr;
            break;
        case SERIAL_IO_TYPE_MEM32 :
        case SERIAL_IO_TYPE_MEM32BE :
            port->regshift = 2;
            port->mapbase = addr;
            break;
        case SERIAL_IO_TYPE_IO_PORT :
            port->iobase = addr;
            break;
        default :
            KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "");
    }

    if (options)
    {
        char *uartclk;

        strtoul(options, 0, (uint32_t *)&device->baud);
        uartclk = strchr(options, ',');
        if (uartclk && strtoul(uartclk + 1, 0, &port->uartclk))
        {
            pr_warn("[%s] unsupported earlycon uart clkrate option", options);
        }
        length = min(strcspn(options, " ") + 1, (size_t)(sizeof(device->options)));
        strncpy(device->options, options, length);
    }

    return KERRNO_SUCCESS;
}

static __init status_t register_earlycon(char *buf, const earlycon_id_t *match)
{
    status_t err;
    uart_port_t *port = &early_console_dev.port;

    /* On parsing error, pass the options buf to the setup function */
    if (buf && !parse_options(&early_console_dev, buf))
    {
        buf = NULL;
    }

    if (!port->uartclk)
    {
        port->uartclk = BASE_BAUD * 16;
    }
    if (port->mapbase)
    {
        port->membase = (vaddr_t)earlycon_map(port->mapbase, 64);
    }

    earlycon_init(&early_console_dev, match->name);
    err = match->setup(&early_console_dev, buf);
    if (err < 0)
    {
        return err;
    }
    if (!early_console_dev.con->write)
    {
        KERRNO_RETURN_NO_PRINT(ENODEV, "");
    }
    console_register(early_console_dev.con);

    earlycon_print_info(&early_console_dev);

    return KERRNO_SUCCESS;
}

__init status_t setup_earlycon(char *buf)
{
    const earlycon_id_t *match;
    bool empty_compatible = true;

    if (!buf || !buf[0])
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_BAD_VALUE, "");
    }

    if (console_is_registered(&early_con))
    {
        KERRNO_RETURN_NO_PRINT(KERRNO_ALREADY, "");
    }

again:
    for (match = earlycon_table_start; match < earlycon_table_end; match++)
    {
        size_t len = strlen(match->name);

        if (strncmp(buf, match->name, len))
        {
            continue;
        }

        /* prefer entries with empty compatible */
        if (empty_compatible && *match->compatible)
        {
            continue;
        }

        if (buf[len])
        {
            if (buf[len] != ',')
            {
                continue;
            }
            buf += len + 1;
        }
        else
        {
            buf = NULL;
        }

        return register_earlycon(buf, match);
    }

    if (empty_compatible)
    {
        empty_compatible = false;
        goto again;
    }

    KERRNO_RETURN_NO_PRINT(KERRNO_ENTRY_NOT_FOUND, "");
}

__init status_t param_setup_earlycon(const char *buf)
{
    status_t err;

    if (!buf || !buf[0])
    {
        // if (IS_ENABLED(CONFIG_ACPI_SPCR_TABLE))
        // {
        //     earlycon_acpi_spcr_enable = true;
        //     return 0;
        // }
        // else if (!buf)
        // {
        // return early_init_dt_scan_chosen_stdout();
        // }
    }

    err = setup_earlycon((char *)buf);
    if (err)
    {
        return 0;
    }
    return err;
}

CMDLINE_PARAM("earlycon", param_setup_earlycon, 1);
