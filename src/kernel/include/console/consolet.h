/*
 * File: consolet.h
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "drivers/tty/ttyt.h"

#include <types.h>

typedef enum cons_flags
{
    CON_PRINTBUFFER = BIT(0),
    CON_CONSDEV = BIT(1),
    CON_ENABLED = BIT(2),
    CON_BOOT = BIT(3),
    CON_ANYTIME = BIT(4),
    CON_BRL = BIT(5),
    CON_EXTENDED = BIT(6),
    CON_SUSPENDED = BIT(7),
    CON_NBCON = BIT(8),
    CON_NBCON_ATOMIC_UNSAFE = BIT(9),
} cons_flags_t;

typedef struct console console_t;

typedef struct console
{
    char name[16];
    status_t (*write)(console_t *co, wchar_t *s, size_t count);
    status_t (*read)(console_t *co, wchar_t *s, size_t count);
    status_t (*device)(console_t *co, int *index, tty_driver_t **tty_out);
    status_t (*unblank)(void);
    status_t (*setup)(console_t *co, char *options);
    status_t (*exit)(console_t *co);
    status_t (*match)(console_t *co, char *name, int idx, char *options);
    cons_flags_t flags;
    short index;
    int cflag;
    uint32_t ispeed;
    uint32_t ospeed;
    uint64_t seq;
    unsigned long dropped;
    void *data;
} console_t;
