/*
 * File: paging.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"
#include "stdio.h"
#include <memory.h>
#include <string.h>

uint64_t page_table_low[1024] __attribute__((aligned(4096)));
uint64_t page_table_kernel[1024] __attribute__((aligned(4096)));

pml4_t *page_map_level_4_table = (pml4_t*)0x1000;
pdpt_t *page_directory_pointer_table = (pdpt_t*)0x2000;
pd_t *page_directory_table = (pd_t*)0x3000;
pdpt_t *page_directory_pointer_table_kernel = (pdpt_t*)0x4000;
pd_t *page_directory_table_kernel = (pd_t*)0x5000;
cr3_t *_cr3 = (cr3_t*)0x6000;

#define VA_PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define VA_PDPT_IDX(va) (((va) >> 30) & 0x1FF)
#define VA_PD_IDX(va) (((va) >> 21) & 0x1FF)
#define VA_PT_IDX(va) (((va) >> 12) & 0x1FF)
#define VA_PAGE_OFF(va) ((va) & 0xFFF)

void fill_64bit_table()
{
    // need a basic table that maps low and kernel

    // Feels weird to see uint64_t types now i am so fucking
    // use to see uint32_t damn
    // - BjornBEs 23-03-2026 00:42
    pt_t *page_table_low64 = (pt_t *)page_table_low;
    pt_t *page_table_kernel64 = (pt_t *)page_table_kernel;
    memset(page_map_level_4_table, 0, 4096);
    memset(page_directory_pointer_table, 0, 4096);
    memset(page_directory_pointer_table_kernel, 0, 4096);
    memset(page_directory_table, 0, 4096);
    memset(page_directory_table_kernel, 0, 4096);
    memset(page_table_low64, 0, 4096);
    memset(page_table_kernel64, 0, 4096);
    memset(_cr3, 0, sizeof(cr3_t));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    // pml4 mapping 0x0000000000 -> 0x8000000000
    page_map_level_4_table->e[0].pfn = (uint64_t)(page_directory_pointer_table) >> 12;
    page_map_level_4_table->e[0].present = 1;
    page_map_level_4_table->e[0].rw = 1;
    // what? to fucking where the moon?
    // - BjornBEs 23-03-2026 00:48

    // pdpt mapping 0x00000000 -> 0x40000000
    page_directory_pointer_table->e[0].pfn = (uint64_t)page_directory_table >> 12;
    page_directory_pointer_table->e[0].present = 1;
    page_directory_pointer_table->e[0].rw = 1;

    // pd mapping 0x000000 -> 0x200000
    page_directory_table->e[0].pfn = (uint64_t)page_table_low64 >> 12;
    page_directory_table->e[0].present = 1;
    page_directory_table->e[0].rw = 1;

    // pt mapping first 2MB 0x000000 - 0x3FFFFF
    for (int i = 0; i < PT64_ENTRIES; i++)
    {
        page_table_low64->e[i].pfn = i;
        page_table_low64->e[i].present = 1;
        page_table_low64->e[i].rw = 1;
    }

    // kernel is at v0xFFFFFFFF80000000
    // pml4 mapping 0xFF8000000000 -> 0xFFFFFFFFFFFF
    page_map_level_4_table->e[511].pfn = (uint64_t)page_directory_pointer_table_kernel >> 12;
    page_map_level_4_table->e[511].present = 1;
    page_map_level_4_table->e[511].rw = 1;
    // We have too much memory why do ppl need 281 Tb's of RAM
    // you don't need that many minecraft servers?
    // - BjornBEs 23-03-2026 01:04

    // pdpt mapping ? -> ?
    page_directory_pointer_table_kernel->e[510].pfn = (uint64_t)page_directory_table_kernel >> 12;
    page_directory_pointer_table_kernel->e[510].present = 1;
    page_directory_pointer_table_kernel->e[510].rw = 1;

    // pd mapping ? -> ?
    page_directory_table_kernel->e[0].pfn = (uint64_t)page_table_kernel64 >> 12;
    page_directory_table_kernel->e[0].present = 1;
    page_directory_table_kernel->e[0].rw = 1;

    // Identity map for kernel ? -> ?
    for (int i = 0; i < PT64_ENTRIES; i++)
    {
        page_table_kernel64->e[i].pfn = i;
        page_table_kernel64->e[i].present = 1;
        page_table_kernel64->e[i].rw = 1;
    }
    // i will say it again.
    // We have too much memory
    // - BjornBEs 23-03-2026 01:09

    // And also "just a "basic table"?
    // - BjornBEs 23-03-2026 01:15

    _cr3->pml4_pfn = (uint64_t)page_map_level_4_table >> 12;
    // as a man once said
    // "Fuck you, Fuck you, and fuck you too Hello"
    // can't remember the name but fuck you paging until
    // we meet again.
    // - BjornBEs 23-03-2026 01:23
#pragma GCC diagnostic pop
}

void fill_32bit_table()
{
    memset(page_directory_table, 0, 4096);
    memset(page_table_low, 0, 4096);
    memset(page_table_kernel, 0, 4096);

    uint32_t *page_table_low32 = (uint32_t*)(void*)page_table_low;
    uint32_t *page_table_kernel32 = (uint32_t*)(void*)page_table_kernel;
    uint32_t *page_directory_table32 = (uint32_t*)(void*)page_directory_table;

    // Set first page directory entry to point to page_table_low
    page_directory_table32[0] = (uint64_t)((uint32_t)page_table_low32) | 0x03; // Present + RW

    // Identity map first 4MB 0x000000 - 0x3FFFFF
    for (int i = 0; i < 1024; i++)
    {
        page_table_low32[i] = (i * 0x1000) | 0x03; // Present + RW
    }

    // Map higher half (index 768) map 0x00100000 -> 0xC0000000
    page_directory_table32[768] = ((uint32_t)page_table_kernel32) | 0x03; // Present + RW
    // Identity map for kernel
    for (int i = 0; i < 1024; i++)
    {
        page_table_kernel32[i] = ((0x00100000 + i * 0x1000)) | 0x03;
    }
}