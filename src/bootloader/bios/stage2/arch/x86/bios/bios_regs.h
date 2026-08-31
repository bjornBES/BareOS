/*
 * File: bios_regs.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <defs.h>

typedef struct _biosregs {
    union {
		struct {
            uint32_t edi; // + 0
			uint32_t esi; // + 4
			uint32_t ebx; // + 8
			uint32_t edx; // + 12
			uint32_t ecx; // + 16
			uint32_t eax; // + 20
			uint16_t gs_32; // + 24
			uint16_t fs_32; // + 26
			uint16_t es_32; // + 28
			uint16_t ds_32; // + 30
			uint32_t eflags; // + 32
		};
		struct {
            uint16_t di, di_h;
			uint16_t si, si_h;
			uint16_t bx, bx_h;
			uint16_t dx, dx_h;
			uint16_t cx, cx_h;
			uint16_t ax, ax_h;
			uint16_t gs;
			uint16_t fs;
			uint16_t es;
			uint16_t ds;
			uint16_t flags, flags_h;
		};
		struct {
            uint8_t dil, dih, edi2, edi3;
			uint8_t sil, sih, esi2, esi3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t al, ah, eax2, eax3;
			uint8_t gsl, gsh;
			uint8_t fsl, fsh;
			uint8_t esl, esh;
			uint8_t dsl, dsh;
			uint8_t flagl, flagh, eflags2, eflags3;
		};
	};
} PACKED bios_regs_t;