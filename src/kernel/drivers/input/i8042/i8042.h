/*
 * File: i8042.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/devicet.h"

#include <types.h>
#include <binary.h>

#define I8042_DATA_REG		0x60
#define I8042_STATUS_REG	0x64
#define I8042_COMMAND_REG	0x64


#define I8042_STAT_OUTPUT_FULL 1
#define I8042_STAT_OUTPUT_EMPTY 0

#define I8042_STAT_INPUT_FULL 1
#define I8042_STAT_INPUT_EMPTY 0

#define I8042_STAT_OUTPUT_BUFF_STATE BIT(0)
#define I8042_STAT_INPUT_BUFF_STATE BIT(1)

// PS/2 Controller Configuration Byte

// First PS/2 port interrupt (1 = enabled, 0 = disabled)
#define I8042_CCB_FIRST_PORT_INT BIT(0)

// Second PS/2 port interrupt (1 = enabled, 0 = disabled)
#define I8042_CCB_SECOND_PORT_INT BIT(1)

// System Flag (1 = system passed POST, 0 = how did we get here?) 
#define I8042_CCB_SYSTEM_FLAG BIT(2)

// First PS/2 port clock (1 = disabled, 0 = enabled)
#define I8042_CCB_FIRST_PORT_CLOCK BIT(4)

// Second PS/2 port clock (1 = disabled, 0 = enabled)
#define I8042_CCB_SECOND_PORT_CLOCK BIT(5)

// First PS/2 port translation (1 = enabled, 0 = disabled)
#define I8042_CCB_FIRST_PORT_TRANSLATION BIT(6)

// PS/2 Controller Output Port

// System reset (output)
#define I8042_COP_SYS_RESET BIT(0)

typedef struct i8042_port {
	// struct serio *serio;
    device_t *dev;
	kernel_irq_t irq;
	bool exists;
	bool driver_bound;
	signed char mux;
} i8042_port_t;

bool i8042_check_status(uint8_t bit, uint16_t ticks, uint8_t value);
void i8042_wait_output_buffer_fill();
void i8042_wait_input_buffer_empty();
void i8042_write(uint8_t port, uint8_t data);
uint8_t i8042_read(uint8_t port);
void i8042_flush_output_buffer(size_t timeout);
void i8042_write_cmd(uint8_t cmd);
void i8042_write_controller_conf(uint8_t conf);
uint8_t i8042_read_controller_conf();

void i8042_write_first_channel(uint8_t data);
status_t i8042_first_channel_wait_for_ack();
bool i8042_first_channel_wait_for_response();
status_t i8042_first_channel_wait_for_reset();

void i8042_write_second_channel(uint8_t data);
status_t i8042_second_channel_wait_for_ack();
bool i8042_second_channel_wait_for_response();

// ps2kbd.c

status_t i8042_kbd_probe(i8042_port_t *port);