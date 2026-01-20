#include "paging.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_table_low[1024] __attribute__((aligned(4096)));
uint32_t page_table_kernel[1024] __attribute__((aligned(4096)));

void fill_table()
{
    // identity map first 4 MB (low memory)
    for (int i = 0; i < 1024; i++)
        page_table_low[i] = (i * 0x1000) | 0x3; // Present + RW

    // map kernel 4 MB to 0xC0000000
    for (int i = 0; i < 1024; i++)
        page_table_kernel[i] = (0x00100000 + i * 0x1000) | 0x3;

    page_directory[0] = ((uint32_t)page_table_low) | (PDE_PRESENT | PDE_RW);
    page_directory[768] = ((uint32_t)page_table_kernel) | (PDE_PRESENT | PDE_RW);
}