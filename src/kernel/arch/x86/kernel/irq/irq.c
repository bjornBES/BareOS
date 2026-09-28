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

#include "kernel/irq/irq.h"
#include "kernel/acpi/apic/apic.h"

#include "irq/irq.h"

#include "init.h"

#include "kerrno.h"

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
