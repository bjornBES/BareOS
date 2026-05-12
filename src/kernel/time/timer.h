/*
 * File: timer.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel.h"
#include <stdbool.h>

typedef struct {
    const char *name;
    bool     (*probe)();
    void     (*init)(uint32_t hz);
    uint64_t (*get_ticks)();
    void     (*sleep_ms)(time_t ms);
    time_t   (*elapsed_ms)(time_t ms);
} timer_driver;

void timer_sleep_ms(uint64_t ms);
uint64_t timer_get_ticks(void);
time_t timer_elapsed_time(time_t start);
void timer_init();