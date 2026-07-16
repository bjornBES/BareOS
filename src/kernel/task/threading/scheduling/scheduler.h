/*
 * File: scheduler.h
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "task/threading/thread.h"

thread_t *scheduler_get_current();

void scheduler_init(thread_t *main_thread);
void scheduler_thread_info();
void scheduler_thread_exit();

void scheduler_sleep(uint64_t ns);
void scheduler_sleep_ms(uint64_t ms);
void scheduler_sleep_sec(time_t sec);

void scheduler_remove(thread_t *thread);
void scheduler_add(thread_t *thread);
void scheduler_block(thread_t *thread);
void scheduler_unblock(thread_t *thread);

thread_t *scheduler_find_waiting(process_t *proc);

int scheduler_yield();

int schedule(intr_frame_t *regs);