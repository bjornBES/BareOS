#pragma once
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void i686_IRQInitialize();
void i686_IRQRegisterHandler(int irq, IRQHandler handler);