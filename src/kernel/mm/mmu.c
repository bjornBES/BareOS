/*
 * File: mmu.c
 * File Created: 04 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "mm/mmu.h"

#include "asm/mmu_arch.h"
#include "asm/page_arch.h"

#include "debug/debug.h"

#include "align.h"

#define DEF_FLAGS(name, pre, r, w, exec_code, is_user, cac, glo, grow_down) \
    mmu_flags_t name = {.present = pre,                                     \
                        .read = r,                                          \
                        .write = w,                                         \
                        .exec = exec_code,                                  \
                        .user = is_user,                                    \
                        .cached = cac,                                      \
                        .global = glo,                                      \
                        .grows_down = grow_down,                            \
                        .cow = 0,                                           \
                        .allocatable = 0}

#define DEFINE_FLAGS_READ_ONLY(name, exec, user) DEF_FLAGS(name, 1, 1, 0, exec, user, 0, !user, 0)
#define DEFINE_FLAGS_WRITE_ONLY(name, user)      DEF_FLAGS(name, 1, 0, 1, 0, user, 0, !user, 0)
#define DEFINE_FLAGS_RW(name, exec, user)        DEF_FLAGS(name, 1, 1, 1, exec, user, 0, !user, 0)
#define DEFINE_FLAGS_STACK(name, user)           DEF_FLAGS(name, 1, 1, 1, 0, user, 0, !user, 1)
#define DEFINE_FLAGS_MMIO(name)                  DEF_FLAGS(name, 1, 1, 1, 0, 0, 1, 0, 0)

#define MODULE                                   "mmu"

mmu_flags_t flags_none = {0};

// kernel .text
DEFINE_FLAGS_RW(kernel_text_flags, /* exec */ 1, /* user */ 0);

// kernel .data / heap
DEFINE_FLAGS_RW(kernel_data_flags, 0, 0);

// smp trampoline mapping
DEFINE_FLAGS_RW(trampoline_flags, /* exec */ 1, /* user */ 0);

// vdso segment
DEFINE_FLAGS_READ_ONLY(vdso_text_flags, /* exec */ 1, /* user */ 1);

// kernel stack
DEFINE_FLAGS_STACK(kernel_stack_flags, 0);

// user .text
DEFINE_FLAGS_READ_ONLY(text_flags, /* exec */ 1, /* user */ 1);

// user stack
DEFINE_FLAGS_STACK(stack_flags, 1);

// user .data / heap
DEFINE_FLAGS_RW(data_flags, 0, 1);

// user .rodata
DEFINE_FLAGS_READ_ONLY(rodata_flags, /* exec */ 0, /* user */ 1);

// MMIO
DEFINE_FLAGS_MMIO(mmio_flags);

size_t mmu_map_region(page_table_t *table, vaddr_t _virt, paddr_t _phys, size_t size, mmu_flags_t flags)
{
    vaddr_t virt = _virt;
    paddr_t phys = _phys;

    // Round up to page boundary
    size_t pages = PAGE_ALIGN_UP(size) / PAGE_SIZE;
    if (!mmu_arch_is_prints_disable())
    {
        log_info(MODULE, "mapping region [v%p-v%p] to [p%p-p%p] size in pages is %u with 0x%lx to %p", virt, virt + pages * PAGE_SIZE, phys, phys + pages * PAGE_SIZE, pages, flags, table->page_dir);
    }

    bool prints = mmu_arch_is_prints_disable();
    mmu_arch_disable_prints();
    size_t total_size = 0;
    for (size_t i = 0; i < pages; i++)
    {
        size_t mapped_size = mmu_arch_map(table, virt, phys, flags);
        mmu_arch_flush_page(virt);
        virt += mapped_size;
        phys += mapped_size;
        total_size += mapped_size;
    }
    mmu_arch_set_prints(prints);

    return total_size;
}

status_t mmu_free_region(page_table_t *table, vaddr_t _virt, size_t size)
{
    vaddr_t virt = _virt;
    // error things maybe?

    // Round up to page boundary
    size_t pages = PAGE_ALIGN_UP(size) / PAGE_SIZE;

    for (size_t i = 0; i < pages; i++)
    {
        mmu_arch_unmap(table, virt);
        virt += PAGE_SIZE;
    }
    return KERRNO_SUCCESSES;
}
