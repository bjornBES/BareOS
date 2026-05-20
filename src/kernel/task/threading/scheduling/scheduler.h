/*
 * File: scheduler.h
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "task/threading/thread.h"
#include "kernel/registers.h"

thread_t *scheduler_get_current();

void scheduler_init(thread_t *main_thread);
void scheduler_thread_info();
void scheduler_thread_exit();
void scheduler_remove(thread_t *thread_t);
void scheduler_add(thread_t *thread_t);
void scheduler_sleep_ms(time_t ms);
void scheduler_block(thread_t *t);
void scheduler_unblock(thread_t *t);
thread_t *scheduler_find_waiting(process_t *proc);
int scheduler_yield();
void schedule(registers *regs);