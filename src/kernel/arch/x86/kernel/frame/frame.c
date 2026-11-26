/*
 * File: frame.c
 * File Created: 15 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Sep 2026
 * Modified By: BjornBEs
 * -----
 */


#include "asm/frame_arch.h"
#include "type_arch.h"

#include "debug/debug.h"

#define MODULE "x86-frame"

void frame_arch_dump_frame(intr_frame_t *frame)
{
    pr_info("VEC=0x%02x ERR=0x%04lx PC =0x%04x:%016llx SP =0x%04x:%016llx", frame->interrupt, frame->error, frame->cs, frame->pc, frame->ss, frame->sp);
    pr_info("RAX: 0x%016llx RBX: 0x%016llx RCX: 0x%016llx RDX: 0x%016llx", frame->ax, frame->bx, frame->cx, frame->dx);
    pr_info("RSI: 0x%016llx RDI: 0x%016llx RSP: 0x%016llx RBP: 0x%016llx", frame->si, frame->di, frame->sp, frame->bp);
    pr_info("R8:  0x%016llx R9:  0x%016llx R10: 0x%016llx R11: 0x%016llx", frame->r8, frame->r9, frame->r10, frame->r11);
    pr_info("R12: 0x%016llx R13: 0x%016llx R14: 0x%016llx R15: 0x%016llx", frame->r12, frame->r13, frame->r14, frame->r15);
    pr_info("RIP: 0x%016llx RFL: 0x%08lx", frame->pc, frame->flags);
    pr_info("ES:  0x%04x", frame->es);
    pr_info("CS:  0x%04x", frame->cs);
    pr_info("SS:  0x%04x", frame->ss);
    pr_info("DS:  0x%04x", frame->ds);
    pr_info("FS:  0x%04x", frame->fs);
    pr_info("GS:  0x%04x", frame->gs);

    // trace_info(NO_MODULE, "SP:  0x%04x:%016llx }", frame->ss, frame->sp);
    // trace_info(NO_MODULE, "BP:  0x%04x:%016llx }", frame->ss, frame->bp);
    // trace_info(NO_MODULE, "PC:  0x%04x:%016llx }", frame->cs, frame->pc);
}