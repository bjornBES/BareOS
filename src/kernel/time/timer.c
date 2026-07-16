/*
 * File: timer.c
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "timer.h"
#include "debug/debug.h"

#include "kernel/memory.h"
#include <boot/bootparams.h>

#define MODULE "TIMER"

static uint64_t boot_unix_ns = 0; // set from RTC at boot
static uint64_t boot_ns = 0;      // timer_now_ns() at boot moment

static device_t *timers[MAX_TIMERS];
static uint32_t timer_count = 0;
static device_t *active_source = NULL;
static device_t *active_event = NULL;

void timer_init(boot_params_t *bp)
{
    log_debug(MODULE, "timer_init(%p)", bp);
    boot_unix_ns = SEC_TO_NS(bp->boot_time);
    // pick best source
    log_debug(MODULE, "timer_count = %u", timer_count);
    for (uint32_t i = 0; i < timer_count; i++)
    {
        timer_priv_t *p = (timer_priv_t *)timers[i]->priv;
        if (!(p->role & TIMER_ROLE_SOURCE))
        {
            continue;
        }
        if (!p->is_stable)
        {
            continue;
        }
        if (!active_source)
        {
            active_source = timers[i];
            continue;
        }
        timer_priv_t *best = (timer_priv_t *)active_source->priv;
        if (p->rating > best->rating)
        {
            active_source = timers[i];
        }
    }

    if (active_source != NULL)
    {
        log_info(MODULE, "found %s as source timer", active_source->name);
    }
    else
    {
        log_warn(MODULE, "didn't find any source timers");
    }

    // pick best event
    for (uint32_t i = 0; i < timer_count; i++)
    {
        timer_priv_t *p = (timer_priv_t *)timers[i]->priv;
        if (!(p->role & TIMER_ROLE_EVENT))
        {
            continue;
        }
        if (!active_event)
        {
            active_event = timers[i];
            continue;
        }
        timer_priv_t *best = (timer_priv_t *)active_event->priv;
        if (p->rating > best->rating)
        {
            active_event = timers[i];
        }
    }

    if (active_event != NULL)
    {
        log_info(MODULE, "found %s as event timer", active_event->name);
    }

    if (!active_source)
    {
        panic(MODULE, __FILE__, __LINE__, "no stable clock source found");
    }
    if (!active_event)
    {
        panic(MODULE, __FILE__, __LINE__, "no clock event device found");
    }

    boot_ns = timer_now_ns();
    log_info(MODULE, "boot time is %u", NS_TO_MS(boot_ns));

    log_info("TIMER", "source: %s event: %s",
             ((timer_priv_t *)active_source->priv)->name,
             ((timer_priv_t *)active_event->priv)->name);
}

void timer_register(device_t *dev)
{
    if (timer_count >= MAX_TIMERS)
    {
        return;
    }
    timers[timer_count++] = dev;
}

uint64_t timer_now_ns()
{
    timer_priv_t *p = (timer_priv_t *)active_source->priv;
    if (p->read_counter == NULL || p->counter_freq == NULL)
    {
        log_err(MODULE, "timer (%s) doesn't have timer_now functions", active_source->name);
        return 0;
    }
    uint64_t ticks = p->read_counter(active_source);
    uint64_t freq = p->counter_freq(active_source);
    return SEC_TO_NS(ticks) / freq;
}

void timer_sleep_ns(uint64_t ns)
{
    if (active_source == NULL)
    {
        for (size_t i = 0; i < ns * 10000; i++)
        {
        }
    }
    else
    {
        timer_priv_t *p = (timer_priv_t *)active_source->priv;
        if (p == NULL || p->read_counter == NULL || p->counter_freq == NULL)
        {
            log_err(MODULE, "timer (%s) doesn't have timer_now functions", active_source->name);
            for (size_t i = 0; i < ns * 10000; i++)
            {
            }
            return;
        }
        uint64_t now = timer_now_ns();
        while (now < timer_now_ns())
        {
            ;
        }
    }
}

void timer_sleep_ms(uint64_t ms)
{
    timer_sleep_ns(MS_TO_NS(ms));
}

void timer_sleep_sec(time_t sec)
{
    timer_sleep_ns(SEC_TO_NS(sec));
}

uint64_t timer_get_boot_time()
{
    return boot_unix_ns + (timer_now_ns() - boot_ns);
}

void timer_set_oneshot(uint64_t ns, void (*cb)(void))
{
    timer_priv_t *p = (timer_priv_t *)active_event->priv;
    p->set_oneshot(active_event, ns, (void (*)(device_t *))cb);
}
