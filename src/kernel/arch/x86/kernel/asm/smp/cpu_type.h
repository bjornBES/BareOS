/*
 * File: cpu_type.h
 * File Created: 08 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "cpu_config.h"
#include "task/threading/thread_type.h"
#include "task/threading/scheduling/sched_algorithm.h"
#include "kernel/asm/task/tss.h"
#include "kernel/asm/segment/gdt.h"
#include "kernel/asm/acpi/apic/lapic.h"
#include <lists/list.h>
#include <types.h>


// typedef struct thread thread_t;

typedef struct arch_cpu_info
{
    struct arch_cpu_info *self;

    // per-core kernel stack (used during interrupt entry)
    vaddr_t user_stack;
    vaddr_t kernel_stack;

    // scheduler
    struct thread *current; // thread_t running on this core
    struct thread *idle;    // this core's idle thread_t

    lapic_id apic_id;       // this core's LAPIC ID
    cpu_id cpu_id;          // sequential index 0..n
    bool online;            // has this AP finished init
    bool need_resched;
    bool has_stopped;

    spinlock_t local_runq_lock;
    sched_class_t *sched_class; // which algorithm this core (or system) uses
    void *runq_data;                  // opaque — algo-specific struct, cast internally
    // int local_count;
    device_t *lapic_timer_dev;

    // calling functions using IPI
    void (*func_pending)(void*);
    void *func_arg_pending;

    // per-core TSS (needed so rsp0 is independent per core)
    tss_t tss;

    // per-core GDT (needed to hold the TSS descriptor)
    gdt_entry_t gdt_table[GDT_ENTRIES];
    gdtr_t gdtr;

} cpu_t;

