/*
 * File: threadt.h
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "thread/priority.h"

#include "asm/threadt_arch.h"
#include "asm/contextt_arch.h"

#include "stack_info/stack_info.h"

#include "lists/list.h"

#include <types.h>

typedef uintptr_t (*thread_func_t)(uintptr_t arg);

typedef enum
{
    THREAD_READY = 1,
    THREAD_RUNNING,
    THREAD_SLEEP,
    THREAD_DEAD,
    THREAD_BLOCKED,
    THREAD_JUST_WOKE,
    THREAD_REMAINS,
} thread_state_t;

typedef struct arch_cpu_info cpu_t;
typedef struct thread thread_t;

typedef struct thread
{
    list_node_t node; // intrusive — used by whichever queue currently
    // struct process *proc;

    // kernel stack
    vaddr_t kstack;
    stack_info_t kernel_stack;

    tid_t tid;
    char name[32];

    context_t ctx;

    thread_state_t state;

    priority_t priority;      // 0 = lowest
    uint32_t timeslice;       // ticks remaining this quantum
    uint32_t timeslice_reset; // what to reload when quantum expires

    uint64_t wake_time;

    uint8_t is_wait_thread : 1;
    uint8_t is_main_thread : 1;
    uint8_t is_kernel_thread : 1;
    uint8_t is_user_thread : 1;
    uint8_t is_idle_thread : 1;
    uint8_t in_queue : 1;
    uint8_t rev : 2;

    // signals
    sigset_t blocked_signals;
    // signal_pending signal_queue;

    cpu_t *last_cpu;     // cache-warmth hint, NULL if never run
    cpu_t *cpu_affinity; // explicit hint, NULL if none (free to roam)

    thread_arch_t arch;

    uintptr_t return_value;

    thread_t *next_runnable;
} thread_t;
