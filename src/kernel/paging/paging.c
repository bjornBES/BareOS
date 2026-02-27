/*
 * File: paging.c
 * File Created: 17 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"
#include "stdio.h"
#include "debug.h"
#include <memory.h>
#include <string.h>
#include <util/binary.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 65536

uint8_t page_bitmap[MAX_PAGES / 8]; // 1 bit per page
uint32_t *p_table __attribute__((aligned(4096)));
uint32_t *page_directory __attribute__((aligned(4096)));

void *get_physaddr(void *virtualaddr)
{
    uint32_t pdindex = GetPageDirectoryIndex(virtualaddr);
    uint32_t ptindex = GetPageTableIndex(virtualaddr);

    uint32_t *pt = ((uint32_t *)p_table) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.

    return (void *)((pt[ptindex] & ~0xFFF) + ((uint32_t)virtualaddr & 0xFFF));
}

void init_paging(void *page_table)
{
    p_table = page_table;
    int page_table_index = GetPageDirectoryIndex(page_table);
    mark_page_used(page_table_index);
    mark_page_used(1);
    mark_page_used(2);
    mark_page_used(3);
    mark_page_used(0x300);
    // map_page_4kb_blocks(page_table, page_table, 1024);
    map_page_4kb_blocks((void *)0x400000, (void *)0x400000, 1024);
    // map_page_4kb_blocks((void*)0x800000, (void*)0x800000, 1024);
    // map_page_4kb_blocks((void*)0xC00000, (void*)0xC00000, 1024);

    // [Paging] addr = 0x1000000
    void *addr = get_physaddr((void *)0x1000000);
    log_debug("Paging", "addr = %p", addr);
}

void map_page(void *virtAddr, void *physAddr)
{
    log_info("Paging", "Start mapping virt %p to phys %p", virtAddr, physAddr);
    disableInterrupts();
    int page_index = GetPageDirectoryIndex(virtAddr);
    if (!check_page(page_index))
    {
        write_error(LVL_ERROR, "Paging", "Error: Page %u has not been allocated", page_index);
        return;
    }

    log_debug("paging", "page_index = %u", page_index);
    int page_table_index = GetPageTableIndex(virtAddr);

    if (page_directory[page_index] != 0)
    {
        write_error(LVL_ERROR, "Paging", "Error map(%p (%u, %u), %p): Page directory %u has been set", virtAddr, page_index, page_table_index, physAddr, page_index);
        return;
    }

    log_debug("paging", "page_table_index = %u", page_table_index);
    uint32_t *page_table = p_table + (0x400 * page_index);
    log_debug("paging", "page_table = %p", page_table);

    log_debug("paging", "page_directory + index = %p", &page_directory[page_index]);
    page_directory[page_index] = ((uint32_t)&page_table[page_table_index]) | 0x03;

    log_debug("paging", "page_table + index = %p", &page_table[page_table_index]);
    page_table[page_table_index] = (uint32_t)(physAddr) | 0x03;

    __asm__ volatile("invlpg (%0)" : : "r"(virtAddr) : "memory");
    enableInterrupts();
    log_info("Paging", "Done mapping virt %p to phys %p", virtAddr, physAddr);
}

void map_page_4kb_blocks(void *virtAddr, void *physAddr, int count)
{
    log_info("Paging", "Start mapping virt %p to phys %p %i times", virtAddr, physAddr, count);
    disableInterrupts();
    int page_index = GetPageDirectoryIndex(virtAddr);
    if (!check_page(page_index))
    {
        write_error(LVL_ERROR, "Paging", "Error: Page %u has not been allocated", page_index);
        return;
    }

    log_debug("paging", "page_index = %u", page_index);
    int page_table_index = GetPageTableIndex(virtAddr);

    log_debug("paging", "page_table_index = %u", page_table_index);
    uint32_t *page_table = p_table + (0x400 * page_index);
    log_debug("paging", "page_table = %p", page_table);

    log_debug("paging", "page_directory + index = %p", &page_directory[page_index]);
    page_directory[page_index] = ((uint32_t)&page_table[page_table_index]) | 0x03;

    log_debug("paging", "page_table + index = %p", &page_table[page_table_index]);
    log_debug("paging", "page_table range %p-%p", &page_table[page_table_index], &page_table[page_table_index] + count * 0x1000);
    for (size_t i = 0; i < count; i++)
    {
        page_table[page_table_index + i] = (uint32_t)(physAddr + i * 0x1000) | 0x03;
    }

    __asm__ volatile("invlpg (%0)" : : "r"(virtAddr) : "memory");
    enableInterrupts();
    log_info("Paging", "Done mapping virt %p to phys %p", virtAddr, physAddr);
}

/// @brief Will allocate a page in the bitmap
/// @return Returns the physical address of the page
void *alloc_page()
{
    int i = find_free_page();
    if (i == -1)
    {
        write_error(LVL_CRITICAL, "Paging", "crit error: Out of pages");
        log_debug("Paging", "why and how the fuck did that happen");
        return NULL;
    }

    mark_page_used(i);

    return (void *)(i * PAGE_SIZE);
}

void free_page(void *page)
{
    int page_index = (uint32_t)(page) / PAGE_SIZE;

    if (!check_page(page_index))
    {
        write_error(LVL_ERROR, "Paging", "error: Wrong page, ");
    }
}

int find_free_page()
{
    for (size_t i = 0; i < MAX_PAGES; i++)
    {
        if (!check_page(i))
        {
            return i;
        }
    }
    return -1;
}

inline int check_page(uint32_t page_index)
{
    return BIT_GET(page_bitmap[page_index / 8], page_index % 8);
}

inline void mark_page_used(uint32_t page_index)
{
    BIT_SET(page_bitmap[page_index / 8], (page_index % 8));
}

inline void mark_page_free(uint32_t page_index)
{
    BIT_UNSET(page_bitmap[page_index / 8], (page_index % 8));
}
