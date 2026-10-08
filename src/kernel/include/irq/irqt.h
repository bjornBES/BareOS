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

#include "irq_controller.h"

#include <types.h>
#include <config.h>

typedef status_t (*irq_handler_func_t)(intr_frame_t* frame, void *ctx);

typedef struct irq_handler
{
    uint8_t state;
    irq_handler_func_t handler;
    void *ctx;
} irq_handler_t;

typedef enum
{
    IRQ_SOURCE_MSI,
    IRQ_SOURCE_IRQ,
    IRQ_SOURCE_GSI,
} irq_source_t;

typedef struct irq_descriptor
{
    kernel_irq_t id;
    uint32_t flags;
    // irq_polarity_t polarity;
    // irq_trigger_t trigger;

    interrupt_vector_t vectors[CONFIG_MAX_CPUS];
} irq_descriptor_t;

typedef struct irq_range
{
    kernel_irq_t first;
    kernel_irq_t last;
} irq_range_t;
typedef struct irq_pick_reg
{
    bool use_bit_mask;
    const irq_mask_t *allowed;

    bool use_range;
    irq_range_t range;
    
    size_t count;
    size_t align;
    irq_source_t source;
    cpu_logical_id_t target;
} irq_pick_reg_t;
