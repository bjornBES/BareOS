/*
 * File: irq.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 06 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "irq.h"
#include "errno/errno.h"
#include "kernel/irq.h"
#include "kernel/x86.h"
#include "kernel/ivt.h"
#include "i8259.h"
#include "acpi/table.h"
#include "kernel/asm/acpi/apic/apic.h"
#include "kernel/asm/acpi/apic/ioapic.h"
#include "debug/debug.h"
#include "kernel/asm/exceptions/isr.h"

#include "drivers/serial/UART/UART.h"

#include <util/arrays.h>
#include "stdio.h"
#include "kernel/io.h"

#define MODULE "IRQ"

const pic_driver *driver = NULL;
static bool irq_used[24];
static irq_handler_t irq_handlers[24];
static void *irq_ctx[24];

int x86_irq_handler(intr_frame_t *regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;

/*     if (irq > 0)
    {
        log_debug(MODULE, "Interrupt Req %u", irq);
    } */

    if (irq_arch_is_masked(irq))
    {
        irq_arch_eoi(irq);
        return RETURN_GOOD;
    }

    if (irq_used[irq])
    {
        irq_handlers[irq](regs, irq_ctx[irq]);
    }
    else
    {
        log_err(MODULE, "Unhandled IRQ %d base was %u", irq, regs->interrupt);
        return RETURN_FAILED;
    }
    
    irq_arch_eoi(irq);
    return RETURN_GOOD;
}

void irq_arch_initialize()
{
    const pic_driver *drivers[] = {
        apic_get_driver(),
        i8259_get_driver(),
    };

    i8259_get_driver()->disable();

    for (int i = 0; i < ARRAY_SIZE(drivers); i++)
    {
        if (drivers[i]->probe())
        {
            driver = drivers[i];
            break;
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
        ivt_arch_set_handler(i + PIC_REMAP_OFFSET, x86_irq_handler);
    }
}

int irq_arch_register(uint32_t irq, irq_handler_t handler, void *ctx)
{
    uint32_t real_irq = driver->get_real_irq(irq);
    if (irq_used[real_irq])
    {
        return -EBUSY;
    }
    log_debug(MODULE, "Registering IRQ handler (%p) for IRQ %d", handler, real_irq);

    irq_used[real_irq] = true;
    irq_handlers[real_irq] = handler;
    irq_ctx[real_irq] = ctx;

    uint8_t vector = PIC_REMAP_OFFSET + real_irq;
    ioapic_set_entry(real_irq, vector, 0);

    // unmask interrupt
    irq_arch_unmask(real_irq);
    return RETURN_GOOD;
}

void irq_arch_unregister(uint32_t irq)
{
    uint32_t real_irq = driver->get_real_irq(irq);
    irq_used[real_irq] = false;
    irq_handlers[real_irq] = NULL;
    irq_ctx[real_irq] = NULL;
    irq_arch_mask(real_irq);
}

void irq_arch_mask(uint32_t irq)
{
    driver->mask(irq);
}

void irq_arch_unmask(uint32_t irq)
{
    driver->unmask(irq);
}

bool irq_arch_is_masked(uint32_t irq)
{
    return driver->is_masked(irq);
}

inline void irq_arch_enable()
{
    inline_asm("sti" : : : "memory");
}

inline void irq_arch_disable()
{
    inline_asm("cli" : : : "memory");
}

uint32_t irq_pick_free_irq(uint32_t allowed_mask)
{
    // iterate allowed IRQs from the HPET capability register
    for (uint32_t irq = 0; irq < 24; irq++)
    {
        // skip if not in allowed set
        if (!(allowed_mask & (1u << irq)))
        {
            continue;
        }

        // skip legacy IRQs that are already taken
        if (irq_used[irq])
        {
            continue;
        }

        // skip IRQ 0 (PIT) and IRQ 1 (keyboard) — reserved
        if (irq == 0 || irq == 1)
        {
            continue;
        }

        return irq;
    }

    // no free IRQ found — should not happen on sane hardware
    panic(MODULE, __FILE__, __LINE__, "pick_free_irq: no free IRQ in allowed set 0x%X", allowed_mask);
    return 0xFF;
}
