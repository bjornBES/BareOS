/*
 * File: signal.c
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/syscall.h"
#include "task/process.h"
#include "signal_type.h"

#include <stdbool.h>

static inline bool is_signal_valid()
{
    return 0;
}

void signal_try_deliver(thread_t *t, syscall_info *info, intr_frame_t *regs);

int signal_send(thread_t *t, int signum);
int signal_send_group(process_t *proc, int signum);

void signal_mask(thread_t *t, int signum);
void signal_unmask(thread_t *t, int signum);
bool signal_is_masked(thread_t *t, int signum);
bool signal_is_pending(thread_t *t, int signum);

int signal_set_action(thread_t *t, int signum, signal_action_t *sa);
int signal_get_action(thread_t *t, int signum, signal_action_t *out);

void signal_default_action(thread_t *t, int signum);

void signal_init();
