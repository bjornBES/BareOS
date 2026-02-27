/*
 * File: paging.h
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <core/Defs.h>

#define PDE_PRESENT 0x1
#define PDE_RW 0x2
#define PDE_USER 0x4
#define PDE_4KB 0x0

#define GetPageDirectoryIndex(addr) (uint32_t)addr >> 22
#define GetPageTableIndex(addr) (uint32_t)addr >> 12 & 0x3FF

extern uint32_t *page_directory __attribute__((aligned(4096)));
extern uint32_t *p_table __attribute__((aligned(4096)));

void *get_physaddr(void *virtualaddr);
void init_paging(void *page_table);
void *alloc_page();
void free_page(void* page);
int find_free_page();
void map_page(void *virtAddr, void *physAddr);
int check_page(uint32_t page_index);
void mark_page_used(uint32_t page_index);
void mark_page_free(uint32_t page_index);
void map_page_4kb_blocks(void *virtAddr, void *physAddr, int count);