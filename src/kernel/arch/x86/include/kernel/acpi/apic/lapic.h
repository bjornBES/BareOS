/*
 * File: lapic.h
 * File Created: 14 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 14 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include <defs.h>

#define LAPIC_REG_ID 0x20
#define LAPIC_REG_EOI 0xB0
#define LAPIC_REG_SPURIOUS 0xF0
#define LAPIC_REG_ICR_LOW 0x300
#define LAPIC_REG_ICR_HIGH 0x310
#define LAPIC_REG_ICR 0x330
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_TIMER_INITIAL 0x380
#define LAPIC_REG_TIMER_CURRENT 0x390
#define LAPIC_REG_TIMER_DIVIDE 0x3E0

#define LAPIC_SPURIOUS_ENABLE (1 << 8)
#define LAPIC_TIMER_PERIODIC (1 << 17)
#define LAPIC_TIMER_MASKED (1 << 16)

extern vaddr_t local_apic_base;

void lapic_write_icr(uint32_t high, uint32_t low);
void lapic_write(uint32_t reg, uint64_t value);
uint64_t lapic_read(uint32_t reg);
uint32_t lapic_get_id();

void lapic_enable();
void lapic_wait_idle();

status_t lapic_timer_init(uint32_t lapic_id, cpu_logical_id_t logical_id);