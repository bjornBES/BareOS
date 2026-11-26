/*
 * File: irq_arch.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "type_arch.h"

#include "asm/cput_arch.h"

#include "irq/irqt.h"

#include <types.h>

#define IRQ_BASE 32

void irq_arch_enable();
void irq_arch_disable();

/// @brief 
/// @param desc 
/// @param current 
/// @param source 
/// @param handler 
/// @param ctx 
/// @param trigger 
/// @param polarity 
/// @param target 
/// @param out_target 
/// @return KERRNO_SUCCESS on successes or an kerrno number.
/// @retval KERRNO_IRQ_ALLOC_FAILED: If the allocation failed.
/// @retval KERRNO_DEVICE_FULL: If the given CPU doesn't have any empty entries.
status_t irq_arch_register(irq_descriptor_t *desc, irq_controller_t *current, irq_source_t source, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target, cpu_logical_id_t *out_target);
irq_handler_t *irq_arch_get_handler(interrupt_vector_t vector);
