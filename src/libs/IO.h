/*
 * File: IO.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "core/Defs.h"
#include <stdint.h>

void ASMCALL Outb(uint16_t port, uint8_t value);
uint8_t ASMCALL Inb(uint16_t port);
void ASMCALL Outw(uint16_t port, uint16_t value);
uint16_t ASMCALL Inw(uint16_t port);
void ASMCALL Outd(uint16_t port, uint32_t value);
uint32_t ASMCALL Ind(uint16_t port);

uint8_t ASMCALL enableInterrupts();
uint8_t ASMCALL disableInterrupts();
