/*
 * File: irq.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "irq/irq.h"
#include "ivt/ivt.h"

#include "cpu/cpu.h"

#include "asm/cpu_arch.h"
#include "asm/irq_arch.h"

#include "asm/vectors_arch.h"

#include "resource/resource.h"

#include "memory.h"
#include "kerrno.h"
#include "panic.h"

#include <array.h>

#define MODULE "irq"

irq_descriptor_t irq_descriptors[MAX_IRQ_VECTORS - IRQ_BASE];

irq_controller_t *current;

int irq_handler(intr_frame_t *regs)
{
    interrupt_vector_t vector = regs->interrupt;

    irq_handler_t *handler = irq_arch_get_handler(vector);

    if (irq_is_masked(vector))
    {
        irq_eoi(vector);
        return KERRNO_SUCCESSES;
    }

    if (FLAG_IS_SET(handler->state, HANDLER_IN_USE))
    {
        handler->handler(regs, handler->ctx);
    }
    else
    {
        log_err(MODULE, "Unhandled IRQ %d base was %u", vector, regs->interrupt);
        return KERRNO_SUCCESSES;
    }

    irq_eoi(vector);
    return KERRNO_SUCCESSES;
}

status_t irq_initialize(irq_controller_t *(*get_ops)())
{
    ENTER_FUNC("%p", get_ops);
    irq_controller_t *controller = get_ops();
    if (controller->probe() != KERRNO_SUCCESSES)
    {
        current = NULL;
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "%s failed probe", controller->name);
    }

    if (controller->initialize() != KERRNO_SUCCESSES)
    {
        current = NULL;
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "%s failed initialize", controller->name);
    }

    memset(irq_descriptors, 0, sizeof(irq_descriptors));
    for (size_t i = 0; i < ARRAY_SIZE(irq_descriptors); i++)
    {
        irq_descriptors[i].cpu = ANY_CPU;
        irq_descriptors[i].flags |= HANDLER_SHARED;
    }

    irq_descriptors[EXC_SYSCALL - IRQ_BASE].flags = HANDLER_IN_USE;

    resource_t *irqs = resource_create();
    resource_request(&irq_space.root, irqs, IRQ_BASE, MAX_IRQ_VECTORS, "Kernel IRQ", RES_TYPE_RAM, RES_FLAG_NONE);
    resource_dump(&irq_space.root);

    current = get_ops();

    return KERRNO_SUCCESSES;
}

status_t irq_register_handler(kernel_irq_t irq, irq_source_t source, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target)
{
    ENTER_FUNC("0x%x, %u, %p, %p, %u, %u, 0x%x", irq, source, handler, ctx, trigger, polarity, target);
    irq_descriptor_t *descriptor = &irq_descriptors[irq];
    if (source == IRQ_SOURCE_MSI)
    {
        FLAG_SET(descriptor->flags, HANDLER_MSI);
    }
    FLAG_SET(descriptor->flags, HANDLER_IN_USE);
    descriptor->id = irq;
    descriptor->polarity = polarity;
    descriptor->trigger = trigger;

    status_t ret = irq_arch_register(descriptor, current, source, handler, ctx, target);

    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "ivt entry already in use");
    }

    trace_info(MODULE, "Registering IRQ handler (%p) on irq %u/vector %d in cpu%u", handler, irq, descriptor->vector, descriptor->cpu);

    ret = ivt_set_handler(descriptor->vector, irq_handler);

    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "ivt entry already in use");
    }

    return KERRNO_SUCCESSES;
}

status_t irq_unregister_handler(kernel_irq_t irq)
{

    return KERRNO_SUCCESSES;
}

status_t irq_mask(kernel_irq_t irq)
{
    return KERRNO_SUCCESSES;
}

status_t irq_unmask(kernel_irq_t irq)
{
    return KERRNO_SUCCESSES;
}

bool irq_is_masked(kernel_irq_t irq)
{
    return false;
}

status_t irq_eoi(kernel_irq_t irq)
{
    return KERRNO_SUCCESSES;
}

status_t irq_pick_free_entry(kernel_irq_t allowed_mask, irq_source_t source, kernel_irq_t *out)
{
    ENTER_FUNC("0x%x, %u, %p", allowed_mask, source, out);
    return irq_arch_pick_free_entry(allowed_mask, source, ANY_CPU, out);
}

status_t irq_pick_free_entry_cpu(kernel_irq_t allowed_mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out)
{
    ENTER_FUNC("0x%x, %u, 0x%x, %p", allowed_mask, source, target, out);
    return irq_arch_pick_free_entry(allowed_mask, source, target, out);
}
