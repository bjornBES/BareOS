/*
 * File: IO.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "core/Defs.h"
#include <stdint.h>

void ASMCALL outb(uint16_t port, uint8_t value);
uint8_t ASMCALL inb(uint16_t port);
void ASMCALL outw(uint16_t port, uint16_t value);
uint16_t ASMCALL inw(uint16_t port);
void ASMCALL outd(uint16_t port, uint32_t value);
uint32_t ASMCALL ind(uint16_t port);

void ASMCALL enableInterrupts();
void ASMCALL disableInterrupts();

void ASMCALL callNMI();
