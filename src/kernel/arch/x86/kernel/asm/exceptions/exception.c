/*
 * File: exception.c
 * File Created: 04 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "exception.h"
#include "handler.h"
#include "stdio.h"
#include "kernel.h"
#include "isr.h"
#include "kernel/ivt.h"

#define MODULE "exception"

arch_handler exception_handlers_arch[256];
int count[256];

int last_vector;

int exception_handler(intr_frame_t *regs)
{
    int interrupt = regs->interrupt;
    if (exception_handlers_arch[interrupt] == NULL)
    {
        return RETURN_FAILED;
    }
    
    if (interrupt < 0x20)
    {
        if (last_vector == interrupt)
        {
            count[interrupt]++;
        }
        else
        {
            count[last_vector] = 0;
            count[interrupt] = 0;
        }
    }
    if (count[interrupt] >= 3)
    {
        panic(MODULE, __FILE__, __LINE__, "Too many exceptions on vector %u", last_vector);
    }
    last_vector = interrupt;
    if (exception_handlers_arch[interrupt] != NULL)
    {
        int result = exception_handlers_arch[interrupt](regs);
        if (interrupt != 0x80 && interrupt < 0x20)
        {
            ivt_dump_frame(regs);
        }
        if (result == RETURN_GOOD)
        {
            count[interrupt] = 0;
        }
        else
        {
            // log_crit(MODULE, "Unhandled exception %d %s 0x%x", regs->interrupt, g_Exceptions[regs->interrupt], regs->error);
            KernelPanic(MODULE, "Unhandled exception %d", regs->interrupt);
        }
    }
    return RETURN_GOOD;
}

void ivt_arch_set_handler(uint32_t interrupt, int (*handler)(intr_frame_t *))
{
    x86_isr_register_handler(interrupt);
    exception_handlers_arch[interrupt] = handler;
}

void exception_init()
{
    x86_isr_initialize();
    handler_init();
}
