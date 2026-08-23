/*
 * File: disk.c
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "disk.h"

#include "stdio.h"

#include "x86.h"
#include "bios.h"

int disk_initialize(disk_t *disk, uint16_t boot_drive)
{
    // get drive parameters
    bios_regs_t out_regs;

    bios_regs_t regs;
    bios_arch_init_regs(&regs);
    regs.ah = 0x08;
    regs.dl = boot_drive;
    regs.es = 0;
    regs.di = 0;
    regs.flags = 0;
    bios_arch_intcall(0x13, &regs, &out_regs);

    if ((out_regs.flags & X86_EFLAGS_CF) == X86_EFLAGS_CF)
    {
        return 1;
    }

    disk->cylinders = out_regs.ch | ((out_regs.cl & 0b11000000) >> 6) + 1;
    disk->sectors = out_regs.cl & 0x3F;
    disk->heads = out_regs.dh + 1;
    disk->id = boot_drive;

    if (boot_drive >= 0x80)
    {
        // extensions - installation check
        bios_arch_init_regs(&regs);
        regs.ah = 0x41;
        regs.bx = 0x55AA;
        regs.dl = boot_drive;
        regs.flags = 0;
        bios_arch_intcall(0x13, &regs, &out_regs);
        if ((out_regs.flags & X86_EFLAGS_CF) == X86_EFLAGS_CF && out_regs.ah == 0x01)
        {
            return 2;
        }
        disk->have_extensions = 1;
    }
    return 0;
}