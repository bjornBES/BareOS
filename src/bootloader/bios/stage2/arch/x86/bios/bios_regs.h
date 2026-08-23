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
            uint16_t di;
			uint16_t si;
			uint16_t bx;
			uint16_t dx;
			uint16_t cx;
			uint16_t ax;
			uint16_t gs;
			uint16_t fs;
			uint16_t es;
			uint16_t ds;
			uint16_t flags;
		};
		struct {
            uint8_t dil, dih;
			uint8_t sil, sih;
			uint8_t bl, bh;
			uint8_t dl, dh;
			uint8_t cl, ch;
			uint8_t al, ah;
			uint8_t gsl, gsh;
			uint8_t fsl, fsh;
			uint8_t esl, esh;
			uint8_t dsl, dsh;
			uint8_t flagl, flagh;
		};
	};
} PACKED bios_regs_t;