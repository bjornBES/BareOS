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

#include "siginfo.h"
#include "kernel/signal/signal_types.h"
#include "kernel/signal/signal_config.h"
#include "kernel/syscall.h"
#include "kernel.h"
#include <signals.h>
#include <types.h>

#define SIGMASK(sig) (1ul << ((sig) - 1))

typedef struct thread thread_t;

typedef struct
{
    sigset_t signal;
} signal_pending;

typedef void (*signal_restore)();
typedef void (*signal_kernel_handler)(thread_t *t, int signal_number, siginfo_t *info, void *ucontext);
typedef void (*signal_handler)(int signal_number);

typedef struct sigaction
{
    union
    {
        signal_handler sa_handler;
        void (*sa_sigaction)(int, siginfo_t *, void *);
        signal_kernel_handler default_handler;
    } handler;

    sigset_t sa_mask;
    uint64_t sa_flags;
    signal_restore sa_restorer;
} sigaction_t;

typedef struct
{
    signal_kernel_handler handler;
    sigaction_t action;
} kernel_signal_action;

typedef struct __signal_table_t
{
    sigaction_t actions[NUMBER_SIGNAL];
} signal_table;
