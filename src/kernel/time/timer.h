/*
 * File: timer.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/timer.h"
#include "kernel.h"
#include "device/device.h"

#include <boot/bootparams.h>

#define MAX_TIMERS 8

#define MS_TO_NS(ms) (ms * 1000000)
#define SEC_TO_NS(sec) (sec * 1000000000ull)
#define NS_TO_MS(ns) (ns / 1000000)
#define NS_TO_SEC(ns) (ns / 1000000000ull)

typedef enum
{
    TIMER_ROLE_SOURCE = (1 << 0), // can read current time
    TIMER_ROLE_EVENT = (1 << 1),  // can fire interrupts
    TIMER_ROLE_BOTH = (TIMER_ROLE_SOURCE | TIMER_ROLE_EVENT),
} timer_role_t;

typedef struct timer_priv
{
    const char *name;
    uint8_t rating;  // higher = preferred
    timer_role_t role;
    bool is_stable;  // TSC unstable on some CPUs
    bool is_per_cpu; // LAPIC timer is per CPU

    // clock source ops — NULL if TIMER_ROLE_EVENT only
    uint64_t (*read_counter)(device_t *dev);
    uint64_t (*counter_freq)(device_t *dev); // Hz

    // clock event ops — NULL if TIMER_ROLE_SOURCE only
    int (*set_oneshot)(device_t *dev, uint64_t ns, void (*cb)(device_t *));
    int (*set_periodic)(device_t *dev, uint64_t ns, void (*cb)(device_t *));
    void (*cancel)(device_t *dev);
} timer_priv_t;

// kernel picks best source/event device automatically
void timer_init(boot_params_t *bp);

// registration — called by arch drivers
void timer_register(device_t *dev);
void timer_unregister(device_t *dev);

// clock source API — what scheduler/kernel uses
uint64_t timer_now_ns();        // nanoseconds since kernel boot
uint64_t timer_get_boot_time(); // nanoseconds since stage2 boot
uint64_t timer_now_ticks();     // raw ticks of current source

void timer_sleep_ns(uint64_t ns);
void timer_sleep_ms(uint64_t ms);
void timer_sleep_sec(time_t sec);

// clock event API — what scheduler uses
void timer_set_oneshot(uint64_t ns, void (*cb)(void));
void timer_cancel();

// current active devices
device_t *timer_get_source();
device_t *timer_get_event();
