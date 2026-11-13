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

    /// @brief
    status_t (*open)(tty_struct_t *tty);

    /// @brief
    status_t (*close)(tty_struct_t *tty);

    /// @brief
    status_t (*flush_buffer)(tty_struct_t *tty);

    /// @brief
    status_t (*receive)(tty_struct_t *tty, wchar_t ch);

    /// @brief
    status_t (*read)(tty_struct_t *tty, vfs_node_t *file, uint8_t *buf, size_t count, ssize_t *out_size);

    /// @brief
    status_t (*write)(tty_struct_t *tty, vfs_node_t *file, const uint8_t *buf, size_t count, ssize_t *out_size);

    /// @brief
    status_t (*write_wakeup)(tty_struct_t *tty);

    /// @brief
    status_t (*set_termios)(tty_struct_t *tty, const termios_t *old);

    /// @brief
    status_t (*flush)(tty_struct_t *tty);
    
    /// @brief
    status_t (*hangup)(tty_struct_t *tty);
    
    /// @brief
    status_t (*poll)(tty_struct_t *tty, vfs_node_t *file, uint32_t *mask_out);

    /*
     * The following routines are called from below.
     */
    // /// @brief
    // status_t (*write_wakeup)(tty_struct_t *tty);

    // /// @brief
    // status_t (*dcd_change)(tty_struct_t *tty, bool active);

    // /// @brief
    // status_t (*receive_buf)(tty_struct_t *tty, const char *cp, const char *fp, size_t count, size_t *out_size);

    // /// @brief
    // status_t (*lookahead_buf)(tty_struct_t *tty, const char *cp, const char *fp, size_t count);
} tty_ldisc_ops_t;

typedef struct tty_ldisc
{
    const tty_ldisc_ops_t *ops;
    tty_struct_t *tty;
} tty_ldisc_t;
