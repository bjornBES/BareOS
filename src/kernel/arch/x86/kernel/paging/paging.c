/*
 * File: paging.c
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/mmu_arch.h"
#include "asm/frame_arch.h"
#include "asm/ivt_arch.h"
#include "asm/vectors_arch.h"

#include "mm/pmm/pmm.h"
#include "mm/memdefs.h"

#include "debug/debug.h"

#include "ivt/ivt.h"

#include "align.h"
#include "memory.h"
#include "string.h"
#include "paging.h"
#include "panic.h"

#include <binary.h>

/// @ingroup arch_x86_mmu
/// @{

#define MODULE "x86-paging"

typedef union
{
    uint64_t raw;

    struct
    {
        uint64_t ignored0 : 3;
        uint64_t pwt : 1;   /* Page-level write-through */
        uint64_t pcd : 1;   /* Page-level cache disable */
        uint64_t ignored1 : 7;
        uint64_t addr : 40; /* Physical address >> 12 */
        uint64_t reserved : 12;
    } PACKED;
} cr3_t;

page_table_t kernel_page;

bool paging_disable_print = false;

void stack_trace(uint32_t max_frames)
{
    // ENTER_FUNC("%u, %p", max_frames, regs);
    vaddr_t bp;
    inline_asm("mov %0, rbp" : "=r"(bp));

    typedef struct
    {
        vaddr_t bp;
        vaddr_t ip;
    } stack_frame_t;

    stack_frame_t *frame = (stack_frame_t *)bp;

    trace_debug(MODULE, "Stack trace:");
    for (uint32_t i = 0; i < max_frames; i++)
    {
        // sanity check — bail if EBP looks invalid
        if (!frame || frame->ip == 0)
        {
            break;
        }

        trace_debug(MODULE, "  [%u] ip = %p, bp = %p", i, frame->ip, frame->bp);
        frame = (stack_frame_t *)frame->bp;
    }
}

int mmu_arch_page_fault(intr_frame_t *frame)
{
    // irq_arch_disable();
    mmu_fault_info info;

    vaddr_t cr2;
    __asm__("mov %0, cr2" : "=rm"(cr2));

    uint64_t cr3;
    uint64_t kernel_paging = (uint64_t)kernel_page.page_dir_phys;
    __asm__ volatile("mov %0, cr3" : "=r"(cr3));
    bool cr3_is_kernel = cr3 == kernel_paging;
    info.page_directory.page_dir = phys_to_virt_auto(cr3);
    info.page_directory.page_dir_phys = (paddr_t)cr3;

    frame_arch_dump_frame(frame);
    log_info(NO_MODULE, "CR2: 0x%016llx CR3: 0x%016llx", cr2, cr3);

    info.fault_addr = cr2;

    // mmu_flags_t entry_flags;
#ifdef PAGING_64
    /*     page_table_entry64 *entry = paging64_get_entry(&info.page_directory, cr2, PAGING_LEVEL_PT, 0, 0);
        if (!entry)
        {
            entry = paging64_get_entry(&info.page_directory, cr2, PAGING_LEVEL_PD, 0, 0);
        }
        if (entry)
        {
            entry_flags = pte_to_mm_flags(entry->raw & PAGE_FLAGS_MASK);
            // trace_debug(MODULE, "flags = %x raw = %x", entry_flags, entry->raw & PAGE_FLAGS_MASK);
        } */
    paging_print_info(&info.page_directory, cr2);
#else
#endif

    info.as_kernel = cr3_is_kernel;
    info.present = BIT_GET(frame->error, 0);
    info.write = BIT_GET(frame->error, 1);
    info.user = BIT_GET(frame->error, 2);
    info.fetch = BIT_GET(frame->error, 4);
    // info.entry_flags = entry_flags;
    // info.is_cow = entry_flags.cow;
    info.pc = frame->pc;
    info.sp = frame->sp;

    trace_info(MODULE, "[Page Fault] present %s", info.present BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] write %s", info.write BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] user %s", info.user BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] exec %s", info.fetch BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] is cow page %s", info.is_cow BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] reserved bit violation %s", BIT_GET(frame->error, 3) BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] protection keys %s", BIT_GET(frame->error, 5) BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] shadow-stack access %s", BIT_GET(frame->error, 6) BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] HLAT paging %s", BIT_GET(frame->error, 7) BOOL_TO_STRING);
    trace_info(MODULE, "[Page Fault] SGX-specific access-control %s", BIT_GET(frame->error, 15) BOOL_TO_STRING);

    KERNEL_PANIC(MODULE, "");

    // int result = mmu_page_fault_handler(frame, &info);
    return 1;
}

