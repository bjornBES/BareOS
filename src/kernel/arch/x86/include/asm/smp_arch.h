/*
 * File: smp_arch.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#include "cpu/cpu.h"

void cpu_arch_init_bsp();
void smp_arch_boot_ap(cpu_entry_t *entry, vaddr_t trampoline);