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

#include "vfs/vfs_internals.h"

#include <types.h>

/* line disciplines */
#define N_TTY         0
#define N_DEVELOPMENT 1
#define NR_LDISCS     2

struct tty_struct;

typedef struct tty_ldisc_ops
{
    char *name;
    int num;

    /// @brief
    status_t (*open)(struct tty_struct *tty);

    /// @brief
    status_t (*close)(struct tty_struct *tty);

    /// @brief
    status_t (*flush_buffer)(struct tty_struct *tty);

    /// @brief
    status_t (*receive)(struct tty_struct *tty, wchar_t ch);

    /// @brief
    status_t (*read)(struct tty_struct *tty, vfs_node_t *file, uint8_t *buf, size_t count, ssize_t *out_size);

    /// @brief
    status_t (*write)(struct tty_struct *tty, vfs_node_t *file, const uint8_t *buf, size_t count, ssize_t *out_size);

    /// @brief
    status_t (*write_wakeup)(struct tty_struct *tty);

    /// @brief
    status_t (*set_termios)(struct tty_struct *tty, const termios_t *old);

    /// @brief
    status_t (*flush)(struct tty_struct *tty);
    
    /// @brief
    status_t (*hangup)(struct tty_struct *tty);
    
    /// @brief
    status_t (*poll)(struct tty_struct *tty, vfs_node_t *file, uint32_t *mask_out);

    /*
     * The following routines are called from below.
     */
    // /// @brief
    // status_t (*write_wakeup)(struct tty_struct *tty);

    // /// @brief
    // status_t (*dcd_change)(struct tty_struct *tty, bool active);

    // /// @brief
    // status_t (*receive_buf)(struct tty_struct *tty, const char *cp, const char *fp, size_t count, size_t *out_size);

    // /// @brief
    // status_t (*lookahead_buf)(struct tty_struct *tty, const char *cp, const char *fp, size_t count);
} tty_ldisc_ops_t;

typedef struct tty_ldisc
{
    const tty_ldisc_ops_t *ops;
    struct tty_struct *tty;
} tty_ldisc_t;
