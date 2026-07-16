/*
 * File: irq.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "ctx.h"

typedef void (*irq_handler_t)(intr_frame_t* frame, void *ctx);

void irq_arch_initialize();

int irq_arch_register(uint32_t irq, irq_handler_t fn, void *ctx);
void irq_arch_unregister(uint32_t irq);

void irq_arch_mask(uint32_t irq);
void irq_arch_unmask(uint32_t irq);

void irq_arch_eoi(uint32_t vec);

void irq_arch_enable();
void irq_arch_disable();

uint32_t irq_pick_free_irq(uint32_t irq_mask);
bool irq_arch_is_masked(uint32_t irq);

void irq_arch_register_override(uint32_t gsi, uint8_t source_irq, uint16_t flags);

