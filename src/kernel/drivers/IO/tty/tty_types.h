/*
 * File: tty_type.h
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <types.h>

#include "tty_config.h"
#include "tty_internal_types.h"

#include "device/device_types.h"

typedef uint8_t cc_t;
typedef uint32_t speed_t;
typedef uint32_t tcflag_t;

typedef struct tty_dev_ops
{
    void (*write_char)(device_t *dev, char c);
    void (*write_buf)(device_t *dev, const uint8_t *buf, size_t n);
    void (*flush_output)(device_t *dev);
    ringbuf_index (*get_input_buf)(device_t *dev);
    void (*flush_input)(device_t *dev);
} tty_dev_ops_t;

typedef struct
{
    tcflag_t c_iflag; // input flags  (ICRNL, IXON etc.)
    tcflag_t c_oflag; // output flags (ONLCR etc.)
    tcflag_t c_cflag; // control flags (baud rate, stop bits - mostly serial)
    tcflag_t c_lflag; // local flags  (ICANON, ECHO, ISIG etc.)
    cc_t c_cc[NCCS];  // special characters (VINTR, VEOF, VERASE etc.)
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
} termios_t;

#define TTY_BUF_SIZE 256

typedef struct
{
    uint8_t buf[TTY_BUF_SIZE];
    size_t head;   // write position
    size_t tail;   // read position
    bool has_line; // a complete line is ready (\n was received)
} tty_line_buf_t;

typedef struct tty_struct
{
    int id;

    // input side
    tty_line_buf_t line_buf; // canonical line discipline buffer

    // behaviour
    termios_t termios; // current terminal settings

    // output device
    device_t *output_dev;

    // input device
    device_t *input_dev;

    // ops
    tty_dev_ops_t in_ops;
    tty_dev_ops_t out_ops;

    // blocking read support
    // wait_queue_t read_wait; // threads blocked in read() sleep here

    // process group (for signal delivery)
    // pid_t fg_pgid; // foreground process group ID
} tty_struct_t;
