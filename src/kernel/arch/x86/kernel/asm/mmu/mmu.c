/*
 * File: mmu.c
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "kernel/mmu.h"
#include "mm/allocator/memory_allocator.h"
#include "mm/mmu/mmu.h"
#include "kernel/asm/exceptions/exception.h"
#include "kernel/ivt.h"
#include "kernel/irq.h"
#include "kernel/string.h"
#include "kernel.h"
#include "mm/pmm.h"

#include "kernel/asm/paging/paging.h"

#include <util/binary.h>

#define MODULE "x86-MMU"

extern bool paging_print_out;

page_table_t kernel_page;

void mmu_arch_flush_all()
{
    uint64_t kernel_cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(kernel_cr3));
    log_debug(MODULE, "reloading cr3 %p", kernel_cr3);
    __asm__ volatile("mov cr3, %0" ::"r"(kernel_cr3) : "memory");
}

void mmu_arch_flush_page(vaddr_t virt)
{
    __asm__ volatile("invlpg [%0]" ::"r"((uintptr_t)(virt)) : "memory");
}

void mmu_arch_current_table(page_table_t *table)
{
    paddr_t cr3;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    vaddr_t virt_cr3 = phys_to_virt_auto(cr3);
    table->page_dir = virt_cr3;
    table->page_dir_phys = cr3;
}

void mmu_arch_load_table(page_table_t *table)
{
    paddr_t cr3 = (paddr_t)table->page_dir_phys;
    __asm__ volatile("mov cr3, %0" : : "r"(cr3));
}

int mmu_arch_page_fault(intr_frame_t *regs)
{
    mmu_fault_info info;

    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));

    fprintf(VFS_FD_DEBUG, "========== PAGE FAULT ==========\n");
    fprintf(VFS_FD_DEBUG, "\t{ regs @ %p, cr2 = %p }\n", regs, cr2);
    ivt_dump_frame(regs);
    fprintf(VFS_FD_DEBUG, "\t{ error = 0x%x }\n", regs->error);
    fprintf(VFS_FD_DEBUG, "========== PAGE FAULT ==========\n");

    info.fault_addr = cr2;

    uint64_t cr3;
    uint64_t kernel_paging = (uint64_t)kernel_page.page_dir_phys;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    bool cr3_is_kernel = cr3 == kernel_paging;
    info.page_directory.page_dir = phys_to_virt_auto(cr3);
    info.page_directory.page_dir_phys = (paddr_t)cr3;
    mmu_flags_t entry_flags;
#ifdef PAGING_64
    page_table_entry64 *entry = paging64_get_entry(&info.page_directory, cr2, PAGING_LEVEL_PT, 0, 0);
    if (!entry)
    {
        entry = paging64_get_entry(&info.page_directory, cr2, PAGING_LEVEL_PD, 0, 0);
    }
    if (entry)
    {
        entry_flags = pte_to_mm_flags(entry->raw & PAGE_FLAGS_MASK);
        log_debug(MODULE, "flags = %x raw = %x", entry_flags, entry->raw & PAGE_FLAGS_MASK);
        paging_print_info(&info.page_directory, cr2);
    }
#else
#endif
    info.as_kernel = cr3_is_kernel;
    info.present = BIT_GET(regs->error, 0);
    info.write = BIT_GET(regs->error, 1);
    info.user = BIT_GET(regs->error, 2);
    info.exec = !BIT_GET(regs->error, 4);
    info.entry_flags = entry_flags;
    info.is_cow = entry_flags.cow;
    fprintf(VFS_FD_DEBUG, "[Page Fault] reserved bit violation %s\n", BIT_GET(regs->error, 3) BOOL_TO_STRING);
    fprintf(VFS_FD_DEBUG, "[Page Fault] protection keys %s\n", BIT_GET(regs->error, 5) BOOL_TO_STRING);
    fprintf(VFS_FD_DEBUG, "[Page Fault] shadow-stack access %s\n", BIT_GET(regs->error, 6) BOOL_TO_STRING);
    fprintf(VFS_FD_DEBUG, "[Page Fault] HLAT paging %s\n", BIT_GET(regs->error, 7) BOOL_TO_STRING);
    fprintf(VFS_FD_DEBUG, "[Page Fault] SGX-specific access-control %s\n", BIT_GET(regs->error, 15) BOOL_TO_STRING);
    info.pc = regs->pc;
    info.sp = regs->sp;
    int result = mmu_page_fault_handler(regs, &info);
    ivt_dump_frame(regs);
    return result;
}

void mmu_arch_init(boot_params_t *bp)
{
    paddr_t heap_end = pmm_init(bp);
    vaddr_t heap_end_virt = phys_to_virt_auto(heap_end);

    irq_arch_disable();

    size_t kernel_size = (size_t)heap_end_virt - (size_t)bp->kernel_address;

#ifdef __x86_64__
    paging_init64(&kernel_page, (vaddr_t)bp->kernel_address, kernel_size, start, end);
#else
    paging_init32((vaddr_t)bp->kernel_address, kernel_size, start, end);
#endif

    log_debug(MODULE, "paging done");
    pmm_map();

    allocator_init();

    ivt_arch_set_handler(EXC_PF, mmu_arch_page_fault);
}

page_table_t *mmu_arch_create_table()
{
    page_table_t *result = malloc(sizeof(page_table_t));
#ifdef PAGING_64
    int starting_index = 256;
    page_map_level_4 *user_pml4 = (page_map_level_4 *)pmm_alloc_frame();
    page_map_level_4 *user_table_virt = (page_map_level_4 *)phys_to_virt_auto((paddr_t)user_pml4);

    for (size_t i = 0; i < PT64_ENTRIES; i++)
    {
        user_table_virt->e[i].raw = 0;
    }

    page_map_level_4 *kernel_table_virt = (page_map_level_4 *)kernel_page.page_dir;
#else
#ifdef PAGING_32
#endif
#endif

    for (int i = starting_index; i < PAGE_TABLE_ENTRIES; i++)
    {
        user_table_virt->e[i].raw = kernel_table_virt->e[i].raw;
    }
    log_debug(MODULE, "user_table_virt = %p", user_table_virt);
    log_debug(MODULE, "user_pml4 = %p", user_pml4);
    log_debug(MODULE, "result = %p", result);

    result->page_dir = (vaddr_t)user_table_virt;
    result->page_dir_phys = (paddr_t)user_pml4;
    log_debug(MODULE, "result->page_dir_virt = %p", result->page_dir);
    log_debug(MODULE, "result->page_dir_phys = %p", result->page_dir_phys);

    mmu_arch_map_kernel(result);
    return result;
}

void mmu_arch_destroy_table(page_table_t *table)
{
    pmm_free_frame(table->page_dir_phys);
    free(table);
#ifdef PAGING_64
// paging 64
#else
#ifdef PAGING_32
// paging 32
#endif
#endif
}

void mmu_arch_map_kernel(page_table_t *table)
{
#ifdef PAGING_64
    // paging 64
    page_map_level_4 *user_table_virt = (page_map_level_4 *)table->page_dir;
    page_map_level_4 *kernel_table_virt = (page_map_level_4 *)kernel_page.page_dir;
    int starting_index = 256;
#else
#ifdef PAGING_32
    // paging 32
    page_map_level_4 *user_table_virt = (page_map_level_4 *)table->page_dir;
    page_map_level_4 *kernel_table_virt = (page_map_level_4 *)kernel_page.page_dir;
    int starting_index = 256;
#endif
#endif
    log_debug(MODULE, "user user_table_virt = %p", user_table_virt);
    log_debug(MODULE, "kernel user_table_virt = %p", kernel_table_virt);

    for (int i = starting_index; i < PAGE_TABLE_ENTRIES; i++)
    {
        user_table_virt->e[i].raw = kernel_table_virt->e[i].raw;
    }
}

int mmu_arch_map(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags)
{
    return paging_map_page(table, virt, phys, flags);
}

int mmu_arch_map_debug(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags)
{
    paging_print_out = true;
    paging_map_page(table, virt, phys, flags);
    paging_print_out = false;
    return RETURN_GOOD;
}

paddr_t mmu_arch_unmap(page_table_t *table, vaddr_t virt)
{
    ENTER_FUNC(MODULE, "%p, %p", table, virt);
    paddr_t addr = paging_unmap_page(table, virt);
    if (addr == RETURN_FAILED)
    {
        return 0;
    }
    paging_clean_up(table, virt);
    return addr;
}

int mmu_arch_protect(page_table_t *table, vaddr_t virt, mmu_flags_t flags)
{
    uint64_t f = mm_flags_to_pte(flags);
#ifdef PAGING_64
    // paging 64
    page_table_entry64 *entry = paging64_get_pt_entry(table, virt, 0, 0);
    if (entry != NULL)
    {
        goto update;
    }
    return RETURN_FAILED;
#else
#ifdef PAGING_32
// paging 32
#endif
#endif
update:
    log_debug(MODULE, "found %p = {%p, %p}", entry, entry->addr, entry->raw & PAGE_FLAGS_MASK);
    paddr_t addr = entry->addr;
    entry->raw = f & PAGE_FLAGS_MASK;
    entry->addr = addr;
    return RETURN_GOOD;
}

paddr_t mmu_arch_virt_to_phys(page_table_t *table, vaddr_t virt)
{
    return paging_get_physical(table, virt);
}

int mmu_arch_is_present(page_table_t *table, vaddr_t virt)
{
    // ENTER_FUNC(MODULE, "%p, %p", table, virt);
#ifdef PAGING_64
    // paging 64
    page_table_entry64 *entry = paging64_get_pt_entry(table, virt, 0, 0);
    if (entry == NULL)
    {
        return RETURN_FAILED;
    }
    if (entry != NULL && entry->present)
    {
        return RETURN_GOOD;
    }
#else
#ifdef PAGING_32
// paging 32
#endif
#endif
    return RETURN_FAILED;
}

int mmu_arch_query(page_table_t *table, vaddr_t virt, mmu_mapping_t *out)
{
    return RETURN_FAILED;
}

void mmu_arch_walk(page_table_t *table, mmu_walk_cb_t cb, void *ctx)
{
}

vaddr_t mmu_arch_phys_to_virt(page_table_t *table, paddr_t phys)
{
    return paging_get_virtual(table, phys);
}
