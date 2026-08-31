/*
 * File: cpu.c
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/cpu_arch.h"
#include "cpu_config.h"

#include "debug/debug.h"

#define MODULE "x86-cpu"

cpu_t cpus[MAX_CPUS] = {0};
cpu_t *bsp_cpu = NULL;

cpu_t *cpu_arch_get_current()
{
    FUNC_NOT_IMPLEMENTED();
    return &cpus[0];
}

cpu_t *cpu_arch_get(cpu_id id)
{
    if (id < MAX_CPUS)
    {
        return &cpus[id];
    }
    return NULL;
}
