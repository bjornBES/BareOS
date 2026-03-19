/*
 * File: paging.c
 * File Created: 26 Feb 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#include "paging.h"
#include "stdio.h"
#include <memory.h>
#include <string.h>

uint32_t pageDirectory[1024] __attribute__((aligned(4096)));
uint32_t pageTableLow[1024] __attribute__((aligned(4096)));
uint32_t pageTableLow2[1024] __attribute__((aligned(4096)));
uint32_t pageTable2[1024] __attribute__((aligned(4096)));
uint32_t pageTableKernel[1024] __attribute__((aligned(4096)));

void *paging_get_physical(void *virtualaddr)
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    unsigned long *pt = ((unsigned long *)&pageTableLow) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

void fill_table()
{
    memset(pageDirectory, 0, 4096);
    memset(pageTableLow, 0, 4096);
    memset(pageTableLow2, 0, 4096);
    memset(pageTableKernel, 0, 4096);

    // Set first page directory entry to point to page_table_low
    pageDirectory[0] = ((uint32_t)pageTableLow) | 0x03; // Present + RW

    // Identity map first 4MB 0x001000 - 0x3FFFFF
    for (int i = 1; i < 1023; i++)
    {
        pageTableLow[i] = (i * 0x1000) | 0x03; // Present + RW
    }

    // Set first page directory entry to point to page_table_low
    pageDirectory[1] = ((uint32_t)pageTableLow2) | 0x03; // Present + RW

    // Identity map next 4MB 0x400000 - 0x7FFFFF
    for (int i = 0; i < 1024; i++)
    {
        pageTableLow2[i] = (0x400000 + i * 0x1000) | 0x03; // Present + RW
    }

    // Map higher half (index 768) map 0x00100000 -> 0xC0000000
    pageDirectory[768] = ((uint32_t)pageTableKernel) | 0x03; // Present + RW
    // Identity map for kernel
    for (int i = 0; i < 1024; i++)
    {
        pageTableKernel[i] = ((0x00100000 + i * 0x1000)) | 0x03;
    }

    // Map (index 689) 0xAC400000 - 0xAC400000
    pageDirectory[689] = ((uint32_t)pageTable2) | 0x03; // Present + RW
    // Identity map for kernel
    for (int i = 0; i < 1024; i++)
    {
        pageTable2[i] = ((0xAC400000 * i * 0x1000)) | 0x03;
    }
}