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

#include "debug/debug.h"

#include <config.h>

#define MODULE "x86-cpu"

cpu_t cpus[CONFIG_MAX_CPUS] = {0};
cpu_t *bsp_cpu = NULL;

cpu_t *cpu_arch_get_current()
{
    FUNC_NOT_IMPLEMENTED();
    return &cpus[0];
}

cpu_t *cpu_arch_get(uint32_t id)
{
    if (id < CONFIG_MAX_CPUS)
    {
        return &cpus[id];
    }
    return NULL;
}
