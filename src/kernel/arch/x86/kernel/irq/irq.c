/*
 * File: irq.c
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/irq_arch.h"
#include "asm/cpu_arch.h"
#include "asm/smp_arch.h"

#include "kernel/irq/irq.h"
#include "kernel/cpu/cpu.h"
#include "kernel/acpi/apic/apic.h"

#include "ivt/ivt.h"
#include "irq/irq.h"

#include "init.h"

#include "kerrno.h"
#include "memory.h"

#include "math.h"

#include <defs.h>

#define MODULE "x86-irq"

void irq_arch_enable()
{
    inline_asm("sti");
}

void irq_arch_disable()
{
    inline_asm("cli");
}

void irq_save(reg_t *irq)
{
#ifdef __x86_64__
    inline_asm(
        "pushfq\n\t"
        "pop %0" : "=m"(irq) : : "memory");
#else
    inline_asm(
        "pushfd\n\t"
        "pop %0" : "=r"((uintptr_t)irq) : : "memory");
#endif
}

void irq_restore(reg_t irq)
{
#ifdef __x86_64__
    inline_asm(
        "push %0\n\t"
        "popfq" : : "r"(irq) : "memory");
#else
    inline_asm(
        "push %0\n\t"
        "popfd" : : "r"(irq) : "memory");
#endif
}

status_t irq_arch_initcall()
{
    ENTER_FUNC("", 0);
    cpu_t *bsp_cpu = cpu_arch_get_bsp();

    // check CPUID.0x01:EDX[9] APIC
    if (bsp_cpu->cpuid.leaf_0x1_0->apic)
    {
        status_t status = irq_initialize(apic_get_driver);
        if (status != KERRNO_SUCCESSES)
        {
            log_err(MODULE, "PIC time");
            goto pic_time; // sorry...
        }
        return KERRNO_SUCCESSES;
    }
    else
    {
pic_time:
        FUNC_NOT_IMPLEMENTED();
    }
    return KERRNO_UNSUCCESS;
}

POSTCORE_INITCALL(irq_arch_initcall);

INTERNAL interrupt_vector_t irq_arch_alloc_vector(cpu_t *cpu, interrupt_vector_t offset)
{
    for (interrupt_vector_t v = offset; v < MAX_IRQ_VECTORS - IRQ_BASE; v++)
    {
        if (!FLAG_GET(cpu->irq_handlers[v].state, HANDLER_IN_USE))
        {
            FLAG_SET(cpu->irq_handlers[v].state, HANDLER_IN_USE);
            return v + IRQ_BASE;
        }
    }
    return KERRNO_UNSUCCESS;
}

void cpu_inc_register_int(uintptr_t _)
{
    ENTER_FUNC("%p", _);
    irq_descriptor_t *desc = (irq_descriptor_t *)_;
    cpu_t *cpu = THISCPU();
    memcpy(&cpu->irq_handlers[desc->vector], &desc->handler, sizeof(irq_handler_t));
    cpu->irq_count++;
}

INTERNAL status_t irq_arch_get_vector(cpu_logical_id_t target, interrupt_vector_t offset, interrupt_vector_t *vector_out, cpu_entry_t **entry_out)
{
    cpu_entry_t *entry = NULL;
    if (target == ANY_CPU)
    {
        uint32_t lowest_irq_count = UINT32_MAX;
        for (size_t i = 0; i < cpu_get_count(); i++)
        {
            entry = cpu_get_entry(i);
            lowest_irq_count = min(entry->cpu->irq_count, lowest_irq_count);
            if (entry->cpu->irq_count == lowest_irq_count)
            {
                target = entry->logical_id;
            }
        }
    }
    entry = cpu_get_entry(target);
    cpu_t *cpu = entry->cpu;

    interrupt_vector_t vector = irq_arch_alloc_vector(cpu, offset);
    if (vector == KERRNO_UNSUCCESS)
    {
        for (size_t i = 0; i < cpu_get_count(); i++)
        {
            entry = cpu_get_entry(i);
            cpu = entry->cpu;
            vector = irq_arch_alloc_vector(cpu, offset);
            if (vector != KERRNO_UNSUCCESS)
            {
                break;
            }
        }
    }
    if (vector == KERRNO_UNSUCCESS)
    {
        KERRNO_RETURN(KERRNO_UNSUCCESS, "didn't find a free cpu handler");
    }
    *vector_out = vector;
    *entry_out = entry;
    return KERRNO_SUCCESSES;
}

status_t irq_arch_register(irq_descriptor_t *desc, irq_controller_t *current, irq_source_t source, irq_handler_func_t handler, void *ctx, cpu_logical_id_t target)
{
    interrupt_vector_t vector;
    cpu_entry_t *entry;
    status_t ret;
    cpu_t *cpu;
    interrupt_vector_t offset = 0;
    do
    {
        ret = irq_arch_get_vector(target, offset, &vector, &entry);
        if (ret != KERRNO_SUCCESSES)
        {
            return ret;
        }
        cpu = entry->cpu;

        if (source == IRQ_SOURCE_IRQ)
        {
            ret = current->route(vector - IRQ_BASE, vector, cpu->logical_id, desc->trigger, desc->polarity);
        }
        else
        {
            FUNC_NOT_IMPLEMENTED();
        }
        if (ret != KERRNO_SUCCESSES)
        {
            if (offset == 0)
            {
                offset = vector - IRQ_BASE;
            }
            FLAG_UNSET(cpu->irq_handlers[vector - IRQ_BASE].state, HANDLER_IN_USE);
            offset++;
        }
    } while (ret != KERRNO_SUCCESSES);

    desc->vector = vector;
    desc->cpu = cpu->logical_id;
    desc->handler.ctx = ctx;
    desc->handler.handler = handler;
    desc->handler.state = HANDLER_IN_USE;
    if (source == IRQ_SOURCE_MSI)
    {
        FLAG_SET(desc->handler.state, HANDLER_MSI);
    }

    trace_debug(MODULE, "cpu @ %p THISCPU() @ %p", cpu, THISCPU());
    if (THISCPU() == NULL || cpu == THISCPU())
    {
        cpu->irq_count++;
        memcpy(&cpu->irq_handlers[desc->vector], &desc->handler, sizeof(irq_handler_t));
    }
    else
    {
        smp_arch_call_function(cpu->logical_id, cpu_inc_register_int, (uintptr_t)desc);
    }

    return KERRNO_SUCCESSES;
}

irq_handler_t *irq_arch_get_handler(interrupt_vector_t vector)
{
    cpu_t *entry = cpu_arch_get_current();
    return &entry->irq_handlers[vector];
}

INTERNAL status_t irq_internal_pick_free(kernel_irq_t allowed_mask, irq_source_t source, cpu_t *cpu, kernel_irq_t *out)
{
    for (kernel_irq_t irq = 0; irq < MAX_IRQ_VECTORS - IRQ_BASE; irq++)
    {
        // skip if not in allowed set
        if (!(allowed_mask & (1u << irq)))
        {
            continue;
        }

        if (FLAG_GET(cpu->irq_handlers[irq].state, HANDLER_RESERVED))
        {
            continue;
        }

        if (FLAG_IS_SET(cpu->irq_handlers[irq].state, HANDLER_IN_USE) && source == IRQ_SOURCE_MSI && !FLAG_GET(cpu->irq_handlers[irq].state, HANDLER_MSI))
        {
            continue;
        }

        if (FLAG_IS_SET(cpu->irq_handlers[irq].state, HANDLER_IN_USE))
        {
            continue;
        }

        trace_debug(MODULE, "cpu%u->handler[%u].state = 0x%x", cpu->arch_id, irq, cpu->irq_handlers[irq].state);
        trace_debug(MODULE, "irq%u is free to take", irq);
        FLAG_SET(cpu->irq_handlers[irq].state, HANDLER_RESERVED);
        *out = irq;
        return KERRNO_SUCCESSES;
    }
    return KERRNO_UNSUCCESS;
}

status_t irq_arch_pick_free_entry(kernel_irq_t allowed_mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *output)
{
    ENTER_FUNC("0x%x, %u, 0x%x, %p", allowed_mask, source, target, output);
    cpu_entry_t *entry = NULL;
    if (target == ANY_CPU)
    {
        uint32_t lowest_irq_count = UINT32_MAX;
        for (size_t i = 0; i < cpu_get_count(); i++)
        {
            entry = cpu_get_entry(i);
            // trace_debug(MODULE, "lowest_irq_count = %u = %u lcpu = %u", lowest_irq_count, entry->cpu->irq_count, i);
            // trace_debug(MODULE, "irq_count(%u) < lowest_irq_count(%u)", entry->cpu->irq_count, lowest_irq_count);
            if (entry->cpu->irq_count < lowest_irq_count)
            {
                target = entry->cpu->logical_id;
                lowest_irq_count = min(entry->cpu->irq_count, lowest_irq_count);
            }
        }
    }
    trace_debug(MODULE, "target = %i", target);
    trace_debug(MODULE, "entry @ %p", entry);
    entry = cpu_get_entry(target);
    trace_debug(MODULE, "entry->cpu @ %p", entry->cpu);
    cpu_t *cpu = entry->cpu;
    return irq_internal_pick_free(allowed_mask, source, entry->cpu, output);
    // no free IRQ found — should not happen on sane hardware
    KERNEL_PANIC(MODULE, "pick_free_irq: no free IRQ in allowed set 0x%X", allowed_mask);
    return 0xFF;
}
