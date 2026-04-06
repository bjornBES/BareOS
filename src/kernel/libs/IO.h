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

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);
void outd(uint16_t port, uint32_t value);
uint32_t ind(uint16_t port);

void enable_interrupts();
void disableInterrupts();

void callNMI();
