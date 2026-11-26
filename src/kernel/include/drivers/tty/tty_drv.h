/*
 * File: tty_drv.h
 * File Created: 16 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "termios.h"
#include "tty_core_types.h"

#include "drivers/base/drivert.h"

#include "lists/list_node.h"

#include "vfs/vfs_internals.h"

#include <binary.h>

static inline void tty_set_operations(tty_driver_t *driver, const tty_ops_t *op)
{
	driver->ops = op;
}