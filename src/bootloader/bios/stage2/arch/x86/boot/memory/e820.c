/*
 * File: e820.c
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mem_detect.h"
#include "memory.h"
#include "bios/bios.h"
#include "stdio.h"
#include "x86.h"

typedef struct
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t acpi;
} e820_memory_block;

enum e820_memory_block_type
{
    E820_USABLE = 1,
    E820_RESERVED = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS = 4,
    E820_BAD_MEMORY = 5,
};

void mm_arch_detect_memory(boot_params_t *bt)
{
    e820_memory_block block;
    uint32_t continuation = 0;

    int mem_region_count = 0;
    bios_regs_t out_regs;

    bios_regs_t regs;
    bios_arch_init_regs(&regs);
    regs.eax = 0x0000E820;
    regs.edx = 0x534D4150;
    regs.ecx = 0x24;
    seg_off_t memory = mem_linear_to_segoff(&block);
    regs.es = memory.segment;
    regs.di = memory.offset;
    regs.flags = 0;
    memset(&block, 0, sizeof(e820_memory_block));
    while (true)
    {
        regs.ebx = continuation;
        bios_arch_intcall(0x15, &regs, &out_regs);
        if ((out_regs.flags & X86_EFLAGS_CF) == X86_EFLAGS_CF)
        {
            break;
        }
        continuation = out_regs.ebx;
        if (continuation == 0)
        {
            break;
        }
        e820_memory_block *entry = mem_segoffset_to_linear(memory.segment, memory.offset);
        bt->memory.entries[mem_region_count].addr = block.addr;
        bt->memory.entries[mem_region_count].size = block.size;
        bt->memory.entries[mem_region_count].type = block.type;
        if (out_regs.ecx == 0x24)
        {
            bt->memory.entries[mem_region_count].ext_data = block.acpi;
        }
        else
        {
            bt->memory.entries[mem_region_count].ext_data = 0;
        }
        mem_region_count++;

        printf("E820: base=0x%llx, length=0x%llx, type=0x%x, acpi=0x%x\n", entry->addr, entry->size, entry->type, entry->acpi);
        memset(&block, 0, sizeof(e820_memory_block));
    }
    bt->memory.count = mem_region_count;
    printf("E820: count=%u\n", bt->memory.count);
}
