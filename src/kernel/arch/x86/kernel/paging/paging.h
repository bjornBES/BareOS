/*
 * File: paging.h
 * File Created: 03 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 03 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "mm/mmu.h"
#include "asm/page_arch.h"

extern bool paging_disable_print;

#ifdef __x86_64__
#define PAGING_64        1
#endif

paddr_t paging_print_info(page_table_t *page_dir, vaddr_t cr2);
int map(page_table_t *page_table, vaddr_t start_virt, paddr_t start_phys, size_t size, mmu_flags_t flags);
int paging_clean_up(page_table_t *page_table, vaddr_t virtAddr);
paddr_t paging_unmap_page(page_table_t *page_table, vaddr_t virtAddr);
size_t allocate_leaf(page_table_t *page_table, vaddr_t start_virt, paddr_t start_phys, mmu_flags_t flags);
void paging_print_tree(page_table_t *page_dir);