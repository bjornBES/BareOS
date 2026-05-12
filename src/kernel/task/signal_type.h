/*
 * File: signal_type.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/signal/signal.h"
#include "kernel/syscall.h"
#include "kernel/registers.h"
#include "kernel.h"
#include <signals.h>

typedef struct
{
    uint32_t signal_number;
    pid_t proc_id;
} signal_info;

typedef struct __signal_frame_t
{
    registers proc_reg;
    signal_info signal_info;
} signal_frame;

typedef struct
{
    sigset signal;
} signal_pending;

typedef void(*signal_restore)();
typedef void (*signal_kernel_handler)(int signal_number, syscall_info *arch_info, signal_info *info);
typedef struct
{
    signal_kernel_handler handler;
    signal_action action;
} kernel_signal_action;

typedef struct __signal_table_t {
    signal_action actions[NUMBER_SIGNAL];
} signal_table;