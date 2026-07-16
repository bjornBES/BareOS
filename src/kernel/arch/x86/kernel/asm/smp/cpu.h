/*
 * File: cpu.h
 * File Created: 08 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "cpu_config.h"
#include "cpu_type.h"
#include "task/threading/thread_type.h"
#include "kernel/asm/task/tss.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/acpi/acpi.h"
#include <types.h>

extern cpu_t cpus[MAX_CPUS];
extern cpu_t *bsp_cpu;

cpu_t *cpu_get(cpu_id id);
void cpu_init_bsp();
void cpu_init_ap(lapic_id apic_id);
