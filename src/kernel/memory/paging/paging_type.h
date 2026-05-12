/*
 * File: paging_type.h
 * File Created: 05 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/paging/paging_types.h"

#include <types.h>

typedef struct paging_page
{
    void *page_dir;
    void *page_dir_phys;
} paging_page_t;

typedef struct
{
    virt_addr fault_addr; // CR2 on x86, FAR_EL1 on arm64
    paging_page_t page_directory;
    reg_t pc;
    reg_t sp;
    uint8_t write : 1;
    uint8_t user : 1;
    uint8_t present : 1;   // page not present vs protection violation
    uint8_t exec : 1;      // instruction fetch fault (NX/XD bit)
    uint8_t as_kernel : 1; // indicates if the active cr3 is the kernel
    uint8_t res : 3;
} paging_info;
