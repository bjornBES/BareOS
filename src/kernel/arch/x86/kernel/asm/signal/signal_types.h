/*
 * File: signal_types.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "kernel/asm/exceptions/isr.h"
#include <siginfo.h>
#include "sigcontext.h"

typedef struct sigaltstack {
	void *ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;

typedef struct ucontext
{
    uint64_t uc_flags;
    struct ucontext *uc_link; // usually NULL
    stack_t uc_stack;         // alt signal stack info (sigaltstack)
    sigcontext_t uc_mcontext;   // THE important part — full saved register state
    sigset_t uc_sigmask;      // signal mask to restore on return
} ucontext_t;

typedef struct signal_arch_frame
{
    void *pretcode;      // return address: points to the restorer trampoline
    ucontext_t uc;  // saved machine state + signal mask, for sigreturn
    siginfo_t info; // the siginfo_t you just built
} signal_arch_frame_t;
