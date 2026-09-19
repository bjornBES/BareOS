/*
 * File: cpu_arch.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "cput_arch.h"

#include "cpu/cpu.h"

#define THISCPU() cpu_arch_get_current()

status_t cpu_create(cpu_entry_t *entry);
cpu_t *cpu_arch_get_bsp();
cpu_t *cpu_arch_get_current();
cpu_t *cpu_arch_get(cpu_logical_id_t logical_id);
void cpu_arch_set_kernel_stack(cpu_t *cpu, vaddr_t stack_top);
