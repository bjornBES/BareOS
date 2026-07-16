/*
 * File: I8042_keyboard.h
 * File Created: 21 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "device/device.h"

#define I8042_CHANNEL_KEYBOARD_CONNECTED 0xAB
#define I8042_CHANNEL_KEYBOARD_INITALIZED 0xAC

device_t *I8042_keyboard_device();
void I8042_first_channel_handler(device_t *dev);
    