/*
 * File: CPU.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/asm/smp/cpu_type.h"

typedef struct arch_cpu_info cpu_t;

cpu_t *cpu_arch_get_current();
cpu_t *cpu_arch_get(cpu_id id);
void cpu_arch_set_kernel_stack(cpu_t *cpu, vaddr_t stack_top);