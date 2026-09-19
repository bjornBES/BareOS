/*
 * File: smp.c
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "cpu/cpu.h"
#include "asm/cpu_arch.h"

#include "debug/debug.h"

#include "string.h"

#include <config.h>

#define MODULE "SMP"

cpu_logical_id_t cpu_count = 0;
cpu_logical_id_t bsp_index = 0;
cpu_entry_t entries[CONFIG_MAX_CPUS];

status_t cpu_register(uint32_t processor_uid, uint32_t arch_id, bool is_bsp)
{
    if (cpu_count == CONFIG_MAX_CPUS)
    {
        KERRNO_RETURN(KERRNO_BAD_INDEX, "Can't support anymore cpus");
    }
    ENTER_FUNC("%u, %u, %s", processor_uid, arch_id, is_bsp BOOL_TO_STRING)
    entries[cpu_count].logical_id = cpu_count;
    entries[cpu_count].processor_uid = processor_uid;
    entries[cpu_count].arch_id = arch_id;
    entries[cpu_count].is_bsp = is_bsp;
    cpu_create(&entries[cpu_count]);
    if (is_bsp == false)
    {
        entries[cpu_count].state = CPU_STATE_OFFLINE;
    }
    else
    {
        entries[cpu_count].state = CPU_STATE_ONLINE;
        bsp_index = cpu_count;
    }
    cpu_count++;
    
    return KERRNO_SUCCESSES;
}

cpu_entry_t *cpu_get_entry(cpu_logical_id_t id)
{
    if (id >= cpu_count)
    {
        return NULL;
    }
    return &entries[id];
}

cpu_entry_t *cpu_get_bsp()
{
    return &entries[bsp_index];
}