/*
 * File: signal_type.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/signal/signal_types.h"
#include "kernel/signal/signal_config.h"
#include "kernel/syscall.h"
#include "kernel.h"
#include <signals.h>

typedef struct thread thread_t;

typedef struct
{
    uint32_t signal_number;
    pid_t proc_id;
} signal_info;

typedef struct
{
    sigset_t signal;
} signal_pending;

typedef void(*signal_restore)();
typedef void (*signal_kernel_handler)(thread_t *t, int signal_number);
typedef struct
{
    signal_kernel_handler handler;
    signal_action_t action;
} kernel_signal_action;

typedef struct __signal_table_t {
    signal_action_t actions[NUMBER_SIGNAL];
} signal_table;