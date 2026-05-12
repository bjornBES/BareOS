/*
 * File: signal.c
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 May 2026 20:38:57
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/syscall.h"
#include "process.h"
#include "signal_type.h"

#include <stdbool.h>

static inline bool is_signal_valid()
{
    return 0;
}

void signal_enqueue(process_t *proc, int signum);
void signal_try_call(syscall_info *info);
void signal_init();
