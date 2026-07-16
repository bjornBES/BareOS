/*
 * File: ctx.c
 * File Created: 01 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ctx.h"
#include "kernel/x86.h"
#include "kernel/ctx.h"
#include "kernel/segments.h"
#include "kernel/memory.h"
#include <types.h>

#define MODULE "X86-CTX"

vaddr_t ctx_arch_init(cpu_ctx_t *ctx, vaddr_t entry, vaddr_t kstack, vaddr_t ustack, vaddr_t arg)
{
    // carve a fake frame at top of kernel stack
    kstack -= sizeof(intr_frame_t);
    intr_frame_t *frame = (intr_frame_t *)kstack;
    memset(frame, 0, sizeof(intr_frame_t));

    frame->pc = entry;
    if (ustack == 0)
    {
        frame->sp = kstack;
        frame->cs = KERNEL_CODE_SELECTOR;
        frame->ss = KERNEL_DATA_SELECTOR;
        frame->ds = KERNEL_DATA_SELECTOR;
    }
    else
    {
        frame->sp = ustack;
        frame->cs = USER_CODE_SELECTOR | 3;
        frame->ss = USER_DATA_SELECTOR | 3;
        frame->ds = USER_DATA_SELECTOR | 3;
    }
    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;
    frame->di = arg;

    // point ctx at the fake frame
    ctx->frame.sp = kstack;
    ctx->frame.regs = frame;
    return kstack;
}

void ctx_dump(cpu_ctx_t *ctx)
{
    intr_frame_t *frame = ctx->frame.regs;
    fprintf(VFS_FD_DEBUG, "\t{ frame @ %p }\n", frame);
    fprintf(VFS_FD_DEBUG, "\t{ ax = 0x%lx, bx = 0x%lx, cx = 0x%lx, dx = 0x%lx }\n", frame->ax, frame->bx, frame->cx, frame->dx);
    fprintf(VFS_FD_DEBUG, "\t{ di = 0x%lx, si = 0x%lx, r8 = 0x%lx, r9 = 0x%lx }\n", frame->di, frame->si, frame->r8, frame->r9);
    fprintf(VFS_FD_DEBUG, "\t{ r10 = 0x%lx, r11 = 0x%lx, r12 = 0x%lx, r13 = 0x%lx }\n", frame->r10, frame->r11, frame->r12, frame->r13);
    fprintf(VFS_FD_DEBUG, "\t{ r14 = 0x%lx, r15 = 0x%lx, bp = 0x%lx, flags = 0x%lx }\n", frame->r14, frame->r15, frame->bp, frame->flags);
    fprintf(VFS_FD_DEBUG, "\t{ pc = 0x%x:%p, sp = 0x%x:%p }\n", frame->cs, frame->pc, frame->ss, frame->sp);
}

vaddr_t ctx_arch_get_ip(cpu_ctx_t *ctx)
{
    return ctx->frame.regs->pc;
}

void ctx_arch_set_ip(cpu_ctx_t *ctx, vaddr_t ip)
{
    ctx->frame.regs->pc = ip;
}

vaddr_t ctx_arch_get_sp(cpu_ctx_t *ctx)
{
    return ctx->frame.regs->sp;
}

void ctx_arch_set_sp(cpu_ctx_t *ctx, vaddr_t sp)
{
    ctx->frame.regs->sp = sp;
}

void ctx_arch_set_retval(cpu_ctx_t *ctx, uint64_t val)
{
    ctx->frame.regs->ax = val;
}

vaddr_t frame_arch_get_ip(intr_frame_t *ctx)
{
    return ctx->pc;
}

void frame_arch_set_ip(intr_frame_t *ctx, vaddr_t ip)
{
    ctx->pc = ip;
}

vaddr_t frame_arch_get_sp(intr_frame_t *ctx)
{
    return ctx->sp;
}

void frame_arch_set_sp(intr_frame_t *ctx, vaddr_t sp)
{
    ctx->sp = sp;
}

void frame_arch_set_retval(intr_frame_t *ctx, uint64_t val)
{
    ctx->ax = val;
}

void ctx_arch_clone_frame(intr_frame_t *child_frame, intr_frame_t *parent_frame)
{
    memcpy(child_frame, parent_frame, sizeof(intr_frame_t));
}

vaddr_t ctx_arch_init_fork(cpu_ctx_t *child_ctx, vaddr_t kernel_stack_top, intr_frame_t *child_frame)
{
    memset(child_ctx, 0, sizeof(cpu_ctx_t));

    kernel_stack_top -= sizeof(intr_frame_t);
    intr_frame_t *frame = (intr_frame_t *)kernel_stack_top;
    memcpy(frame, child_frame, sizeof(intr_frame_t));

    // frame->sp = (reg_t)child_frame;
    frame->cs = USER_CODE_SELECTOR | 3;
    frame->ss = USER_DATA_SELECTOR | 3;
    frame->ds = USER_DATA_SELECTOR | 3;
    frame->flags = X86_EFLAGS_IF | X86_EFLAGS_FIXED;

    // point ctx at the fake frame
    child_ctx->frame.sp = kernel_stack_top;
    child_ctx->frame.regs = frame;
    return kernel_stack_top;
}
