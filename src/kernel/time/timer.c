/*
 * File: timer.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "timer.h"

#include "debug/debug.h"

static const timer_driver *current_driver = NULL;

#if __x86_64__ || __i686__
#include "kernel/pit.h"
#include "kernel/hpet.h"

static const timer_driver pit_driver = {
    .name = "PIT",
    .probe = NULL,
    .init = x86_pit_init,
    .get_ticks = x86_pit_get_ticks,
    .sleep_ms = NULL,
};

static const timer_driver hpet_driver = {
    .name = "HPET",
    .probe = HPET_probe,
    .init = HPET_init,
    .get_ticks = HPET_get_ticks,
    .sleep_ms = HPET_sleep_ms,
    .elapsed_ms = HPET_elapsed_ms,
};
#endif

void timer_init()
{
    current_driver = &pit_driver;
    current_driver->init(100);

#if __x86_64__ || __i686__
    current_driver = &hpet_driver;
    if (current_driver->probe())
    {
        current_driver->init(100);
    }
    else
    {
        current_driver = &pit_driver;
    }
#endif

    log_info("TIMER", "using %s", current_driver->name);
}

uint64_t timer_get_ticks(void)
{
    if (!current_driver)
        return 0;
    return current_driver->get_ticks();
}

void timer_sleep_ms(uint64_t ms)
{
    if (!current_driver || !current_driver->sleep_ms)
        return;
    current_driver->sleep_ms(ms);
}

time_t timer_elapsed_time(time_t start)
{
    if (!current_driver || !current_driver->elapsed_ms)
        return -1;
    return current_driver->elapsed_ms(start);
}
