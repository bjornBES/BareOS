/*
 * File: kernel.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel.h"

#include "cpuid.h"
#include "paging.h"

#include <defs.h>
#include <binary.h>

SYSV_ASMCALL void x86_enable64(uint64_t boot_params, uint64_t entry);

void enter_kernel(boot_params_t *boot_params, boot_start *entry)
{
    // check CPUID.0x01:EBX[23:16] APIC_ID_SPACE
    // check CPUID.0x01:EBX[31:24] INITIAL_APIC_ID

    // check CPUID.0x01:EDX[9] APIC

    // check CPUID.0x01:EDX[3] PSE (page size extensions for large pages of size 4 MByte)
    // check CPUID.0x01:EDX[17] PSE_36 (supports the 36-Bit Page Size Extension which enables 4-MByte)

    // check CPUID.0x80000000:EAX Maximum Input Value for Extended Function CPUID Information
    // check CPUID.0x80000001:EDX[29] intel64

    cpuid_regs regs;
    cpuid(0x80000000, 0, &regs);
    if (regs.eax != 0)
    {
        cpuid(0x01, 0, &regs);
        // check CPUID.0x01:EDX[3] PSE
        // check CPUID.0x01:EDX[6] PAE
        // check CPUID.0x01:EDX[5] MSR
        if (BIT_GET(regs.edx, 3) == 0 || BIT_GET(regs.edx, 6) == 0 || BIT_GET(regs.edx, 5) == 0)
        {
            goto _32bit_kernel;
        }

        // set PAE bit in cr4
        uint32_t cr4;
        inline_asm("mov %0, cr4" : "=r"(cr4));
        cr4 |= BIT(5); // enable PAE
        cr4 |= BIT(4); // enable PSE
        inline_asm("mov cr4, %0" : : "r"(cr4));

        // setup the page tables + set cr3 with pml4
        paging64_init(boot_params);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
        x86_enable64((uint64_t)boot_params, (uint64_t)entry);
#pragma GCC diagnostic pop

        UNREACHABLE();
    }
    else
    {
_32bit_kernel:
        // set cr3
        // enable 32 bit paging
        // push boot_params params
        // jump to kernel
    }
}
