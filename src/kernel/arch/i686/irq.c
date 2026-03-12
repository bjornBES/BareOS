#include "irq.h"
#include "pic.h"
#include "i8259.h"

#include <util/arrays.h>
#include <stdio.h>
#include <IO.h>

#define PIC_REMAP_OFFSET 0x20
#define MODULE "PIC"

IRQHandler _IRQHandlers[16];
static const PICDriver *_Driver = NULL;

void i686_IRQ_Handler(Registers *regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

    if (_IRQHandlers[irq] != NULL)
    {
        _IRQHandlers[irq](regs);
    }
    else
    {
        fprintf(stddebug, "Unhandled IRQ %d...", irq);
    }
    
    _Driver->SendEndOfInterrupt(irq);
}

void i686_IRQInitialize()
{
    const PICDriver *drivers[] ={
        i8259_GetDriver(),
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

    _Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    for (size_t i = 0; i < 16; i++)
    {
        i686_IRQRegisterHandler(PIC_REMAP_OFFSET + 1, i686_IRQ_Handler);
    }

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void i686_IRQRegisterHandler(int irq, IRQHandler handler)
{
    if (irq > 15)
    {
        return;
    }

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