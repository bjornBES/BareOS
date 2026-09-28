/*
 * File: hpet.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "acpi/table.h"

#include "asm/irq_arch.h"
#include "irq/irq.h"

#include "debug/debug.h"
#include "dev/device.h"

#include "mm/memdefs.h"
#include "mm/ioremap.h"

#include "timer/timer.h"

#include "init.h"

#include "memory.h"
#include "math.h"

#include <binary.h>

#define MODULE           "x86-hpet"

#define HPET_REG_COUNTER 0xF0 // main counter value

extern vaddr_t hpet_base;

uint64_t hpet_arch_read(uint32_t reg)
{
    uint32_t l = *(volatile uint32_t *)(hpet_base + reg);
    uint32_t h = *(volatile uint32_t *)(hpet_base + reg + 4);
    return (uint64_t)h << 32 | l;
}

void hpet_arch_write(uint32_t reg, uint64_t value)
{
    *(volatile uint64_t *)(hpet_base + reg) = value;
}

uint64_t hpet_arch_read_counter()
{
    return hpet_arch_read(HPET_REG_COUNTER);
}
