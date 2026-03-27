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
#include "pic.h"
#include "x86.h"
#include "i8259.h"
#include "debug/debug.h"

#include "drivers/serial/UART/UART.h"

#include <util/arrays.h>
#include "libs/stdio.h"
#include "libs/IO.h"

#define PIC_REMAP_OFFSET 0x20
#define MODULE "IRQ"

IRQHandler _IRQHandlers[16];
static const PICDriver *_Driver = NULL;

void x86_IRQ_Handler(Registers *regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

    UART_write_fstr(COM1, "Interrupt Req %u\r\n", irq);

    if (_IRQHandlers[irq] != NULL)
    {
        _IRQHandlers[irq](regs);
    }
    else
    {
        log_err(MODULE, "Unhandled IRQ %d...", irq);
    }
    
    _Driver->SendEndOfInterrupt(irq);
}

void x86_irq_initialize()
{
    const PICDriver *drivers[] ={
        i8259_get_driver(),
    };

    for (int i = 0; i < SIZE(drivers); i++)
    {
        if (drivers[i]->Probe())
        {
            _Driver = drivers[i];
        }
    }

    if (_Driver == NULL)
    {
        fprintf(stddebug, "No PIC found!");
        return;
    }

    log_info(MODULE, "Found %s PIC.", _Driver->Name);
    _Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    for (size_t i = 0; i < 16; i++)
    {
        x86_isr_register_handler(i + PIC_REMAP_OFFSET, x86_IRQ_Handler);
    }

    enable_interrupts();
}

void x86_irq_register_handler(int irq, IRQHandler handler)
{
/*     if (irq > 15)
    {
        return;
    } */

    log_debug(MODULE, "Registering IRQ handler for IRQ %d", irq);

    _IRQHandlers[irq] = handler;
    // unmask interrupt
    if (irq < 8)
    {
        uint8_t data = inb(0x21) & ~(1 << irq);
        outb(0x21, data);
    }
    else
    {
        uint8_t data = inb(0xA1) & ~(1 << (irq - 8));
        outb(0xA1, data);
    }
}