/*
 * File: signal.h
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <siginfo.h>
#include <signals.h>

typedef void (*signal_handler)(int signal_number);
typedef void (*signal_restore)();
typedef struct sigaction
{
    union
    {
        signal_handler sa_handler;
        void (*sa_sigaction)(int, siginfo_t *, void *);
    } handler;
    sigset_t sa_mask;
    uint64_t sa_flags;
    signal_restore sa_restorer;
} sigaction_t;

int rt_sig_action(int signum, sigaction_t *action, sigaction_t *old_action);
int kill(pid_t pid, int sig);
