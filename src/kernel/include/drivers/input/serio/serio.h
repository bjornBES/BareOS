/*
 * File: serio.h
 * File Created: 26 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "seriot.h"

#include "asm/serio_arch.h"

#include <types.h>

status_t serio_register_port(serio_dev_t *serio);
