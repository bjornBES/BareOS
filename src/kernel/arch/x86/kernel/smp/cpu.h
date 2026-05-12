/*
 * File: cpu.h
 * File Created: 08 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "cpu_config.h"
#include "task/threading/thread_type.h"
#include "kernel/task/tss.h"
#include "kernel/gdt.h"
#include "kernel/apic/apic.h"
#include <types.h>

typedef struct cpu
{
    struct cpu *self;
    lapic_id apic_id; // this core's LAPIC ID
    cpu_id cpu_id;    // sequential index 0..n
    bool online;      // has this AP finished init

    // scheduler
    thread *current; // thread running on this core
    thread *idle;    // this core's idle thread

    // per-core kernel stack (used during interrupt entry)
    void *kernel_stack;
    size_t kernel_stack_size;

    // per-core TSS (needed so rsp0 is independent per core)
    tss_t tss;

    // per-core GDT (needed to hold the TSS descriptor)
    gdt_entry_t gdt[GDT_ENTRIES];
    gdtr_t gdtr;

} cpu_t;

extern cpu_t cpus[MAX_CPUS];
extern cpu_t *bsp_cpu;

cpu_t *cpu_get_current(); // reads APIC ID, returns matching cpu_t
cpu_t *cpu_get(cpu_id id);
void cpu_init_bsp();
void cpu_init_ap(lapic_id apic_id);
