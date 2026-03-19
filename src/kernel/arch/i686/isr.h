/*
 * File: isr.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>

typedef struct
{
	uint16_t es;
	uint16_t ds;
    union {
        struct {
            uint32_t edi;
			uint32_t esi;
			uint32_t ebp;
			uint32_t _esp;
			uint32_t ebx;
			uint32_t edx;
			uint32_t ecx;
			uint32_t eax;
		} U32;
		struct {
            uint16_t di, hdi;
			uint16_t si, hsi;
			uint16_t bp, hbp;
			uint16_t _sp, _hsp;
			uint16_t bx, hbx;
			uint16_t dx, hdx;
			uint16_t cx, hcx;
			uint16_t ax, hax;
		} U16;
		struct {
            uint8_t dil, dih, edi2, edi3;
			uint8_t sil, sih, esi2, esi3;
			uint8_t bpl, bph, ebp2, ebp3;
			uint8_t _spl, _sph, _esp2, _esp3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t al, ah, eax2, eax3;
		} U8;
	};
    uint32_t interrupt, error;
    uint32_t eip;
    uint16_t cs;
    uint32_t eflags, esp;
    uint16_t ss;
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers *regs);

void i686_ISRInitialize();
void i686_isr_register_handler(int interrupt, ISRHandler handler);
