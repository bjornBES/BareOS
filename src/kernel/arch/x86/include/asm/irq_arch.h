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

status_t irq_arch_register(irq_descriptor_t *desc, irq_controller_t *current, irq_source_t source, irq_handler_func_t handler, void *ctx, cpu_logical_id_t target);
irq_handler_t *irq_arch_get_handler(interrupt_vector_t vector);

status_t irq_arch_pick_free_entry(kernel_irq_t allowed_mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *output);
