/*
 * File: irq.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void x86_irq_initialize();
void x86_irq_register_handler(int irq, IRQHandler handler);