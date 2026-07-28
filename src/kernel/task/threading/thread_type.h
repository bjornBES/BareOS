/*
 * File: thread_type.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "thread_config.h"
#include "kernel/ctx.h"
// #include "kernel/cpu.h"
#include "signal/signal_type.h"
#include <lists/list.h>

typedef enum
{
    THREAD_READY = 1,
    THREAD_RUNNING,
    THREAD_SLEEP,
    THREAD_DEAD,
    THREAD_BLOCKED,
    THREAD_JUST_WOKE,
    THREAD_REMAINS,
} thread_state;

typedef struct thread
{
    struct process *proc;

    // kernel stack
    vaddr_t kernel_stack;
    vaddr_t raw_kernel_stack;
    size_t stack_size; // stack_top = kernel_stack + stack_size

    uint64_t fs_base;
    uint64_t gs_base;

    tid_t tid;
    char name[32];

    cpu_ctx_t ctx;

    thread_state state;
    uint8_t priority;         // 0 = lowest
    uint32_t timeslice;       // ticks remaining this quantum
    uint32_t timeslice_reset; // what to reload when quantum expires
    uint64_t wake_time;

    // sys
    int *clear_child_tid;

    // signals
    sigset_t blocked_signals;
    signal_pending signal_queue;

    struct arch_cpu_info *last_cpu;     // cache-warmth hint, NULL if never run
    struct arch_cpu_info *cpu_affinity; // explicit hint, NULL if none (free to roam)
    list_node_t node;                   // intrusive — used by whichever queue currently

    struct thread *next_runnable;
} thread_t;
