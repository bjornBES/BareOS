#pragma once
#include <stdint.h>

#define PDE_PRESENT   0x1
#define PDE_RW        0x2
#define PDE_USER      0x4
#define PDE_4KB       0x0

extern uint32_t pageDirectory[1024] __attribute__((aligned(4096)));
extern uint32_t pageTableLow[1024] __attribute__((aligned(4096)));

void *paging_get_physical(void *virtualaddr);
void fill_table();