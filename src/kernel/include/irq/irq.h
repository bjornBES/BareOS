/*
 * File: irq.h
 * File Created: 16 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 16 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "irqt.h"
#include "asm/irq_arch.h"

#include <types.h>

#define ANY_CPU (cpu_logical_id_t)(~0)

status_t irq_initialize(irq_controller_t *(*get_ops)());

/// @brief 
/// @param irq 
/// @param source 
/// @param handler 
/// @param ctx 
/// @param trigger 
/// @param polarity 
/// @param target 
/// @return KERRNO_SUCCESS on successes or an kerrno number
/// @retval KERRNO_ENTRY_IN_USE: If the entry is already in use
status_t irq_register_handler(kernel_irq_t irq, irq_source_t source, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target);
status_t irq_unregister_handler(kernel_irq_t irq);

status_t irq_mask(kernel_irq_t irq);
status_t irq_unmask(kernel_irq_t irq);
bool irq_is_masked(kernel_irq_t irq);

status_t irq_eoi(kernel_irq_t irq);

status_t irq_pick_free_gsi(uint32_t allowed_pins, kernel_irq_t *irq_out, gsi_t *out);
status_t irq_pick_free_entry(irq_mask_t *mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out);
status_t irq_pick_free_range_size(irq_range_t range, size_t count, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out);

static ALWAYS_INLINE status_t irq_pick_one(kernel_irq_t irq, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out)
{
    irq_mask_t mask;
    SET_BIT(irq, mask.bits);
    return irq_pick_free_entry(&mask, source, target, out);
}

void irq_debug();