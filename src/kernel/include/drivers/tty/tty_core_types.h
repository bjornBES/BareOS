/*
 * File: tty_core_types.h
 * File Created: 24 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "termios.h"
#include "tty_ldisct.h"

#include "drivers/base/drivert.h"

#include "dev/devicet.h"

#include "ringbuf/ringbuft.h"

#include <types.h>

typedef struct winsize
{
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
} winsize_t;

typedef enum tty_struct_flags
{
    // TTY_THROTTLED,
    // TTY_IO_ERROR,
    // TTY_OTHER_CLOSED,
    // TTY_EXCLUSIVE,
    // TTY_DO_WRITE_WAKEUP,
    TTY_LDISC_OPEN,
    // TTY_PTY_LOCK,
    // TTY_NO_WRITE_SPLIT,
    // TTY_HUPPED,
    // TTY_HUPPING,
    TTY_LDISC_CHANGING,
    TTY_LDISC_HALTED,
} tty_struct_flags_t;

struct tty_struct;
struct tty_driver;

typedef struct tty_ops
{
    status_t (*install)(struct tty_driver *driver, struct tty_struct *tty);
    status_t (*open)(struct tty_struct *tty, vfs_node_t *out_file);
    status_t (*close)(struct tty_struct *tty, vfs_node_t *file);
    status_t (*write)(struct tty_struct *tty, const uint8_t *buf, size_t count, ssize_t *size_out);
    status_t (*put_char)(struct tty_struct *tty, uint8_t ch);
    status_t (*flush_chars)(struct tty_struct *tty);
    status_t (*start)(struct tty_struct *tty);
    status_t (*write_room)(struct tty_struct *tty, size_t *size_out);
} tty_ops_t;

typedef struct tty_struct
{
    int id;
    int index;

    char name[64];
    uint32_t flags;

    struct tty_dev *tty_dev;
    struct tty_driver *driver;
    const tty_ops_t *ops;

    tty_ldisc_t *ldisc;
    void *disc_data;

    // input side
    // tty_line_buf_t line_buf; // canonical line discipline buffer

    // behaviour
    termios_t termios; // current terminal settings

    winsize_t winsize;

    struct
    {
        spinlock_t lock;
        bool stopped;
        bool tco_stopped;
    } flow;

    struct tty_struct *link;

    // ops
    // tty_dev_ops_t in_ops;
    // tty_dev_ops_t out_ops;


    // blocking read support
    // block_queue_t read_wait; // threads blocked in read() sleep here

    // process group (for signal delivery)
    pid_t fg_pgid; // foreground process group ID
} tty_struct_t;

typedef enum tty_driver_flag
{
    TTY_DRIVER_INSTALLED = BIT(0),
    TTY_DRIVER_RESET_TERMIOS = BIT(1),
    TTY_DRIVER_REAL_RAW = BIT(2),
    TTY_DRIVER_DYNAMIC_DEV = BIT(3),
    TTY_DRIVER_DEVPTS_MEM = BIT(4),
    TTY_DRIVER_HARDWARE_BREAK = BIT(5),
    TTY_DRIVER_DYNAMIC_ALLOC = BIT(6),
    TTY_DRIVER_UNNUMBERED_NODE = BIT(7),
    TTY_DRIVER_NO_WORKQUEUE = BIT(8),
} tty_driver_flag_t;

typedef enum tty_driver_type
{
    TTY_DRIVER_TYPE_SYSTEM,
    TTY_DRIVER_TYPE_CONSOLE,
    TTY_DRIVER_TYPE_SERIAL,
    TTY_DRIVER_TYPE_PTY,
    TTY_DRIVER_TYPE_SCC,
    TTY_DRIVER_TYPE_SYSCONS,
} tty_driver_type_t;

typedef enum tty_driver_subtype
{
    SYSTEM_TYPE_TTY = 1,
    SYSTEM_TYPE_CONSOLE,
    SYSTEM_TYPE_SYSCONS,
    SYSTEM_TYPE_SYSPTMX,

    PTY_TYPE_MASTER = 1,
    PTY_TYPE_SLAVE,

    SERIAL_TYPE_NORMAL = 1,
} tty_driver_subtype_t;


typedef struct tty_driver
{
    LIST_ENTRY_THIS();
    driver_t driver;
    const char *driver_name;
    const char *name;
    int name_base;
    int device;
    int major;
    int minor_start;

    uint32_t num;
    tty_driver_type_t type;
    tty_driver_subtype_t subtype;
    termios_t init_termios;
    uint32_64_t flags;

    tty_struct_t **ttys;
    termios_t **termios;
    void *driver_state;

    const tty_ops_t *ops;
} tty_driver_t;

typedef struct tty_dev
{
    void *driver_data;
    
    tty_struct_t *tty;
    tty_driver_t *driver;
    device_t dev;
} tty_dev_t;

#define to_tty_dev(n) container_of(n, tty_dev_t, dev)