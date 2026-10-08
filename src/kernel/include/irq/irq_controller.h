/*
 * File: irq_controller.h
 * File Created: 05 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

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

    int (*route)(kernel_irq_t irq, interrupt_vector_t vector, cpu_logical_id_t target, irq_trigger_t trigger, irq_polarity_t polarity);

    gsi_t (*irq_to_gsi)(kernel_irq_t irq);

    kernel_irq_t (*gsi_lookup)(gsi_t gsi, irq_trigger_t *trigger, irq_polarity_t *polarity);

    status_t (*bind)(gsi_t gsi, kernel_irq_t irq);
    status_t (*unbind)(gsi_t gsi);

    gsi_t (*pick_free_pin)(uint32_t mask);

    void (*disable)();

    void (*mask)(gsi_t gsi);
    void (*unmask)(gsi_t gsi);
    bool (*is_masked)(gsi_t gsi);

    void (*send_eoi)(gsi_t gsi);
    
    status_t (*debug)();
} irq_controller_t;
