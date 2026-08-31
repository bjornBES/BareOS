/*
 * File: mmu_arch.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "mmu.h"

#define PAGE_SIZE             0x1000     // 4KB
#define PAGE_SIZE_LARGE       0x200000   // 2MB
#define PAGE_SIZE_HUGE        0x40000000 // 1GB

#define PAGE_SHIFT            12         // log2(PAGE_SIZE)
#define PAGE_MASK             (~(PAGE_SIZE - 1))

#define PAGE_ALIGN_DOWN(addr) ((addr) & PAGE_MASK)
#define PAGE_ALIGN_UP(addr)   (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define GET_PAGE_OFFSET(addr) ((addr) & ~PAGE_MASK)
#define PAGE_COUNT(size)      (((size) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PHYS_TO_PAGE(addr)    ((addr) >> PAGE_SHIFT)
#define PAGE_TO_PHYS(page)    ((page) << PAGE_SHIFT)

typedef struct page_table
{
    vaddr_t page_dir;
    paddr_t page_dir_phys;
} page_table_t;


/**
 * @brief
 */
size_t mmu_arch_map(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags);
