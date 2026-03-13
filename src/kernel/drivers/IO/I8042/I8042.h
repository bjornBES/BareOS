/*
 * File: I8042.h
 * File Created: 12 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
#define I8042_COMMAND_PORT 0x64

#define I8042_CHANNEL_NO_DEVICE_CONNECTED 0
#define I8042_CHANNEL_KEYBOARD_CONNECTED 0xAB
#define I8042_CHANNEL_KEYBOARD_INITALIZED 0xAC
#define I8042_CHANNEL_MOUSE_CONNECTED 0x03
#define I8042_CHANNEL_MOUSE_INITALIZED 0x04

#define I8042_DEVICE_PRESENCE_IS_NOT_KNOWN 0xFF
#define I8042_DEVICE_NOT_PRESENT 0
#define I8042_DEVICE_PRESENT 1
#define I8042_DEVICE_PRESENT_BUT_ERROR_STATE 2

extern uint8_t I8042_first_channel_present;
extern uint8_t I8042_second_channel_present;
extern uint8_t I8042_first_channel_device;
extern uint8_t I8042_second_channel_device;

extern uint8_t I8042_first_channel_buffer[10];
extern uint8_t I8042_second_channel_buffer[10];
extern uint8_t I8042_first_channel_buffer_pointer;
extern uint8_t I8042_second_channel_buffer_pointer;

void I8042_init();
