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

pml4_t page_map_level_4_table __attribute__((aligned(4096)));
pdpt_t page_directory_pointer_table __attribute__((aligned(4096)));
pd_t page_directory_table __attribute__((aligned(4096)));
pdpt_t page_directory_pointer_table_kernel __attribute__((aligned(4096)));
pd_t page_directory_table_kernel __attribute__((aligned(4096)));
uint32_t page_table_low32[PT32_ENTRIES] __attribute__((aligned(4096)));
uint32_t page_table_kernel32[PT32_ENTRIES] __attribute__((aligned(4096)));
uint32_t page_directory_table32 __attribute__((aligned(4096)));
cr3_t _cr3 __attribute__((aligned(4096)));

#define VA_PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define VA_PDPT_IDX(va) (((va) >> 30) & 0x1FF)
#define VA_PD_IDX(va) (((va) >> 21) & 0x1FF)
#define VA_PT_IDX(va) (((va) >> 12) & 0x1FF)
#define VA_PAGE_OFF(va) ((va) & 0xFFF)

void fill_64bit_table()
{
    uint32_t stack;
    uint32_t stack_ebp;
    __asm__ ("mov %0, esp" : "=r" (stack));
    __asm__ ("mov %0, ebp" : "=r" (stack_ebp));
    printf("esp = %x\n", stack);
    printf("return = %x\n", *(uint32_t*)(stack_ebp + 4));
    // need a basic table that maps low and kernel

    // Feels weird to see uint64_t types now i am so fucking
    // use to see uint32_t damn
    // - BjornBEs 23-03-2026 00:42
    pt_t *page_table_low64 = (pt_t *)page_table_low;
    pt_t *page_table_kernel64 = (pt_t *)page_table_kernel;
    pt_t *page_table_kernel64_high = (pt_t *)page_table_kernel32;
    pt_t *page_table_kernel64_high2 = (pt_t *)page_table_low32;
    memset(&page_map_level_4_table, 0, 4096);
    memset(&page_directory_pointer_table, 0, 4096);
    memset(&page_directory_pointer_table_kernel, 0, 4096);
    memset(&page_directory_table, 0, 4096);
    memset(&page_directory_table_kernel, 0, 4096);
    memset(&page_table_low, 0, 4096);
    memset(&page_table_kernel, 0, 4096);
    memset(&page_table_kernel32, 0, 4096);
    memset(&page_table_low32, 0, 4096);
    memset(&_cr3, 0, sizeof(cr3_t));
    printf("zero all\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    // pml4 mapping 0x0000000000 -> 0x8000000000
    page_map_level_4_table.e[0].raw = 0;
    page_map_level_4_table.e[0].pfn = (uint64_t)(&page_directory_pointer_table) >> 12;
    page_map_level_4_table.e[0].present = 1;
    page_map_level_4_table.e[0].rw = 1;
    // what? to fucking where the moon?
    // - BjornBEs 23-03-2026 00:48

    // pdpt mapping 0x00000000 -> 0x40000000
    page_directory_pointer_table.e[0].raw = 1;
    page_directory_pointer_table.e[0].pfn = ((uint64_t)&page_directory_table) >> 12;
    page_directory_pointer_table.e[0].present = 1;
    page_directory_pointer_table.e[0].rw = 1;

    // pd mapping 0x000000 -> 0x200000
    page_directory_table.e[0].raw = 1;
    page_directory_table.e[0].pfn = (uint64_t)page_table_low >> 12;
    page_directory_table.e[0].present = 1;
    page_directory_table.e[0].rw = 1;

    // pt mapping first 2MB 0x000000 - 0x3FFFFF
    for (int i = 0; i < PT64_ENTRIES; i++)
    {
        page_table_low64->e[i].raw = 1;
        page_table_low64->e[i].pfn = i;
        page_table_low64->e[i].present = 1;
        page_table_low64->e[i].rw = 1;
    }
    printf("0x0->0x3FFFFF done\n");

    // kernel is at v0xFFFFFFFF80000000
    // pml4 mapping 0xFF8000000000 -> 0xFFFFFFFFFFFF
    int pml4KernelIndex = VA_PML4_IDX(0xFFFFFFFF80000000);
    page_map_level_4_table.e[pml4KernelIndex].raw = 0;
    page_map_level_4_table.e[pml4KernelIndex].pfn = ((uint64_t)&page_directory_pointer_table_kernel) >> 12;
    page_map_level_4_table.e[pml4KernelIndex].present = 1;
    page_map_level_4_table.e[pml4KernelIndex].rw = 1;
    // We have too much memory why do ppl need 281 Tb's of RAM
    // you don't need that many minecraft servers?
    // - BjornBEs 23-03-2026 01:04

    // pdpt mapping ? -> ?
    int pdptKernelIndex = VA_PDPT_IDX(0xFFFFFFFF80000000);
    page_directory_pointer_table_kernel.e[pdptKernelIndex].raw = 0;
    page_directory_pointer_table_kernel.e[pdptKernelIndex].pfn = ((uint64_t)&page_directory_table_kernel) >> 12;
    page_directory_pointer_table_kernel.e[pdptKernelIndex].present = 1;
    page_directory_pointer_table_kernel.e[pdptKernelIndex].rw = 1;

    // pd mapping ? -> ?
    int pdKernelIndex = VA_PD_IDX(0xFFFFFFFF80000000);
    pd_huge_entry64 *entry = (pd_huge_entry64*)&page_directory_table_kernel.e[pdKernelIndex];
    entry->raw = 0;
    entry->present = 1;
    entry->writable = 1;
    entry->ps = 1;
    entry->addr = 1;

    entry = (pd_huge_entry64*)&page_directory_table_kernel.e[pdKernelIndex + 1];
    entry->raw = 0;
    entry->present = 1;
    entry->writable = 1;
    entry->ps = 1;
    entry->addr = 2;

    entry = (pd_huge_entry64*)&page_directory_table_kernel.e[pdKernelIndex + 2];
    entry->raw = 0;
    entry->present = 1;
    entry->writable = 1;
    entry->ps = 1;
    entry->addr = 3;
    printf("kernel done\n");
    // i will say it again.
    // We have too much memory
    // - BjornBEs 23-03-2026 01:09

    // And also "just a "basic table"?
    // - BjornBEs 23-03-2026 01:15

    _cr3.pml4_pfn = ((uint64_t)&page_map_level_4_table) >> 12;
    // as a man once said
    // "Fuck you, Fuck you, and fuck you too Hello"
    // can't remember the name but fuck you paging until
    // we meet again.
    // - BjornBEs 23-03-2026 01:23
    __asm__ ("mov %0, esp" : "=r" (stack));
    paging_end:
    printf("return = %x\n", *(uint32_t*)(stack_ebp + 4));
    printf("esp = %x\n", stack);
#pragma GCC diagnostic pop
}

void fill_32bit_table()
{
    #ifdef __i686__
    memset(&page_directory_table32, 0, 4096);
    memset(page_table_low32, 0, 4096);
    memset(page_table_kernel32, 0, 4096);
    
    uint32_t *_page_directory_table32 = (uint32_t *)&page_directory_table32;
    
    // Set first page directory entry to point to page_table_low
    _page_directory_table32[0] = (uint32_t)page_table_low32 | 0x03; // Present + RW
    
    // Identity map first 4MB 0x000000 - 0x3FFFFF
    for (int i = 0; i < 1024; i++)
    {
        page_table_low32[i] = (i * 0x1000) | 0x03; // Present + RW
    }
    
    // Map higher half (index 768) map 0x00100000 -> 0xC0000000
    _page_directory_table32[768] = ((uint32_t)page_table_kernel32) | 0x03; // Present + RW
    // Identity map for kernel
    for (int i = 0; i < 1024; i++)
    {
        page_table_kernel32[i] = ((0x00100000 + i * 0x1000)) | 0x03;
    }
    #endif
}