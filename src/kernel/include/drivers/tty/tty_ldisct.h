/*
 * File: tty_ldisct.h
 * File Created: 03 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ttyt.h"

#include "vfs/vfs_internals.h"

#include <types.h>

/* line disciplines */
#define N_TTY         0
#define N_DEVELOPMENT 1
#define NR_LDISCS     2

typedef struct tty_ldisc_ops
{
    char *name;
    int num;

    status_t (*open)(tty_struct_t *tty);
    status_t (*close)(tty_struct_t *tty);
    status_t (*flush_buffer)(tty_struct_t *tty);
    ssize_t (*read)(tty_struct_t *tty, vfs_node_t *file, wchar_t *buf, size_t nr, void **cookie, off_t offset, ssize_t *out_size);
    ssize_t (*write)(tty_struct_t *tty, vfs_node_t *file, const wchar_t *buf, size_t nr, ssize_t *out_size);
    status_t (*set_termios)(tty_struct_t *tty, const termios_t *old);
    // __poll_t (*poll)(tty_struct_t *tty, vfs_node_t *file, struct poll_table_struct *wait);
    status_t (*hangup)(tty_struct_t *tty);

    /*
     * The following routines are called from below.
     */
    status_t (*write_wakeup)(tty_struct_t *tty);
    status_t (*dcd_change)(tty_struct_t *tty, bool active);
    status_t (*receive_buf)(tty_struct_t *tty, const wchar_t *cp, const wchar_t *fp, size_t count, size_t *out_size);
    status_t (*lookahead_buf)(tty_struct_t *tty, const wchar_t *cp, const wchar_t *fp, size_t count);
} tty_ldisc_ops_t;

typedef struct tty_ldisc
{
    const tty_ldisc_ops_t *ops;
    tty_struct_t *tty;
} tty_ldisc_t;
