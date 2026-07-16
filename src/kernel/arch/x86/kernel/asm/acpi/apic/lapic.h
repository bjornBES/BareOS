/*
 * File: lapci.h
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "apic.h"
#include "apic_type.h"
#include "kernel.h"

#define LAPIC_REG_ID 0x20
#define LAPIC_REG_EOI 0xB0
#define LAPIC_REG_SPURIOUS 0xF0
#define LAPIC_REG_ICR_LOW 0x300
#define LAPIC_REG_ICR_HIGH 0x310
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_TIMER_INITIAL 0x380
#define LAPIC_REG_TIMER_CURRENT 0x390
#define LAPIC_REG_TIMER_DIVIDE 0x3E0

#define LAPIC_SPURIOUS_ENABLE (1 << 8)
#define LAPIC_TIMER_PERIODIC (1 << 17)
#define LAPIC_TIMER_VECTOR 0x22 // same vector as your PIT/HPET timer IRQ

typedef uint16_t lapic_id;

extern apic_priv_t *priv;

static inline void lapic_write(uint32_t reg, uint32_t value)
{
    *(volatile uint32_t *)(priv->local_apic_base + reg) = value;
}

static inline uint32_t lapic_read(uint32_t reg)
{
    return *(volatile uint32_t *)(priv->local_apic_base + reg);
}

void lapic_enable();
void lapic_wait_idle();
lapic_id lapic_get_id();
void lapic_timer_init();