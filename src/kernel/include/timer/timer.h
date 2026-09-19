/*
 * File: timer.h
 * File Created: 18 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "dev/device_id.h"

#include <types.h>

#define SEC_TO_NANOSEC(sec)        (sec * 1000000000ull)
#define SEC_TO_MICROSEC(sec)       (sec * 1000000ull)
#define SEC_TO_MILLISEC(sec)       (sec * 1000ull)

#define MILLISEC_TO_NANOSEC(ms)    (ms * 1000000ull)
#define MILLISEC_TO_MICROSEC(ms)   (ms * 1000ull)
#define MILLISEC_TO_SEC(ms)        (ms / 1000ull)

#define MICROSEC_TO_NANOSEC(usec)  (usec * 1000ull)
#define MICROSEC_TO_MILLISEC(usec) (usec / 1000ull)
#define MICROSEC_TO_SEC(usec)      (usec / 1000000ull)

#define NANOSEC_TO_MICROSEC(ns)    (ns / 1000ull)
#define NANOSEC_TO_MILLISEC(ns)    (ns / 1000000ull)
#define NANOSEC_TO_SEC(ns)         (ns / 1000000000ull)

/// @brief
typedef enum
{
    TIMER_ROLE_NONE,

    /// @brief sleep/timeout backing store for the timer wheel
    TIMER_ROLE_DEADLINE,

    /// @brief free-running elapsed-time reads (udelay, profiling)
    TIMER_ROLE_COUNTER,

    /// @brief per-CPU preemption tick
    TIMER_ROLE_TICK,
} timer_role_t;

/// @brief
typedef struct timer_caps
{
    uint8_t per_cpu;               /* 1 = independent instance per core (LAPIC-like) */
    uint8_t interrupt_capable : 1; /* 1 = can raise an interrupt on expiry, not read-only */
    uint8_t supports_oneshot : 1;
    uint8_t supports_periodic : 1;
    uint8_t invariant : 1; /* 1 = constant rate across P-states/C-states/sleep */
    uint8_t res : 3;

    uint32_t read_cost_ns;    /* rough cost of one read_counter() call, for selection heuristics */
    uint64_t freq_hz;         /* 0 until known/calibrated */
    uint64_t min_interval_ns; /* smallest interval arm_oneshot/arm_periodic can hit */
    uint64_t max_interval_ns; /* largest interval before the counter would wrap or need reprogram */
} timer_caps_t;

typedef struct timer_source timer_source_t;
typedef void (*timer_callback_t)(void *ctx);

typedef struct timer_source
{
    const char *name; /* debug/log only, never branched on by generic code */
    timer_caps_t caps;
    kernel_dev_t dev;
    void *priv;
    timer_role_t role;

    /// @brief
    uint64_t (*read_counter)(timer_source_t *self);
    /// @brief
    uint64_t (*ticks_to_ns)(timer_source_t *self, uint64_t ticks);

    /// @brief
    status_t (*arm_oneshot)(timer_source_t *self, uint64_t ns_from_now, timer_callback_t cb);

    /// @brief
    status_t (*arm_periodic)(timer_source_t *self, uint64_t period_ns, timer_callback_t cb);

    /// @brief
    status_t (*cancel)(timer_source_t *self);

    /// @brief
    status_t (*set_callback)(timer_source_t *self, timer_callback_t cb, void *ctx);
} timer_source_t;

timer_source_t *timer_create();

status_t timer_register(timer_source_t *src);

uint64_t timer_now_ns();        // nanoseconds since kernel boot
uint64_t timer_now_sec();       // seconds since kernel boot
uint64_t timer_get_boot_time(); // nanoseconds since stage2 boot
uint64_t timer_now_ticks();     // raw ticks of current source
