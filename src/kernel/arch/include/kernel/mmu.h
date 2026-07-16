/*
 * File: mmu.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "mmu_types.h"
#include "kernel/asm/mmu/mmu.h"

#include <types.h>
#include <boot/bootparams.h>

#define PAGE_ALIGN_DOWN(addr)                                         ((addr) & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)                                           (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define GET_PAGE_OFFSET(addr)                                         ((addr) & ~PAGE_MASK)
#define PAGE_COUNT(size)                                              (((size) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PHYS_TO_PAGE(addr)                                            ((addr) >> PAGE_SHIFT)
#define PAGE_TO_PHYS(page)                                            ((page) << PAGE_SHIFT)

#define DEFINE_FLAGS(pre, w, exec_code, is_user, cac, glo, grow_down) {.present = pre, .write = w, .exec = exec_code, .user = is_user, .cached = cac, .global = glo, .grows_down = grow_down, .cow = 0}

// ELF .text segment
static const mmu_flags_t text_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 0, /* exec_code */ 1, /* user */ 1, /* cache */ 0, /* global */ 0, /* grows_down */ 0);
static const mmu_flags_t kernel_text_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 0, /* exec_code */ 1, /* user */ 0, /* cache */ 0, /* global */ 0, /* grows_down */ 0);
static const mmu_flags_t vdso_text_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 1, /* user */ 1, /* cache */ 0, /* global */ 0, /* grows_down */ 0);

// user stack
static const mmu_flags_t stack_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 0, /* user */ 1, /* cache */ 0, /* global */ 0, /* grows_down */ 1);

// user .data / heap
static const mmu_flags_t data_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 0, /* user */ 1, /* cache */ 0, /* global */ 0, /* grows_down */ 0);

// kernel .data / heap
static const mmu_flags_t kernel_data_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 0, /* user */ 0, /* cache */ 0, /* global */ 0, /* grows_down */ 0);

// MMIO
static const mmu_flags_t mmio_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 0, /* user */ 0, /* cache */ 1, /* global */ 0, /* grows_down */ 0);

// kernel mapping
static const mmu_flags_t kern_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 1, /* user */ 0, /* cache */ 0, /* global */ 1, /* grows_down */ 0);

// smp trampoline mapping
static const mmu_flags_t trampoline_flags = DEFINE_FLAGS(/* present */ 1, /* write */ 1, /* exec_code */ 1, /* user */ 0, /* cache */ 0, /* global */ 1, /* grows_down */ 0);

extern page_table_t kernel_page;

void mmu_arch_init(boot_params_t *bp);

// --- Address Space ---
// create a new empty address space, returns physical address of top level table
page_table_t *mmu_arch_create_table();

// destroy an address space and free all arch-owned paging structures
void mmu_arch_destroy_table(page_table_t *table);

// switch to an address space (load CR3 on x86, TTBR on ARM)
void mmu_arch_load_table(page_table_t *table);

// get currently active address space
void mmu_arch_current_table(page_table_t *table);

// --- Kernel Mapping ---
// map kernel pages into a userspace table (for higher half kernel)
void mmu_arch_map_kernel(page_table_t *table);

// --- Mapping ---
// map a single physical page to a virtual address with given flags
// creates intermediate tables as needed
int mmu_arch_map(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags);
int mmu_arch_map_debug(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags);

// unmap a single virtual page
// does NOT free the physical page, that is the callers job
paddr_t mmu_arch_unmap(page_table_t *table, vaddr_t virt);

// change flags on an existing mapping without remapping
int mmu_arch_protect(page_table_t *table, vaddr_t virt, mmu_flags_t flags);

// walk the page table and return the physical address mapped at virt
// returns 0 if not mapped
paddr_t mmu_arch_virt_to_phys(page_table_t *table, vaddr_t virt);

int mmu_arch_is_present(page_table_t *table, vaddr_t virt);

// --- TLB ---
void mmu_arch_flush_page(vaddr_t virt);
void mmu_arch_flush_all();

// --- Diagnostic ---
int mmu_arch_query(page_table_t *table, vaddr_t virt, mmu_mapping_t *out);
void mmu_arch_walk(page_table_t *table, mmu_walk_cb_t cb, void *ctx);

// walk the page table and return the virtual address mapped at phys
// returns 0 if not mapped
vaddr_t mmu_arch_phys_to_virt(page_table_t *table, paddr_t phys);
