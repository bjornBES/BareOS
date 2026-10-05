/*
 * File: irqt.h
 * File Created: 03 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/frame_arch.h"

#include <types.h>

typedef status_t (*irq_handler_func_t)(intr_frame_t* frame, void *ctx);

typedef struct irq_handler
{
    uint8_t state;
    irq_handler_func_t handler;
    void *ctx;
} irq_handler_t;

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

typedef enum
{
    IRQ_SOURCE_MSI,
    IRQ_SOURCE_IRQ,
} irq_source_t;

typedef struct irq_descriptor
{
    kernel_irq_t id;
    uint32_t flags;
    irq_polarity_t polarity;
    irq_trigger_t trigger;

    irq_handler_t handler;

    cpu_logical_id_t cpu;
    interrupt_vector_t vector;
} irq_descriptor_t;
