/*
 * File: tty.h
 * File Created: 15 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "tty_types.h"
#include "device/device.h"
#include "ringbuf/ringbuf.h"

// tty.c

// allocate and initialise a tty, hook it to a gki ring buffer
tty_struct_t *tty_create(device_t *input_dev, device_t *output_dev);

// tear down
void tty_destroy(tty_struct_t *tty);

// drain the gki ring buffer and process all pending events
// called from tty_read() before blocking, or from a deferred worker
void tty_drain(tty_struct_t *tty);

// called from your read() syscall handler
// blocks until a complete line is available (in canonical mode)
ssize_t tty_read(tty_struct_t *tty, uint8_t *buf, size_t n);

// called from your write() syscall handler
// writes buf to the output side, handling \n -> \r\n if ONLCR set
ssize_t tty_write(tty_struct_t *tty, const uint8_t *buf, size_t n);

// called from your ioctl() syscall handler
int tty_ioctl(tty_struct_t *tty, uint32_t cmd, void *arg);

// tty_baudrate.c

speed_t tty_baudrate_baud_rate(const termios_t *termios);
void tty_baudrate_encode_baud_rate(termios_t *termios, speed_t ibaud, speed_t obaud);

// tty_ioctl.c

int tty_ioctl_mode(tty_struct_t *tty, uint32_t cmd, void *arg);

// tty_io.c

void tty_io_drain_output(tty_struct_t *tty);

void tty_io_linebuf_flush(tty_struct_t *tty);

// tty_termios.c

int tty_termios_set(tty_struct_t *tty, termios_t *term);
void tty_termios_init(tty_struct_t *tty);
void tty_termios_save(tty_struct_t *tty);
