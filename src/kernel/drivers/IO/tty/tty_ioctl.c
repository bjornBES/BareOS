/*
 * File: tty_ioctl.c
 * File Created: 10 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty.h"
#include "tty_ioctl.h"
#include "tty_types.h"
#include "tty_flags.h"

#include "kernel/memory.h"

#include <util/arrays.h>

#include "errno/errno.h"

#define MODULE "TTY-IOCTL"

