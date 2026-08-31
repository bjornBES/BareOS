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

#include "debug/debug.h"

#include "stdio.h"
#include "memory.h"

#include "x86.h"
#include "bios.h"

#include <defs.h>

typedef struct extensions_dap
{
    uint8_t size;
    uint8_t res;
    uint16_t count;
    seg_off_t memory;
    uint64_t lba;
} extensions_dap_t;

extensions_dap_t ALIGN(16) s_dap;

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

    disk->cylinders = (out_regs.ch | ((out_regs.cl & 0b11000000) >> 6)) + 1;
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

extern void hexdump(void *ptr, int len);

int disk_read_sectors(disk_t *disk, uint32_t lba, size_t sectors, void *buffer)
{
    // ENTER_FUNC("0x%p, 0x%x, %u, 0x%p", disk, lba, sectors, buffer);
    uint16_t cylinder, sector, head;

    bios_regs_t out_regs;
    bios_arch_init_regs(&out_regs);

    bios_regs_t regs;
    bios_arch_init_regs(&regs);
    seg_off_t memory = mem_linear_to_segoff(buffer);

    int result;
    for (int i = 0; i < 3; i++)
    {
        if (disk->have_extensions)
        {
            extensions_dap_t *dap = (extensions_dap_t *)&s_dap;
            seg_off_t dap_memory = mem_linear_to_segoff(dap);
            dap->size = 0x10;
            dap->res = 0;
            dap->count = sectors;
            dap->memory = memory;
            dap->lba = lba;

            regs.ah = 0x42;
            regs.dl = disk->id;
            regs.ds = dap_memory.segment;
            regs.si = dap_memory.offset;
            regs.flags = 1;
            // bios_arch_dump_frame(&regs);
            // hexdump(&s_dap, 16);
            bios_arch_intcall(0x13, &regs, &out_regs);
            result = out_regs.ah;
            if (result == 0 && (out_regs.flags & X86_EFLAGS_CF) == 0)
            {
                // bios_arch_dump_frame(&out_regs);
                // printf("sectors read = %u/%u\n", dap->count, sectors);
                // printf("%u = x86_Disk_Read(disk->id:0x%x, count: %u, memory: 0x%x:0x%x(0x%p), lba: %u/0x%x) in %u trys\n", result, disk->id, sectors, memory.segment, memory.offset, buffer, lba, lba, i);
                return 0;
            }
        }
        else
        {
            size_t count = sectors;
            int times = sectors / 0xFF + 1;
            for (int j = 0; j < times; j++)
            {
                lba += out_regs.al;
                disk_lba_to_chs(disk, lba, &cylinder, &sector, &head);
                printf("x86_Disk_Read(disk->id:%x, cylinder:%u, sector:%u, head:%u, sectors:%u, buffer:%p)\n", disk->id, cylinder, sector, head, sectors, buffer);
                regs.ah = 0x02;
                if (count > 0xFF)
                {
                    regs.al = 0xFF;
                }
                else
                {
                    regs.al = count;
                }
                regs.ch = cylinder & 0xFF;
                regs.cl = sector & 0x3F;
                if (disk->id >= 0x80)
                {
                    regs.cl |= ((cylinder >> 8) & 0x3) << 6;
                }
                regs.dh = head;
                regs.dl = disk->id;
                regs.es = memory.segment;
                regs.bx = memory.offset + (i * 512);
                regs.flags = 0;
                bios_arch_intcall(0x13, &regs, &out_regs);
                if ((out_regs.flags & X86_EFLAGS_CF) == X86_EFLAGS_CF)
                {
                    printf("Error while reading code = 0x%x\n", out_regs.ah);
#ifdef DEBUG
                    printf("See page http://www.ctyme.com/intr/rb-0606.htm#Table234 for more info,\n");
                    printf("about the code\n");
#endif
                    for (;;)
                    {
                        ;
                    }
                }
                count -= out_regs.al;
            }
        }
        printf("result = %u\n", result);

        regs.ah = 0;
        regs.dl = disk->id;
        regs.flags = 0;
        bios_arch_intcall(0x13, &regs, &out_regs);
        bios_arch_init_regs(&regs);
        bios_arch_init_regs(&out_regs);
    }
    if (result)
    {
        printf("Error while reading code = 0x%x\n", result);
        if (disk->have_extensions)
        {
            hexdump(&s_dap, 16);
        }
#ifdef DEBUG
        printf("See page http://www.ctyme.com/intr/rb-0606.htm#Table234 for more info,\n");
        printf("about the code\n");
#endif
    }
    return 1;
}
