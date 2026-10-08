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
        irq_descriptors[i].id = (~0);
        irq_descriptors[i].flags |= HANDLER_SHAREABLE;
        for (size_t c = 0; c < ARRAY_SIZE(irq_descriptors[i].vectors); c++)
        {
            irq_descriptors[i].vectors[c] = (~0);
        }
        }

    irq_descriptors[EXC_SYSCALL - IRQ_BASE].flags = HANDLER_IN_USE | HANDLER_FULL;

    resource_t *irqs = resource_create();
    resource_request(&irq_space.root, irqs, IRQ_BASE, MAX_IRQ_VECTORS, "Kernel IRQ", RES_TYPE_RAM, RES_FLAG_NONE);
    resource_dump(&irq_space.root);

    current = get_ops();

    return KERRNO_SUCCESSES;
}

status_t irq_release_reserved(kernel_irq_t irq, size_t count)
{
    for (kernel_irq_t i = irq; i < count; i++)
    {
        irq_descriptor_t *entry = &irq_descriptors[i];
        FLAG_UNSET(entry->flags, HANDLER_RESERVED);
    }
    return KERRNO_SUCCESSES;
}

void irq_print_descriptor(irq_descriptor_t *descriptor)
{
    trace_debug(MODULE, "desc @ %p = {id: %u, flags 0x%x}", descriptor, descriptor->id, descriptor->flags);
}

INTERNAL status_t irq_pick_core(irq_pick_reg_t *r, kernel_irq_t *out)
{
    ENTER_FUNC("%p = {%p, {0x%x, 0x%x}, %u, 0x%x, %u, 0x%x}", r, r->allowed, r->range.first, r->range.last, r->count, r->align, r->source, r->target);
    kernel_irq_t i = 0;
    kernel_irq_t end = ARRAY_SIZE(irq_descriptors);
    kernel_irq_t first = (~0);
    size_t count = r->count;
    if (r->use_range)
    {
        i = r->range.first;
        end = r->range.last;
    }
    for (; i < end; i++)
    {
        if ((i % r->align) != 0)
        {
            if (count != r->count)
            {
                irq_release_reserved(first, count);
                count = r->count;
                first = (~0);
            }
            continue;
        }

        if (r->use_bit_mask)
        {
            if (r->allowed != NULL)
            {
                if ((r->allowed->bits[i / sizeof(*r->allowed)] & (1ull << (i % sizeof(*r->allowed)))) == 0)
                {
                    if (count != r->count)
                    {
                        irq_release_reserved(first, count);
                        count = r->count;
                        first = (~0);
                    }
                    continue;
                }
            }
        }
        irq_descriptor_t *entry = &irq_descriptors[i];

        if (FLAG_GET(entry->flags, HANDLER_RESERVED))
        {
            if (count != r->count)
            {
                irq_release_reserved(first, count);
                count = r->count;
                first = (~0);
            }
            continue;
        }
        if (FLAG_GET(entry->flags, HANDLER_FULL))
        {
            if (count != r->count)
            {
                irq_release_reserved(first, count);
                count = r->count;
                first = (~0);
            }
            continue;
        }

        if (r->target != ANY_CPU)
        {
            cpu_t *cpu = cpu_arch_get(r->target);
            irq_handler_t *handler = &cpu->irq_handlers[entry->vectors[r->target]];
            // irq_print_descriptor(entry);
            // trace_debug(MODULE, "handler[%u] @ %p = {state = 0x%x}", entry->vectors[r->target], handler, handler->state);
            if (FLAG_IS_SET(handler->state, HANDLER_IN_USE))
            {
                if (count != r->count)
                {
                    irq_release_reserved(first, count);
                    count = r->count;
                    first = (~0);
                }
                continue;
            }
        }
        else
        {
            if (FLAG_IS_SET(entry->flags, HANDLER_IN_USE))
            {
                if (count != r->count)
                {
                    irq_release_reserved(first, count);
                    count = r->count;
                    first = (~0);
                }
                continue;
            }
        }
        FLAG_SET(entry->flags, HANDLER_RESERVED);
        switch (r->source)
        {
            case IRQ_SOURCE_GSI :
                FLAG_SET(entry->flags, HANDLER_GSI);
                break;
            case IRQ_SOURCE_MSI :
                FLAG_SET(entry->flags, HANDLER_MSI);
                break;
            default :
                break;
        }

        if (first == (~0))
        {
            first = i;
        }
        count--;

        if (count == 0)
        {
            break;
        }
    }
    *out = first;
    return KERRNO_SUCCESSES;
}

