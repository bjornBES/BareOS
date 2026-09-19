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

#include <types.h>

#define IRQ_BASE 32

typedef status_t (*irq_handler_func_t)(intr_frame_t* frame, void *ctx);

void irq_arch_enable();
void irq_arch_disable();