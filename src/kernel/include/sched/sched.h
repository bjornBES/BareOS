/*
 * File: sched.h
 * File Created: 19 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "thread/threadt.h"

#include "asm/cput_arch.h"

#include <types.h>

status_t sched_init(thread_t *main_thread);

status_t schedule(intr_frame_t *regs);

status_t sched_add(thread_t *t);

status_t sched_yield();
void sched_thread_exit(uintptr_t ret);
status_t sched_has_work(cpu_t *cpu);
