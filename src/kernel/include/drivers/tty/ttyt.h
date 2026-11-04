/*
 * File: ttyt.h
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "termbits.h"

#include "dev/devicet.h"

#include "ringbuf/ringbuft.h"

#include <types.h>

typedef uint8_t cc_t;
typedef uint32_t speed_t;
typedef uint32_t tcflag_t;

// typedef struct tty_dev_ops
// {
//     void (*write_char)(device_t *dev, char c);
//     void (*write_buf)(device_t *dev, const uint8_t *buf, size_t n);
//     void (*flush_output)(device_t *dev);
//     ringbuf_index (*get_input_buf)(device_t *dev);
//     void (*flush_input)(device_t *dev);
// } tty_dev_ops_t;

typedef struct termios
{
    tcflag_t c_iflag; // input flags  (ICRNL, IXON etc.)
    tcflag_t c_oflag; // output flags (ONLCR etc.)
    tcflag_t c_cflag; // control flags (baud rate, stop bits - mostly serial)
    tcflag_t c_lflag; // local flags  (ICANON, ECHO, ISIG etc.)
    cc_t c_cc[NCCS];  // special characters (VINTR, VEOF, VERASE etc.)
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
} termios_t;

typedef struct winsize
{
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
} winsize_t;

#define TTY_BUF_SIZE 256

typedef struct
{
    uint8_t buf[TTY_BUF_SIZE];
    size_t head;   // write position
    size_t tail;   // read position
    bool has_line; // a complete line is ready (\n was received)
} tty_line_buf_t;

typedef struct tty_ops tty_ops_t;
typedef struct tty_driver tty_driver_t;
typedef struct tty_dev tty_dev_t;
typedef struct tty_ldisc tty_ldisc_t;

typedef struct tty_struct
{
    int id;
    int index;

    char name[64];

    tty_dev_t *tty_dev;
    tty_driver_t *driver;
    const tty_ops_t *ops;

    tty_ldisc_t *ldisc;

    // input side
    tty_line_buf_t line_buf; // canonical line discipline buffer

    // behaviour
    termios_t termios; // current terminal settings

    ringbuf_t *char_buffer;

    // ops
    // tty_dev_ops_t in_ops;
    // tty_dev_ops_t out_ops;

    winsize_t winsize;

    // blocking read support
    // block_queue_t read_wait; // threads blocked in read() sleep here

    // process group (for signal delivery)
    pid_t fg_pgid; // foreground process group ID
} tty_struct_t;

typedef struct tty_dev
{
    device_t dev;

    void *driver_data;

    tty_struct_t *tty;
    tty_driver_t *driver;
} tty_dev_t;

#define to_tty_dev(n) container_of(n, tty_dev_t, dev)

typedef struct tty_action
{
    // TODO:
} tty_action_t;

typedef struct tty_ops
{
    status_t (*get_index)(tty_driver_t *driver, int *out_index);
    status_t (*install)(tty_driver_t *driver, tty_struct_t *tty);
    status_t (*open)(tty_struct_t *tty, vfs_node_t *filp);
    status_t (*put_char)(tty_struct_t *tty, wchar_t ch);
    status_t (*write)(tty_struct_t *tty, const wchar_t *buf, size_t count, ssize_t *size_out);
} tty_ops_t;

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

    const tty_ops_t *ops;
} tty_driver_t;
