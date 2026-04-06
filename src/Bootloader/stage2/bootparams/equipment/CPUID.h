/*
 * File: CPUID.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <boot/bootparams.h>
#include <stdint.h>
#include <stdbool.h>
#include "x86.h"

#define INIT_CPUID_REG(regs) memset(regs, 0, sizeof(cpuid_regs))

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs;

void ASMCALL CPUID(uint32_t eax, uint32_t ecx, cpuid_regs* outregs);

void DetectCPUID(boot_params* bp);
