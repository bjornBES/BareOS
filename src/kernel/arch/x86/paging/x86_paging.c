/*
 * File: x86_paging.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "x86_paging.h"

#include "kernel.h"

#include "debug/debug.h"

#include "libs/stdio.h"
#include "libs/string.h"

#include "arch/x86/isr.h"

#include <util/binary.h>

#define MODULE "x86_PAGING"

void reload_pages()
{
    uint32_64 kernel_cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(kernel_cr3));
    log_debug(MODULE, "reloading cr3 %p", kernel_cr3);
    __asm__ volatile("mov cr3, %0" ::"r"(kernel_cr3) : "memory");
}

extern virt_addr phys_to_virt_auto(phys_addr p);

void paging_page_fault(Registers *regs)
{
    char buf[2048];
    x86_ISRFormatRegisters(regs, buf, "\n", "[Page Fault]   ");
    fprintf(VFS_FD_DEBUG, "[Page Fault] Unhandled Page Fault %d\n", regs->interrupt);

/*     uint64_t *pml4_raw = (uint64_t *)((uint64_t)0x63c000 + (KERNEL_VIRT_BASE - KERNEL_PHYS_BASE));
    log_debug(MODULE, "raw PML4[511] = 0x%016llx", pml4_raw[511]);
    log_debug(MODULE, "raw PML4[508] = 0x%016llx", pml4_raw[508]);

    uint64_t pdpt_phys = (pml4_raw[511] & 0x000FFFFFFFFFF000ULL);
    uint64_t *pdpt_raw = (uint64_t *)phys_to_virt_auto((phys_addr)pdpt_phys);
    log_debug(MODULE, "raw PDPT[508] = 0x%016llx", pdpt_raw[508]);

    uint64_t pd_phys = (pdpt_raw[508] & 0x000FFFFFFFFFF000ULL);
    uint64_t *pd_raw = (uint64_t *)phys_to_virt_auto((phys_addr)pd_phys);
    log_debug(MODULE, "raw PD[0] = 0x%016llx", pd_raw[0]);

    uint64_t pt_phys = (pd_raw[0] & 0x000FFFFFFFFFF000ULL);
    uint64_t *pt_raw = (uint64_t *)phys_to_virt_auto((phys_addr)pt_phys);
    log_debug(MODULE, "raw PT[0] = 0x%016llx", pt_raw[0]); */

    uint32_64 cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));
    paging_print_info((virt_addr)cr2);

    uint64_t cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    uint64_t pml4_phys = (uint64_t)kernel_page.page_dir - (uint64_t)(KERNEL_VIRT_BASE - KERNEL_PHYS_BASE);
    fprintf(VFS_FD_DEBUG, "[Page Fault] CR3=0x%llx kernel_pml4_phys=0x%llx match=%s\n",
            cr3, pml4_phys, (cr3 == pml4_phys) BOOT_TO_STRING);

    fprintf(VFS_FD_DEBUG, "%s", buf);
    fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", cr2);
    fprintf(VFS_FD_DEBUG, "[Page Fault] Physical address %p\n", paging_get_physical(kernel_page, (virt_addr)cr2));

    fprintf(VFS_FD_DEBUG, "[Page Fault] Error code 0x%x\n", regs->error);
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Present %u\n", BIT_GET(regs->error, 0));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Write %u\n", BIT_GET(regs->error, 1));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    User %u\n", BIT_GET(regs->error, 2));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Reserved write %u\n", BIT_GET(regs->error, 3));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Instruction Fetch %u\n", BIT_GET(regs->error, 4));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Protection key %u\n", BIT_GET(regs->error, 5));

    KernelPanic("Page Fault", "Got page fault from %p", cr2);
}

void paging_x86_init(boot_params *bootParams, phys_addr pmm_start, phys_addr pmm_end)
{
    size_t kernel_size = (size_t)KERNEL_HEAP_VIRT_END - (size_t)bootParams->kernel_address;

#ifdef __x86_64__
    paging_init64(bootParams, kernel_size, pmm_start, pmm_end);
#else
    paging_init32(bootParams, kernel_size, pmm_start, pmm_end);
#endif
    x86_isr_register_handler(14, paging_page_fault);
}