void mmu_arch_init(boot_params_t *bp)
{
    mmu_arch_disable_prints();
    mmu_arch_enable_prints();
    pmm_info_t *info = pmm_get_info();

    paddr_t kernel_start_phys = bp->kernel_phys_base;

    kernel_page.page_dir_phys = pmm_alloc_frame();

    trace_debug(MODULE, "kernel_text_flags = %llx, @ %p", kernel_text_flags, &kernel_text_flags);
    map(&kernel_page, kernel_start_phys, bp->kernel_virt_base, bp->kernel_size, kernel_text_flags);

    paddr_t start_phys_aligned = info->phys_start; // pmm_start gotten from the pmm allocator (buddy)
    paddr_t end_phys_aligned = info->phys_end;     // pmm_end gotten from the pmm allocator (buddy)
    size_t pmm_size = ALIGN_2_UP(info->phys_end - info->phys_start, 0x1000000);
    trace_warn(MODULE, "pmm_start = %p pmm_end = %p", info->phys_start, info->phys_end);
    map(&kernel_page, info->phys_start, (vaddr_t)MEMORY_DIRECT_MAP_VIRT_BASE, pmm_size, kernel_data_flags);

    trace_debug(MODULE, "setting cr3");
    inline_asm("mov cr3, %0" : : "r"(kernel_page.page_dir_phys));
    trace_debug(MODULE, "kernel_page @%p", &kernel_page);

    ivt_set_handler(EXC_FAULT, mmu_arch_page_fault);

    paging_print_tree(&kernel_page);

    pmm_init();
}

status_t mmu_arch_load_table(page_table_t *table)
{
    paddr_t cr3 = (paddr_t)table->page_dir_phys;
    inline_asm("mov cr3, %0" : : "r"(cr3));
    return KERRNO_SUCCESSES;
}

size_t mmu_arch_map(page_table_t *table, vaddr_t virtAddr, paddr_t physAddr, mmu_flags_t flags)
{
    // this is done becurse the kernel should bbe able to map with 2 MB pages only 4 KB for the kernel
    // and then other systems like the DMA can map with 2 MB pages.
    flags.large = 0;
    if (!paging_disable_print)
    {
        ENTER_FUNC("%p, %p, %p, 0x%x", table, virtAddr, physAddr, flags);
    }
    vaddr_t virt = (vaddr_t)ALIGN_2_UP((uint64_t)virtAddr, PAGE_SIZE);

    return allocate_leaf(table, virt, physAddr, flags);
}

paddr_t mmu_arch_unmap(page_table_t *table, vaddr_t virt)
{
    if (!paging_disable_print)
    {
        ENTER_FUNC(MODULE, "%p, %p", table, virt);
    }

    paddr_t addr = paging_unmap_page(table, virt);

    if (addr > 0 && addr < 1024)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "Internal function (paging_unmap_page) return %u", addr);
    }
    int state = paging_clean_up(table, virt);
    if (state != 0)
    {
        KERRNO_RETURN(KERRNO_NOT_ALLOWED, "Internal function (paging_clean_up) return %u", state);
    }
    return addr;
}

void mmu_arch_flush_page(vaddr_t virt)
{
    inline_asm("invlpg [%0]" ::"r"((uintptr_t)(virt)) : "memory");
}

void mmu_arch_flush_all()
{
    uint64_t kernel_cr3;
    inline_asm("mov %0, cr3" : "=r"(kernel_cr3));
    trace_debug(MODULE, "reloading cr3 %p", kernel_cr3);
    inline_asm("mov cr3, %0" ::"r"(kernel_cr3) : "memory");
}

void mmu_arch_disable_prints()
{
    paging_disable_print = true;
}

void mmu_arch_enable_prints()
{
    paging_disable_print = false;
}

void mmu_arch_set_prints(bool disable)
{
    paging_disable_print = disable;
}

bool mmu_arch_is_prints_disable()
{
    return paging_disable_print;
}

/// @}
