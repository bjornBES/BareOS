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

void frame_arch_dump_frame(intr_frame_t *frame)
{
    log_info(NO_MODULE, "VEC=0x%02x ERR=0x%04lx PC =0x%04x:%016llx SP =0x%04x:%016llx", frame->interrupt, frame->error, frame->cs, frame->pc, frame->ss, frame->sp);
    
    log_info(NO_MODULE, "RAX: 0x%016llx RBX: 0x%016llx RCX: 0x%016llx RDX: 0x%016llx", frame->ax, frame->bx, frame->cx, frame->dx);
    log_info(NO_MODULE, "RSI: 0x%016llx RDI: 0x%016llx RSP: 0x%016llx RBP: 0x%016llx", frame->si, frame->di, frame->sp, frame->bp);
    log_info(NO_MODULE, "R8:  0x%016llx R9:  0x%016llx R10: 0x%016llx R11: 0x%016llx", frame->r8, frame->r9, frame->r10, frame->r11);
    log_info(NO_MODULE, "R12: 0x%016llx R13: 0x%016llx R14: 0x%016llx R15: 0x%016llx", frame->r12, frame->r13, frame->r14, frame->r15);
    log_info(NO_MODULE, "RIP: 0x%016llx RFL: 0x%08lx", frame->pc, frame->flags);

    log_info(NO_MODULE, "ES:  0x%04x", frame->es);
    log_info(NO_MODULE, "CS:  0x%04x", frame->cs);
    log_info(NO_MODULE, "SS:  0x%04x", frame->ss);
    log_info(NO_MODULE, "DS:  0x%04x", frame->ds);
    log_info(NO_MODULE, "FS:  0x%04x", frame->fs);
    log_info(NO_MODULE, "GS:  0x%04x", frame->gs);

    // log_info(NO_MODULE, "SP:  0x%04x:%016llx }", frame->ss, frame->sp);
    // log_info(NO_MODULE, "BP:  0x%04x:%016llx }", frame->ss, frame->bp);
    // log_info(NO_MODULE, "PC:  0x%04x:%016llx }", frame->cs, frame->pc);
}