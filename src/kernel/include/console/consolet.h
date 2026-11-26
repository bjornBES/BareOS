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
#include "debug/trace.h"

#include <types.h>

#define PRINTK_MESSAGE_MAX  2048
#define PRINTKRB_RECORD_MAX 1024

typedef struct console_buffers
{
    char outbuf[PRINTK_MESSAGE_MAX];
    char scratchbuf[PRINTKRB_RECORD_MAX];
} console_buffers_t;

typedef enum con_write_level
{
    CON_WRITE_LEVEL_ENTER_FUNC = 0,
    CON_WRITE_LEVEL_DEBUG,
    CON_WRITE_LEVEL_NORMAL,
    CON_WRITE_LEVEL_EMERGENCY,
    CON_WRITE_LEVEL_PANIC,
} con_write_level_t;

typedef struct console_message
{
    console_buffers_t *console_bufs;
    uint32_t outbuf_len;
    uint64_t seq;
    uint32_64_t dropped;
    cpu_logical_id_t cpu_id;
    pid_t pid;
    trace_level_t log_level;
    con_write_level_t level;
} console_message_t;

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

typedef enum con_scroll
{
    SM_UP,
    SM_DOWN,
} con_scroll_t;

typedef struct console console_t;

typedef struct console
{
    LIST_ENTRY_THIS();
    char name[16];

    status_t (*write)(console_t *co, console_message_t *console_msg, const char *s, size_t count);

    // status_t (*read)(console_t *co, char *s, size_t count);
    // status_t (*read_poll)(console_t *co, char *s, size_t count);

    status_t (*device)(console_t *co, int *index, tty_driver_t **tty_out);
    status_t (*unblank)();
    status_t (*setup)(console_t *co, char *options);
    status_t (*exit)(console_t *co);
    status_t (*match)(console_t *co, char *name, int idx, char *options);
    status_t (*device_try_lock)(console_t *con);
    status_t (*device_lock)(console_t *con);
    status_t (*device_unlock)(console_t *con);
    con_write_level_t console_loglevel;
    cons_flags_t flags;
    short index;
    int cflag;
    uint32_t ispeed;
    uint32_t ospeed;
    uint64_t seq;
    unsigned long dropped;
    void *data;
} console_t;

typedef struct console_write_ctx
{
    console_t *console;

    const char *message;
    size_t message_length;

#ifdef DEBUG
    const char *function;
    const char *file;
    uint32_t line;
#endif

    uint64_t seq;

    timespec_t time;

    cpu_logical_id_t cpu_id;
    tid_t thread_id;
    pid_t process_id;
    trace_level_t log_level;
    con_write_level_t level;
} console_write_ctx_t;

typedef struct console_cmdline
{
    char name[16];       /* Name of the driver	    */
    int index;           /* Minor dev. to use	    */
    char devname[32];    /* DEVNAME:0.0 style device name */
    bool user_specified; /* Specified by command line vs. platform */
    char *options;       /* Options for the driver   */
} console_cmdline_t;

#define to_console(n) container_of(n, console_t, LIST_ENTRY_NAME)
