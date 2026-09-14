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

#include "debug/debug.h"

#include "string.h"

#include <config.h>

#define MODULE "SMP"

int cpu_count = 0;
cpu_entry_t entries[CONFIG_MAX_CPUS];

int cpu_register(uint32_t processor_uid, uint32_t arch_id, bool is_bsp)
{
    ENTER_FUNC("%u, %u, %s", processor_uid, arch_id, is_bsp BOOL_TO_STRING)
    entries[cpu_count].logical_id = cpu_count;
    entries[cpu_count].processor_uid = processor_uid;
    entries[cpu_count].arch_id = arch_id;
    entries[cpu_count].is_bsp = is_bsp;
    if (is_bsp == false)
    {
        entries[cpu_count].state = CPU_STATE_OFFLINE;
    }
    else
    {
        entries[cpu_count].state = CPU_STATE_ONLINE;
    }
    cpu_count++;
    
    return 0;
}