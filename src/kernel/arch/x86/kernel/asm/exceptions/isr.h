/*
 * File: isr.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include "kernel/asm/ivt/ivt.h"

void x86_isr_initialize();
void x86_isr_register_handler(int interrupt);