/*
 * File: signal.h
 * File Created: 31 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "signal_types.h"
#include "kernel/asm/signal/signal_types.h"
#include "kernel/asm/signal/signal_config.h"
#include "signal/signal_type.h"
#include "kernel/syscall.h"
#include "kernel/ctx.h"

typedef struct signal_arch_frame signal_arch_frame_t;

// full setup — does everything needed to redirect execution to handler:
void signal_arch_setup_frame(thread_t *t, intr_frame_t *frame, signal_info *info, signal_action_t *sa);

// restores frame saved by signal_arch_setup_frame
// called by signal_return() after sys_sigreturn comes in
void signal_arch_restore_frame(intr_frame_t *frame);

// the actual iretq dispatch — jumps to handler
// called at the end of signal_arch_setup_frame once frame is ready
void signal_arch_dispatch(intr_frame_t *reg_frame);

// vdso trampoline setup — called once at process creation
// plants the sigreturn stub in the vdso page
void signal_arch_setup_vdso();
