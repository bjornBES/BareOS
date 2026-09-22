/*
 * File: timer.c
 * File Created: 18 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "timer/timer.h"

#include "kerrno.h"

#include "memory.h"

#include "math.h"

#include <defs.h>

#define MAX_TIMERS 8

#define MODULE     "timer"

typedef int8_t timer_idx_t;

timer_source_t *timers[MAX_TIMERS] = {0};
uint32_t timer_count = 0;

timer_source_t *cached_tick_source = NULL;
timer_source_t *cached_deadline_source = NULL;
timer_source_t *cached_counter_source = NULL;

timer_source_t *timer_create()
{
    return kmalloc(sizeof(timer_source_t));
}

INTERNAL timer_role_t timer_caps_to_role(timer_caps_t *caps)
{
    timer_role_t role = 0;
    if (caps->per_cpu && caps->interrupt_capable && caps->supports_periodic)
    {
        return TIMER_ROLE_TICK;
    }
    if (caps->interrupt_capable && caps->supports_oneshot)
    {
        role |= TIMER_ROLE_DEADLINE;
    }
    if (caps->invariant)
    {
        role |= TIMER_ROLE_COUNTER;
    }
    return role;
}

INTERNAL timer_source_t *timer_select(timer_role_t role)
{
    timer_source_t *priv_src = NULL;
    switch (role)
    {
        case TIMER_ROLE_COUNTER :
            priv_src = cached_counter_source;
            break;
        case TIMER_ROLE_DEADLINE :
            priv_src = cached_deadline_source;
            break;
        case TIMER_ROLE_TICK :
            priv_src = cached_tick_source;
            break;
        default :
            KERRNO_RETURN_TYPE(KERRNO_BAD_VALUE, timer_source_t *, "%u is not a valid role", role);
    }

    uint32_t lowest_read_cost = UINT32_MAX;
    timer_idx_t lowest_read_cost_index = -1;
    uint64_t lowest_min_interval = UINT64_MAX;
    timer_idx_t lowest_min_interval_index = -1;
    for (timer_idx_t i = 0; i < timer_count; i++)
    {
        timer_source_t *src = timers[i];
        if (src == NULL || src == priv_src)
        {
            continue;
        }
        if (src->role != role)
        {
            continue;
        }

        if (role == TIMER_ROLE_DEADLINE)
        {
            lowest_min_interval = min(lowest_min_interval, src->caps.min_interval_ns);
            lowest_min_interval_index = i;
        }
        lowest_read_cost = min(lowest_read_cost, src->caps.read_cost_ns);
        lowest_read_cost_index = i;
    }

    timer_source_t *new_best = NULL;
    if (role == TIMER_ROLE_DEADLINE && priv_src != NULL)
    {
        if (lowest_min_interval_index == -1)
        {
            KERRNO_RETURN_TYPE(KERRNO_BAD_INDEX, timer_source_t *, "Couldn't find any entry with role TIMER_ROLE_DEADLINE");
        }
        if (lowest_min_interval != priv_src->caps.min_interval_ns)
        {
            if (min(lowest_min_interval, priv_src->caps.min_interval_ns) == lowest_min_interval)
            {
                new_best = timers[lowest_min_interval_index];
            }
            else
            {
                new_best = priv_src;
            }
        }
    }
    else if (priv_src == NULL)
    {
        new_best = timers[lowest_min_interval_index];
    }

    if (priv_src != NULL || (role == TIMER_ROLE_DEADLINE && new_best == NULL))
    {
        if (lowest_read_cost_index == -1)
        {
            KERRNO_RETURN_TYPE(KERRNO_BAD_INDEX, timer_source_t *, "Couldn't find any entry with role %u", role);
        }

        if (min(lowest_read_cost, priv_src->caps.read_cost_ns) == lowest_read_cost)
        {
            new_best = timers[lowest_read_cost_index];
        }
        else
        {
            new_best = priv_src;
        }
    }
    else if (priv_src == NULL)
    {
        new_best = timers[lowest_read_cost_index];
    }

    if (new_best == NULL)
    {
        KERRNO_NO_RETURN(KERRNO_PERMISSION_DENIED, "Everything is NULL how? idk... this should be panic");
        KERNEL_PANIC(MODULE, "and yes it is");
    }

    return new_best;
}

status_t timer_register(timer_source_t *src)
{
    timers[timer_count] = src;
    timer_count++;

    timer_role_t role = timer_caps_to_role(&src->caps);
    src->role = role;

    trace_info(MODULE, "registered %s (role=%d)", src->name, src->role);

    timer_source_t *timer_src = timer_select(role);
    if (timer_src < (timer_source_t *)KERRNO_ERRORS_END)
    {
        KERRNO_RETURN(KERRNO_CANCELED, "timer_select returned a non-valid value");
    }
    switch (role)
    {
        case TIMER_ROLE_COUNTER :
            if (cached_counter_source != timer_src)
            {
                trace_info(MODULE, "%s is now the new best counter source", timer_src->name);
            }
            cached_counter_source = timer_src;
            break;
        case TIMER_ROLE_DEADLINE :
            if (cached_deadline_source != timer_src)
            {
                trace_info(MODULE, "%s is now the new best deadline source", timer_src->name);
            }
            cached_deadline_source = timer_src;
            break;
        case TIMER_ROLE_TICK :
            if (cached_tick_source != timer_src)
            {
                trace_info(MODULE, "%s is now the new best tick source", timer_src->name);
            }
            cached_tick_source = timer_src;
            break;
        default :
            KERRNO_RETURN(KERRNO_BAD_VALUE, "%u is not a valid role", role);
    }
    return KERRNO_SUCCESSES;
}

uint64_t timer_now_ns()
{
    if (cached_counter_source == NULL)
    {
        return 0;
    }
    uint64_t ticks = cached_counter_source->read_counter(cached_counter_source);
    return cached_counter_source->ticks_to_ns(cached_counter_source, ticks);
}

/* uint64_t timer_now_sec()
{

}

uint64_t timer_get_boot_time()
{

}

*/
uint64_t timer_now_ticks()
{
    return cached_counter_source->read_counter(cached_counter_source);
}


void timer_set_device_periodic_wrapper(uintptr_t _args)
{
    ENTER_FUNC("%p", _args);
    periodic_function_args_t *args = (periodic_function_args_t*)_args;
    trace_debug(MODULE, "source = %p", args->source);
    trace_debug(MODULE, "source->name = %s", args->source->name);
    trace_debug(MODULE, "source->arm_periodic = %p", args->source->arm_periodic);
    args->source->arm_periodic(args->source, args->ns, args->cb);
}
