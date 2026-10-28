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

#include "drivers/input/input_codes.h"

#include <types.h>

extern tty_struct_t *active_tty;

status_t tty_receive_key(tty_struct_t *tty, key_event_t *ev, device_id_t dev_id);
status_t tty_receive(tty_struct_t *tty, wchar_t ch);