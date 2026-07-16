/*
 * File: irq.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/irq.h"
#include "pic.h"

#define PIC_REMAP_OFFSET 0x20

void x86_irq_initialize();
void irq_register_handler(int irq, irq_handler_t handler);
