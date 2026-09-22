/*
 * File: smp.c
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "smp/smp.h"

#include "asm/cpu_arch.h"
#include "asm/smp_arch.h"
#include "asm/mmu_arch.h"
#include "asm/vectors_arch.h"

#include "ivt/ivt.h"

#include "cpu/cpu.h"

#include "mm/mmu.h"

#include "kerrno.h"

#include "memory.h"

#include <config.h>

#define MODULE "smp"

// extern void hexdump(void *ptr, size_t len, size_t size);

status_t smp_ipi_call_function_handler(intr_frame_t *frame)
{
    ENTER_FUNC("%p", frame);
    // ivt_dump_frame(frame);
    cpu_t *cpu = cpu_arch_get_current();

    cpu->func_pending(cpu->func_arg_pending);
    cpu->func_pending = NULL;
    // ivt_dump_frame(frame);
    return KERRNO_SUCCESSES;
}

status_t smp_init(boot_params_t *bp)
{
    vaddr_t trampoline = bp->smp.trampoline_phys_address;
    mmu_map_region(&kernel_page, trampoline, trampoline, bp->smp.trampoline_size, kernel_text_flags);

    memcpy((void *)trampoline, bp->smp.core_bringup, sizeof(bp->smp.core_bringup));

    for (size_t i = 0; i < CONFIG_MAX_CPUS; i++)
    {
        cpu_entry_t *entry = cpu_get_entry(i);
        if (entry == NULL)
        {
            break;
        }
        if (entry->is_bsp)
        {
            continue;
        }
        smp_arch_boot_ap(entry, trampoline);
    }

    cpu_arch_init_bsp();

    ivt_set_handler(IPI_CALL_FUNCTION_VECTOR, smp_ipi_call_function_handler);

    return KERRNO_SUCCESSES;
}

status_t smp_call_function(cpu_logical_id_t id, void (*func)(uintptr_t), uintptr_t arg)
{
    cpu_entry_t *entry = cpu_get_entry(id);
    cpu_t *cpu = entry->cpu;

    cpu->func_pending = func;
    cpu->func_arg_pending = arg;
    return smp_arch_send_ipi(entry->arch_id, IPI_CALL_FUNCTION_VECTOR);
}
