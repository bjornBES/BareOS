#include "Equipment.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"
#include <core/Defs.h>
#include <core/arch/i686/bios.h>
#include <util/binary.h>

#define BPCPUID bp->cpu

#define INITCPUIDREG(regs) memset(regs, 0, sizeof(cpuid_regs));

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs;

void ASMCALL CPUID(uint32_t eax, uint32_t ecx, cpuid_regs* outregs);


void DetectCPUID(boot_params* bp)
{
    memset(&bp->cpu, 0, sizeof(CPU_info));

    cpuid_regs reg;

    INITCPUIDREG(&reg);
    CPUID(0, 0, &reg);

    memcpy(BPCPUID.vendor, &reg.ebx, 4);
    memcpy(BPCPUID.vendor + 4, &reg.edx, 4);
    memcpy(BPCPUID.vendor + 8, &reg.ecx, 4);

    printf("CPUID %s\n", BPCPUID.vendor);
}