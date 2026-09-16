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

#include "asm/irq_arch.h"

#include <types.h>

typedef enum
{
    IRQ_TRIGGER_EDGE,
    IRQ_TRIGGER_LEVEL
} irq_trigger_t;

typedef enum
{
    IRQ_POLARITY_HIGH,
    IRQ_POLARITY_LOW
} irq_polarity_t;

typedef struct irq_controller
{
    const char *name;
    status_t (*probe)();
    status_t (*initialize)();
    int (*route)(gsi_t gsi, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity);
    void (*disable)();
    void (*send_eoi)(gsi_t gsi);
    void (*mask)(gsi_t gsi);
    void (*unmask)(gsi_t gsi);
    bool (*is_masked)(gsi_t gsi);
} irq_controller_t;

status_t irq_initialize(irq_controller_t *(*get_ops)());

status_t irq_register_handler(gsi_t gsi, irq_handler_func_t handler, void *ctx, irq_trigger_t trigger, irq_polarity_t polarity, cpu_logical_id_t target);
status_t irq_unregister_handler(gsi_t gsi);

void irq_mask(gsi_t gsi);
void irq_unmask(gsi_t gsi);
bool irq_is_masked(gsi_t gsi);

void irq_eoi(gsi_t gsi);

gsi_t irq_pick_free_gsi(gsi_t gsi_mask);