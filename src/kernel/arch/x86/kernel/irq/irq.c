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
#include "kernel/irq/irq.h"

#include <defs.h>

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
