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

#include "asm/smp_arch.h"
#include "asm/mmu_arch.h"

#include "cpu/cpu.h"

#include "mm/mmu.h"

#include "kerrno.h"

#include "memory.h"

#include <config.h>

// extern void hexdump(void *ptr, size_t len, size_t size);

status_t smp_init(boot_params_t *bp)
{
    vaddr_t trampoline = bp->smp.trampoline_phys_address;
    mmu_map_region(&kernel_page, trampoline, trampoline, bp->smp.trampoline_size, kernel_text_flags);

    memcpy((void*)trampoline, bp->smp.core_bringup, sizeof(bp->smp.core_bringup));

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

    return KERRNO_SUCCESSES;
}