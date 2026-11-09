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
status_t paging_clean_up(page_table_t *page_table, vaddr_t virtAddr);
status_t paging_unmap_page(page_table_t *page_table, vaddr_t virtAddr, paddr_t *out_paddr);
status_t allocate_leaf(page_table_t *page_table, vaddr_t start_virt, paddr_t start_phys, mmu_flags_t flags, size_t *out_size);
void paging_print_tree(page_table_t *page_dir);

status_t paging_get_entry(page_table_t *page_table, vaddr_t virt, mmu_mapping_t *out_entry);

status_t paging_remap_flags(page_table_t *page_table, vaddr_t virt, mmu_flags_t flags);

status_t paging_copy_con_entries(page_table_t *dest, page_table_t *src);

status_t paging_create_table(page_table_t *table);

status_t paging_get_physical(page_table_t *page_table, vaddr_t virt, paddr_t *out_paddr);