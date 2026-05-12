/*
 * File: thread_type.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "thread_config.h"

#include "kernel/threading/context.h"

typedef enum
{
    THREAD_READY = 1,
    THREAD_RUNNING,
    THREAD_SLEEP,
    THREAD_DEAD,
    THREAD_BLOCKED,
    THREAD_JUST_WOKE = 0xFFFE,
    THREAD_REMAINS = 0xFFFF,
} thread_state;

typedef struct
{
    context ctx;
    uint32_t tid;
    thread_state state;

    // kernel stack
    void *stack_base;
    size_t stack_size;  // stack_top = stack_base + stack_size

    time_t wake_time;

    // scheduler
    uint8_t priority;       // 0 = lowest
    uint32_t timeslice;     // ticks remaining this quantum
    uint32_t timeslice_reset; // what to reload when quantum expires

    struct process *proc;
} thread;