/*
 * File: process.c
 * File Created: 26 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/process.h"
#include "kernel/asm/segment/segments.h"
#include "kernel/x86.h"

void process_arch_build_return_frame(intr_frame_t *frame, process_t *proc)
{
    frame->pc = proc->entry;
    frame->sp = proc->vma->start_stack;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
}
