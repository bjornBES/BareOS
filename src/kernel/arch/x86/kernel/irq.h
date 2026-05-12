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
#include "registers.h"
#include "pic.h"

#define PIC_REMAP_OFFSET 0x20

typedef void (*IRQ_handler)(registers* regs);

extern const pic_driver *driver;

void x86_irq_initialize();
void irq_register_handler(int irq, IRQ_handler handler);
bool irq_inject_driver(const pic_driver *new_driver);
