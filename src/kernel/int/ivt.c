/*
 * File: ivt.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "ivt/ivt.h"
#include "asm/ivt_arch.h"

#include "entry/desc/idt/idt.h"

#include "memory.h"
#include "kerrno.h"

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

status_t ivt_handler(interrupt_vector_t vector, intr_frame_t *frame)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "Vector %u's handler has not been set", vector);
    }

    if (FLAG_IS_SET(handlers[vector].state, HANDLER_IS_ACTIVE))
    {
        status_t result = handlers[vector].handler(frame);
        if (result == KERRNO_SUCCESSES)
        {
            handlers[vector].count = 0;
        }
        else
        {
            KERRNO_RETURN(ENOSYS, "Handler returned a non 0 value");
        }
    }
    else
    {
        log_debug(MODULE, "flags = 0x%x", handlers[vector].state);
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "Vector %u's handler is not active", vector);
    }
    return KERRNO_SUCCESSES;
}

void ivt_init()
{
    memset(handlers, 0, sizeof(handlers));
    // handler_init();
}

status_t ivt_set_handler(interrupt_vector_t vector, interrupt_handler handler)
{
    log_info(MODULE, "Registering IVT handler (%p) on vector %d", handler, vector);
    handlers[vector].state = 0;
    FLAG_SET(handlers[vector].state, HANDLER_IN_USE);
    handlers[vector].count = 0;
    handlers[vector].handler = handler;
    status_t status = ivt_enable_vector(vector);
    return status;
}

status_t ivt_enable_vector(interrupt_vector_t vector)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "handler not in use", 0);
    }
    idt_enable_gate(vector);
    FLAG_SET(handlers[vector].state, HANDLER_IS_ACTIVE);
    return KERRNO_SUCCESSES;
}

status_t ivt_disable_vector(interrupt_vector_t vector)
{
    if (!FLAG_IS_SET(handlers[vector].state, HANDLER_IN_USE))
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "handler not in use", 0);
    }
    FLAG_UNSET(handlers[vector].state, HANDLER_IS_ACTIVE);
    idt_disable_gate(vector);
    return KERRNO_SUCCESSES;
}

interrupt_vector_t ivt_find_free_vector(interrupt_vector_t mask)
{
    FUNC_NOT_IMPLEMENTED();
    return 0;
}


