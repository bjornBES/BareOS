/*
 * File: smp.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "kernel/cpu.h"

// init — called after MADT parsed and arch registered CPUs
void smp_init();

// called by each AP once it is up and running
void smp_ap_online();

// per CPU data
cpu_t *smp_get_cpu(cpu_id id);
cpu_t *smp_current_cpu();
uint32_t smp_cpu_count();

// IPI vectors — kernel defines what they mean
#define IPI_VECTOR_RESCHEDULE 0xF0
#define IPI_VECTOR_TLB_FLUSH 0xF1
#define IPI_VECTOR_PANIC 0xF2

void smp_send_reschedule(cpu_id id);
void smp_send_tlb_flush(cpu_id id, vaddr_t addr);
void smp_broadcast_panic();
