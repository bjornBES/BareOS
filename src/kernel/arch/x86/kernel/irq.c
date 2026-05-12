/*
 * File: irq.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "irq.h"
#include "x86.h"
#include "i8259.h"
#include "apic/apic.h"
#include "debug/debug.h"
#include "exceptions/isr.h"

#include "drivers/serial/UART/UART.h"

#include <util/arrays.h>
#include "libs/stdio.h"
#include "libs/IO.h"

#define MODULE "IRQ"

const pic_driver *driver = NULL;
IRQ_handler irq_handlers[16];

void x86_irq_handler(registers *regs, kernel_handler null)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (irq > 0)
    {
        // log_debug(MODULE, "Interrupt Req %u", irq);
    }

    if (irq_handlers[irq] != NULL)
    {
        irq_handlers[irq](regs);
    }
    else
    {
        log_err(MODULE, "Unhandled IRQ %d base was %u", irq, regs->interrupt);
    }

    driver->send_eoi(irq);
}

void x86_irq_initialize()
{
    const pic_driver *drivers[] = {
        i8259_get_driver(),
    };

    for (int i = 0; i < SIZE(drivers); i++)
    {
        if (drivers[i]->probe())
        {
            driver = drivers[i];
        }
    }

    if (driver == NULL)
    {
        fprintf(stddebug, "No PIC found!");
        return;
    }

    log_info(MODULE, "Found %s PIC.", driver->name);
    driver->initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    for (size_t i = 0; i < 16; i++)
    {
        exception_register_arch_handler(i + PIC_REMAP_OFFSET, x86_irq_handler);
    }
    
}

void irq_register_handler(int irq, IRQ_handler handler)
{
    int real_irq = apic_get_gsi(irq);
    log_debug(MODULE, "Registering IRQ handler (%p) for IRQ %d", handler, real_irq);
    
    irq_handlers[real_irq] = handler;
    // unmask interrupt
    driver->unmask(real_irq);
    log_debug(MODULE, "unmasking %d", real_irq);
}

bool irq_inject_driver(const pic_driver *new_driver)
{
    if (new_driver->probe())
    {
        driver->disable();
        driver = new_driver;
    }
    else
    {
        return false;
    }

    log_info(MODULE, "Injected %s PIC.", driver->name);
    driver->initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);
    return true;
}