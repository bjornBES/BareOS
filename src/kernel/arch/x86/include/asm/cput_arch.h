/*
 * File: cput_arch.h
 * File Created: 17 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "entry/desc/gdt/gdt.h"
#include "entry/desc/tss/tss.h"

#include "sync/spinlock.h"

#include "timer/timer.h"
#include "sched/sched_class.h"

#include "kernel/cpuid/types.h"

typedef struct arch_cpu_info
{
    struct arch_cpu_info *self;

    // per-core kernel stack (used during interrupt entry)
    vaddr_t user_stack;
    vaddr_t kernel_stack;

    // scheduler
    struct thread *current;      // thread_t running on this core
    struct thread *idle;         // this core's idle thread_t

    cpu_logical_id_t logical_id; // sequential index 0..n
    uint32_t arch_id;

    uint8_t online : 1; // has this AP finished init
    uint8_t need_resched : 1;
    uint8_t has_stopped : 1;
    uint8_t res : 5;

    cpuid_leaves_t cpuid;

    spinlock_t local_runq_lock;
    sched_class_t *sched_class; // which algorithm this core (or system) uses
    void *runq_data; // opaque — algo-specific struct, cast internally

    timer_source_t *cpu_timer_dev;

    // calling functions using IPI
    void (*func_pending)(uintptr_t);
    uintptr_t func_arg_pending;

    // per-core TSS (needed so rsp0 is independent per core)
    tss_entry_t tss;

    // per-core GDT (needed to hold the TSS descriptor)
    gdt_entry_t gdt_table[GDT_ENTRIES];
    gdtr_t gdtr;

} cpu_t;