status_t irq_register_handler(kernel_irq_t irq, irq_source_t source, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target)
{
    ENTER_FUNC("0x%x, %u, %p, %p, %u, %u, 0x%x", irq, source, handler, ctx, trigger, polarity, target);

    status_t ret;

    irq_descriptor_t *descriptor = &irq_descriptors[irq];
    if (FLAG_IS_SET(descriptor->flags, HANDLER_IN_USE))
    {
        irq_print_descriptor(descriptor);
        KERRNO_RETURN(KERRNO_DONT_DO_THAT, "TODO");
    }
    descriptor->id = irq;

    cpu_logical_id_t target_cpu;

    if (source == IRQ_SOURCE_MSI)
    {
        FLAG_SET(descriptor->flags, HANDLER_MSI);
    }
    else if (source == IRQ_SOURCE_IRQ)
    {
        gsi_t gsi = current->pick_free_pin(~0);
        ret = current->bind(gsi, irq);
        if (ret != KERRNO_SUCCESSES)
        {
            KERRNO_RETURN(KERRNO_DONT_DO_THAT, "TODO");
        }
    }

    FLAG_SET(descriptor->flags, HANDLER_IN_USE);
    // descriptor->polarity = polarity;
    // descriptor->trigger = trigger;

    ret = irq_arch_register(descriptor, current, source, handler, ctx, trigger, polarity, target, &target_cpu);

    if (ret != KERRNO_SUCCESSES)
    {
        KERRNO_RETURN(KERRNO_PERMISSION_DENIED, "ivt entry already in use");
    }

    trace_debug(MODULE, "current @ %p, current->debug @ %p", current, current->debug);
    current->debug();

    trace_debug(MODULE, "target_cpu = %u", target_cpu);
    trace_info(MODULE, "Registering IRQ handler (%p) on irq %u/vector %d in cpu%u", handler, irq, descriptor->vectors[target_cpu] + IRQ_BASE, target_cpu);

    ret = ivt_set_handler(descriptor->vectors[target_cpu], irq_handler);

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
    current->send_eoi(irq);
    return KERRNO_SUCCESSES;
}

status_t irq_pick_free_entry(kernel_irq_t allowed_mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out)
{
    ENTER_FUNC("0x%x, %u, 0x%x, %p", allowed_mask, source, target, out);
    irq_pick_reg_t r = {
        .align = 1,
        .count = 1,
        .allowed = NULL,
        .range = {
            .first = 0,
            .last = (~0)},
        .source = source,
        .target = target,
        .use_bit_mask = true,
        .use_range = false,
    };
    return irq_pick_core(&r, out);
    // return irq_arch_pick_free_entry(allowed_mask, source, target, out);
}
status_t irq_pick_free_range_size(irq_range_t range, size_t count, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out)
{
    irq_pick_reg_t r = {
        .align = 1,
        .count = count,
        .range = range,
        .source = source,
        .target = target,
        .use_bit_mask = false,
        .use_range = true,
    };
    return irq_pick_core(&r, out);
    // return irq_arch_pick_free_entry(allowed_mask, source, target, out);
}

status_t irq_pick_free_gsi(uint32_t allowed_pins, kernel_irq_t *irq_out, gsi_t *out)
{
    ENTER_FUNC("0x%x, %p", allowed_pins, out);
    *out = current->pick_free_pin(allowed_pins);
    irq_pick_reg_t r = {
        .align = 1,
        .count = 1,
        .allowed = NULL,
        .source = IRQ_SOURCE_IRQ,
        .target = ANY_CPU,
        .use_bit_mask = true,
        .use_range = false,
    };
    status_t ret = irq_pick_core(&r, irq_out);

    ret |= current->bind(*out, *irq_out);

    return ret;
}
