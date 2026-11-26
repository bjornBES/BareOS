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
#include "asm/msi_arch.h"
#include "asm/smp_arch.h"

#include "kernel/irq/irq.h"
#include "kernel/cpu/cpu.h"
#include "kernel/acpi/apic/apic.h"

#include "drivers/pci/msi/msi.h"

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
        if (status != KERRNO_SUCCESS)
        {
            trace_err(MODULE, "PIC time");
            goto pic_time; // sorry...
        }
        return KERRNO_SUCCESS;
    }
    else
    {
pic_time:
        FUNC_NOT_IMPLEMENTED();
    }
    KERNEL_PANIC(MODULE, "couldn't find an irq controller");
    return KERRNO_UNSUCCESS;
}

POSTCORE_INITCALL(irq_arch_initcall);

/// @brief 
/// @param cpu 
/// @param offset 
/// @param out_vector 
/// @return KERRNO_SUCCESS on successes or an kerrno number.
/// @retval KERRNO_IRQ_ALLOC_FAILED: If the allocation failed.
/// @retval KERRNO_DEVICE_FULL: If the given CPU doesn't have any empty entries.
INTERNAL status_t irq_arch_alloc_vector(cpu_t *cpu, interrupt_vector_t offset, interrupt_vector_t *out_vector)
{
    if (cpu->irq_count == MAX_IRQ_VECTORS)
    {
        KERRNO_RETURN(KERRNO_DEVICE_FULL);
    }
    for (interrupt_vector_t v = offset; v < MAX_IRQ_VECTORS - IRQ_BASE; v++)
    {
        if (!FLAG_GET(cpu->irq_handlers[v].state, HANDLER_IN_USE))
        {
            FLAG_SET(cpu->irq_handlers[v].state, HANDLER_IN_USE);
            *out_vector = v + IRQ_BASE;
            return KERRNO_SUCCESS;
        }
    }
    KERRNO_RETURN(KERRNO_IRQ_ALLOC_FAILED, "");
}

/// @brief 
/// @param target 
/// @param offset 
/// @param vector_out 
/// @param entry_out 
/// @return KERRNO_SUCCESS on successes or an kerrno number.
/// @retval KERRNO_IRQ_ALLOC_FAILED: If the allocation failed.
/// @retval KERRNO_DEVICE_FULL: If the given CPU doesn't have any empty entries.
INTERNAL status_t irq_arch_get_vector(cpu_logical_id_t target, interrupt_vector_t offset, interrupt_vector_t *vector_out, cpu_entry_t **entry_out)
{
    cpu_entry_t *entry = NULL;
    size_t cpu_count = cpu_get_count();

    // loop for all the CPUs to see which one has the lowest amount of IRQ's allocated.
    if (target == ANY_CPU)
    {
        uint32_t lowest_irq_count = UINT32_MAX;
        for (size_t i = 0; i < cpu_count; i++)
        {
            entry = cpu_get_entry(i);
            lowest_irq_count = min(entry->cpu->irq_count, lowest_irq_count);
            if (entry->cpu->irq_count == lowest_irq_count)
            {
                target = entry->logical_id;
            }
        }
    }

    // TODO make core specific IDT's
    entry = cpu_get_entry(0);
    cpu_t *cpu = entry->cpu;

    interrupt_vector_t vector;
    status_t ret = irq_arch_alloc_vector(cpu, offset, &vector);
    if (ret)
    {
        for (size_t i = 0; i < cpu_count; i++)
        {
            entry = cpu_get_entry(i);
            cpu = entry->cpu;
            ret = irq_arch_alloc_vector(cpu, offset, &vector);
            if (ret)
            {
                KERRNO_RETURN(KERRNO_IRQ_ALLOC_FAILED);
            }
        }
    }
    if (ret)
    {
        KERRNO_RETURN(KERRNO_DEVICE_FULL, "didn't find a free cpu handler");
    }
    *vector_out = vector;
    *entry_out = entry;
    return KERRNO_SUCCESS;
}

status_t irq_arch_register(irq_descriptor_t *desc, irq_controller_t *current, irq_source_t source, irq_handler_func_t handler, void *ctx,
                           irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target, cpu_logical_id_t *out_target)
{
    interrupt_vector_t vector;
    cpu_entry_t *entry;
    status_t ret;
    cpu_t *cpu;
    interrupt_vector_t offset = 0;
    do
    {
        ret = irq_arch_get_vector(target, offset, &vector, &entry);
        KERRNO_CHECK_AND_RETURN(ret);

        cpu = entry->cpu;

        if (source == IRQ_SOURCE_IRQ || source == IRQ_SOURCE_GSI)
        {
            ret = current->route(desc->id, vector, cpu->logical_id, trigger, polarity);
        }
        else
        {
            msi_disable(ctx);
            ret = msi_arch_set_vector(ctx, vector, entry->logical_id, trigger, polarity);
        }

        if (ret != KERRNO_SUCCESS)
        {
            if (offset == 0)
            {
                offset = vector - IRQ_BASE;
            }
            FLAG_UNSET(cpu->irq_handlers[vector - IRQ_BASE].state, HANDLER_IN_USE);
            offset++;
        }
    } while (ret != KERRNO_SUCCESS);

    *out_target = entry->logical_id;

    uint8_t handler_index = vector - IRQ_BASE;
    desc->vectors[cpu->logical_id] = handler_index;

    cpu->irq_handlers[handler_index].ctx = ctx;
    cpu->irq_handlers[handler_index].handler = handler;
    cpu->irq_handlers[handler_index].state = HANDLER_IN_USE;

    if (source == IRQ_SOURCE_MSI)
    {
        FLAG_SET(cpu->irq_handlers[handler_index].state, HANDLER_MSI);
        FLAG_SET(desc->flags, HANDLER_MSI);
    }
    FLAG_SET(desc->flags, HANDLER_IN_USE);

    cpu->irq_count++;

    if (source != IRQ_SOURCE_MSI)
    {
        gsi_t gsi = 0;
        current->irq_to_gsi(desc->id, &gsi);
        current->unmask(gsi);
    }

    if (source == IRQ_SOURCE_MSI)
    {
        pci_device_t *pci_dev = ctx;
        pci_dev->irq = vector;
        msi_enable(ctx);
    }

    return KERRNO_SUCCESS;
}

irq_handler_t *irq_arch_get_handler(interrupt_vector_t vector)
{
    cpu_t *entry = cpu_arch_get_current();
    return &entry->irq_handlers[vector];
}

