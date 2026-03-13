#pragma once
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void i686_irq_initialize();
void i686_irq_register_handler(int irq, IRQHandler handler);