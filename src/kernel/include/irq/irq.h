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

status_t irq_register_handler(kernel_irq_t irq, irq_source_t source, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target);
status_t irq_unregister_handler(kernel_irq_t irq);

status_t irq_mask(kernel_irq_t irq);
status_t irq_unmask(kernel_irq_t irq);
bool irq_is_masked(kernel_irq_t irq);

status_t irq_eoi(kernel_irq_t irq);

status_t irq_pick_free_entry(kernel_irq_t mask, irq_source_t source, kernel_irq_t *out);
status_t irq_pick_free_entry_cpu(kernel_irq_t mask, irq_source_t source, cpu_logical_id_t target, kernel_irq_t *out);