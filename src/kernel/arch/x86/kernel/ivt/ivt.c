/*
 * File: ivt.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ivt.h"
#include "asm/ivt_arch.h"

#include "entry/desc/idt/idt.h"

#include "memory.h"
#include "errno.h"

#include <types.h>
#include <binary.h>

#define MODULE "x86-ivt"

typedef struct ivt_handler
{
    interrupt_handler handler;
    uint8_t state;
    int count;
} ivt_handler_t;

ivt_handler_t handlers[256];

int ivt_arch_handler(interrupt_vector vector, intr_frame_t *frame)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        ERRNO_RETURN(ENOSYS, "Vector %u's handler has not been set", vector);
    }

    if (FLAG_IS_SET(handlers[vector].state, HANDLER_IS_ACTIVE))
    {
        int result = handlers[vector].handler(frame);
        if (result == 1)
        {
            handlers[vector].count = 0;
        }
        else if (result != 0)
        {
            ERRNO_RETURN(ENOSYS, "Handler returned a non 0 value");
        }
    }
    else
    {
        log_debug(MODULE, "flags = 0x%x", handlers[vector].state);
        ERRNO_RETURN(ENOSYS, "Vector %u's handler is not active", vector);
    }
    return 0;
}

void ivt_arch_init()
{
    memset(handlers, 0, sizeof(handlers));
    // handler_init();
}

int ivt_arch_set_handler(interrupt_vector vector, interrupt_handler handler)
{
    if (FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        ERRNO_RETURN(EPERM, "handler is already in use", 0);
    }
    log_debug(MODULE, "Registering IVT handler (%p) on vector %d", handler, vector);
    handlers[vector].state = 0;
    FLAG_SET(handlers[vector].state, HANDLER_IN_USE);
    handlers[vector].count = 0;
    handlers[vector].handler = handler;
    ivt_arch_enable(vector);
    return 0;
}

int ivt_arch_enable(interrupt_vector vector)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        ERRNO_RETURN(EPERM, "handler not in use", 0);
    }
    idt_enable_gate(vector);
    FLAG_SET(handlers[vector].state, HANDLER_IS_ACTIVE);
    return 0;
}

int ivt_arch_disable(interrupt_vector vector)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        ERRNO_RETURN(EPERM, "handler not in use", 0);
    }
    FLAG_UNSET(handlers[vector].state, HANDLER_IS_ACTIVE);
    idt_disable_gate(vector);
    return 0;
}

interrupt_vector ivt_find_free_vector(interrupt_vector mask)
{
    FUNC_NOT_IMPLEMENTED();
    return ENOSYS;
}

void ivt_dump_frame(intr_frame_t *frame)
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
