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
void cpu_arch_set_kernel_stack(vaddr_t stack_top);