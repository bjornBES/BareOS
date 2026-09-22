/*
 * File: cpu_arch.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

/// @defgroup contract_arch_x86_cpu

/// @ingroup contract_arch_x86_cpu
/// @{

#include <types.h>
#include "cput_arch.h"

#include "cpu/cpu.h"

/**
 * @brief Convenience macro for accessing the currently running CPU record.
 *
 * This expands to the architecture-specific current-CPU lookup and is intended
 * for per-CPU data access from code that is executing on the active logical
 * processor.
 */
#define THISCPU() cpu_arch_get_current()

/**
 * @brief Creates a CPU record for a logical processor entry.
 *
 * Initializes the architecture-specific `cpu_t` object associated with a
 * generic `cpu_entry_t`, copying in the APIC and logical IDs and linking the
 * entry back to the `cpu_t` instance.
 *
 * @param entry Logical CPU metadata describing the processor to create.
 *
 * @return KERRNO_SUCCESSES on success.
 */
status_t cpu_create(cpu_entry_t *entry);

/**
 * @brief Returns the bootstrap processor's CPU record.
 *
 * @return Pointer to the BSP's architecture-specific `cpu_t` structure.
 */
cpu_t *cpu_arch_get_bsp();

/**
 * @brief Returns the currently active CPU context.
 *
 * @return Pointer to the current CPU structure for the running processor.
 */
cpu_t *cpu_arch_get_current();

/**
 * @brief Looks up the CPU record for a logical CPU ID.
 *
 * @param logical_id Logical CPU index to query.
 *
 * @return Pointer to the matching `cpu_t`, or `NULL` if out of range.
 */
cpu_t *cpu_arch_get(cpu_logical_id_t logical_id);

/**
 * @brief Stores the top of the kernel stack for a given CPU.
 *
 * This updates the CPU state so that the kernel can later reference the
 * per-processor stack pointer or stack boundary for traps, scheduling, and
 * context switches.
 *
 * @param cpu CPU whose kernel stack pointer should be set.
 * @param stack_top Top address of the kernel stack for that processor.
 */
void cpu_arch_set_kernel_stack(cpu_t *cpu, vaddr_t stack_top);

/// @}
