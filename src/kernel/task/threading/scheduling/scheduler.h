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
#include "sched_types.h"


thread_t *sched_get_current();

void sched_init(thread_t *main_thread);
void sched_thread_info();
void sched_thread_exit();

void sched_sleep(uint64_t ns);
void sched_sleep_ms(uint64_t ms);
void sched_sleep_sec(time_t sec);

void sched_remove(thread_t *thread);
void sched_add(thread_t *thread);
void sched_block(block_queue_t *queue);
void sched_wake_one(block_queue_t *queue);
void sched_wake_all(block_queue_t *queue);
void sched_unblock(thread_t *thread);

thread_t *sched_find_waiting(process_t *proc);

int sched_yield();

int schedule(intr_frame_t *regs);