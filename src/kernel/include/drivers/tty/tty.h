/*
 * File: tty.h
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ttyt.h"
#include "tty_ldisct.h"

#include "vfs/vfs_internals.h"

#include "drivers/input/input_codes.h"

#include <types.h>

#define tty_msg(fn, tty, f, ...) \
	fn(MODULE, "%s %s: " f, tty_driver_name(tty), tty_name(tty), ##__VA_ARGS__)

#define tty_debug(tty, f, ...)	tty_msg(trace_debug, tty, f, ##__VA_ARGS__)
#define tty_notice(tty, f, ...)	tty_msg(trace_info, tty, f, ##__VA_ARGS__)
#define tty_warn(tty, f, ...)	tty_msg(trace_warn, tty, f, ##__VA_ARGS__)
#define tty_err(tty, f, ...)	tty_msg(trace_err, tty, f, ##__VA_ARGS__)

extern tty_struct_t *active_tty;
extern device_t *console_dev;

extern termios_t tty_std_termios;

const char *tty_name(const tty_struct_t *tty);
const char *tty_driver_name(const tty_struct_t *tty);

void start_tty(tty_struct_t *tty);

status_t tty_receive_key(tty_struct_t *tty, key_event_t *ev, device_id_t dev_id);
status_t tty_receive(tty_struct_t *tty, wchar_t ch);

status_t tty_do_open(tty_struct_t *tty, vfs_node_t *out_file);
status_t tty_open(inode_t *inode, vfs_node_t *out_file);

status_t tty_read(tty_struct_t *tty, uint8_t *ch, size_t count, ssize_t *out_size);
status_t tty_write(tty_struct_t *tty, const uint8_t *ch, size_t count, ssize_t *out_size);
status_t tty_put_char(tty_struct_t *tty, uint8_t ch);

status_t tty_write_room(tty_struct_t *tty, size_t *out_size);

status_t tty_do_resize(tty_struct_t *tty, winsize_t *ws);

status_t tty_register_driver(tty_driver_t *driver, tty_dev_t **out_tty_dev);

status_t tty_alloc_driver(unsigned int lines, unsigned long flags, tty_driver_t **out);

// tty_ldisc.c

/// @brief 
/// @param new_ldisc 
/// @return 
status_t tty_register_ldisc(const tty_ldisc_ops_t *new_ldisc);

/// @brief 
/// @param tty 
/// @return 
status_t tty_ldisc_init(tty_struct_t *tty);

/// @brief 
/// @param tty 
/// @param o_tty 
/// @return 
status_t tty_ldisc_setup(tty_struct_t *tty, tty_struct_t *o_tty);

// n_tty.c

/// @brief 
/// @return 
status_t n_tty_init();
