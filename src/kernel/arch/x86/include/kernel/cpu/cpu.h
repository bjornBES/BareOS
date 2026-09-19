/*
 * File: cpu.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/cpu_arch.h"

#include "entry/desc/gdt/gdt.h"
#include "entry/desc/tss/tss.h"

#include <types.h>

status_t cpu_init_ap(uint32_t apic_id, cpu_t *cpu);

cpu_t *cpu_arch_get(cpu_logical_id_t id);
