/*
 * File: lapci.c
 * File Created: 09 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "lapic.h"

void lapic_enable()
{
    // enable local APIC, set spurious vector to 0xFF
    lapic_write(LAPIC_REG_SPURIOUS, 
        lapic_read(LAPIC_REG_SPURIOUS) | LAPIC_SPURIOUS_ENABLE | 0xFF);
}

void lapic_wait_idle()
{
    while (lapic_read(LAPIC_REG_ICR_LOW) & (1 << 12))
        ;
}

lapic_id lapic_get_id()
{
    return (lapic_id)(lapic_read(LAPIC_REG_ID) >> 24);
}

void lapic_timer_init()
{
    // set divide config to 16
    lapic_write(LAPIC_REG_TIMER_DIVIDE, 0x3);

    // set timer to periodic mode, vector 0x22
    lapic_write(LAPIC_REG_TIMER, LAPIC_TIMER_PERIODIC | LAPIC_TIMER_VECTOR);

    // set initial count — tune this value for your tick rate
    // with divide=16 and ~1GHz LAPIC clock, ~62500 gives ~1ms
    lapic_write(LAPIC_REG_TIMER_INITIAL, 62500);
}