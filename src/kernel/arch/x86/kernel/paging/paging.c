/*
 * File: x86_paging.c
 * File Created: 24 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"

#include "kernel.h"

#include "debug/debug.h"

#include "libs/stdio.h"
#include "libs/string.h"
#include "memory/paging/paging.h"
#include "memory/flags.h"

#include "kernel/exceptions/exception.h"

#include <util/binary.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_PWT (1ULL << 3)
#define PAGE_PCD (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE_PAGES (1ULL << 7)
#define PDE_GLOBAL (1ULL << 7)
#define PAGE_NO_EXEC (1ULL << 63)

#define MODULE "x86_PAGING"

void reload_pages()
{
    uint32_64 kernel_cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(kernel_cr3));
    log_debug(MODULE, "reloading cr3 %p", kernel_cr3);
    __asm__ volatile("mov cr3, %0" ::"r"(kernel_cr3) : "memory");
}

extern virt_addr phys_to_virt_auto(phys_addr p);

extern void paging_init32(boot_params *boot, size_t kernel_size, phys_addr pmm_start, phys_addr pmm_end);
extern void paging_init64(boot_params *boot, size_t kernel_size, phys_addr pmm_start, phys_addr pmm_end);

void arch_paging_page_fault(registers *regs, kernel_handler kernel_hand)
{
    paging_info info;
    char buf[2048];
    fprintf(VFS_FD_DEBUG, "[Page Fault] Unhandled Page Fault %d\n", regs->interrupt);

    uint32_64 cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));
    info.fault_addr = (virt_addr)cr2;

    uint64_t cr3;
    uint64_t kernel_paging = (uint64_t)kernel_page.page_dir_phys;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    bool cr3_is_kernel = cr3 == kernel_paging;
    info.page_directory.page_dir = (void*)phys_to_virt_auto((phys_addr)cr3);
    info.page_directory.page_dir_phys = (void*)cr3;
    info.as_kernel = cr3_is_kernel;
    info.present = BIT_GET(regs->error, 0);
    info.write = BIT_GET(regs->error, 1);
    info.user = BIT_GET(regs->error, 2);
    info.exec = BIT_GET(regs->error, 4);
    info.pc = regs->pc;
    info.sp = regs->sp;
    exception_format_registers(regs, buf);
    fprintf(VFS_FD_DEBUG, "%s", buf);
    fprintf(VFS_FD_DEBUG, "[Page Fault] Virtual address %p\n", cr2);

    fprintf(VFS_FD_DEBUG, "[Page Fault] Error code 0x%x\n", regs->error);
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Present %u\n", BIT_GET(regs->error, 0));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Write %u\n", BIT_GET(regs->error, 1));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    User %u\n", BIT_GET(regs->error, 2));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Reserved write %u\n", BIT_GET(regs->error, 3));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Instruction Fetch %u\n", BIT_GET(regs->error, 4));
    fprintf(VFS_FD_DEBUG, "[Page Fault]    Protection key %u\n", BIT_GET(regs->error, 5));

    void *_info = (void *)&info;
    if (kernel_hand == NULL)
    {
        kernel_panic("No kernel Handler");
    }
    kernel_hand(_info);
}

void paging_load_cr3(paging_page_t new_paging_page)
{
    phys_addr proc_pd_phys = (phys_addr)((uint32_64)new_paging_page.page_dir_phys);
    fprintf(VFS_FD_DEBUG, "switching to new pml4 = %p", proc_pd_phys);
    // Switch to process page directory
    disableInterrupts();
    __asm__ volatile(
        "mov cr3, %0\n" ::"r"((uint64_t)proc_pd_phys) : "memory");
    enable_interrupts();
}

void paging_get_current_cr3(paging_page_t *paging_phys)
{
    uint32_64 cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    virt_addr virt_cr3 = phys_to_virt_auto((phys_addr)cr3);
    paging_phys->page_dir = (void *)virt_cr3;
    paging_phys->page_dir_phys = (void *)cr3;
}

void arch_paging_init(boot_params *bootParams, phys_addr pmm_start, phys_addr pmm_end)
{
    size_t kernel_size = (size_t)KERNEL_HEAP_VIRT_END - (size_t)bootParams->kernel_address;

#ifdef __x86_64__
    paging_init64(bootParams, kernel_size, pmm_start, pmm_end);
#else
    paging_init32(bootParams, kernel_size, pmm_start, pmm_end);
#endif
    exception_register_arch_handler(EXC_PAGE, arch_paging_page_fault);
}
