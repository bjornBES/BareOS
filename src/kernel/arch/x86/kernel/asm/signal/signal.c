/*
 * File: signal.c
 * File Created: 01 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "signal.h"
#include "task/threading/thread_type.h"
#include "kernel/x86.h"
#include "kernel/ivt.h"
#include "kernel/memory.h"
#include "kernel/signal/signal.h"
#include "kernel/segments.h"
#include "mm/memdefs.h"

#include <util/binary.h>
#include <defs.h>

#define MODULE "X86-SIGNAL"

/* void signal_arch_return(syscall_info *info)
{
    intr_frame_t *regs = info->regs;
    signal_frame *frame = (signal_frame *)regs->sp;
    memcpy(regs, &frame->proc_reg, sizeof(intr_frame_t));
} */

void signal_arch_setup_frame(thread_t *t, intr_frame_t *frame, signal_info *info, signal_action_t *sa)
{
    // TODO add SA_ONSTACK in year 2030
    
    vaddr_t user_stack = frame->sp;
    user_stack -= sizeof(signal_arch_frame_t);
    user_stack &= ~0xFull;

    signal_arch_frame_t *sf = (signal_arch_frame_t *)user_stack;
    sf->saved_mask = t->signal_mask;
    
    log_debug(MODULE, "sf @ %p, out_frame @ %p", &sf->proc_reg, &frame);
    sf->proc_reg = *frame;
    
    log_debug(MODULE, "frame @ %p", frame);
    user_stack -= sizeof(vaddr_t);
    *(vaddr_t *)user_stack = USER_VDSO_VIRT;

    frame->pc = (vaddr_t)sa->handler;
    frame->sp = user_stack;
    frame->di = info->signal_number;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
}

void signal_arch_restore_frame(intr_frame_t *frame)
{
    signal_arch_frame_t *sig_frame = (signal_arch_frame_t *)frame->sp;
    memcpy(frame, &sig_frame->proc_reg, sizeof(intr_frame_t));
    ctx_arch_switch((vaddr_t)frame);
}

void signal_arch_dispatch(intr_frame_t *frame)
{
    ctx_arch_switch((vaddr_t)frame);
}

void signal_arch_setup_vdso()
{
}
