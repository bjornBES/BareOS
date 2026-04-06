#pragma once

#include "pic.h"

const PICDriver* i8259_get_driver();
void i8259_send_eoi(int irq);
