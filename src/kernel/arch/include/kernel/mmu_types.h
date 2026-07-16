/*
 * File: mmu_types.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct mmu_flags
{
    uint8_t present : 1;
    uint8_t write : 1;
    uint8_t exec : 1;
    uint8_t user : 1;
    uint8_t cached : 1;
    uint8_t global : 1;
    uint8_t grows_down : 1;
    uint8_t cow : 1;
    uint8_t reserved : 8;
} mmu_flags_t;

#define FLAGS_TO_RAW(flags) ((uint16_t)*((uint8_t*)&flags))

//0x19

typedef struct page_table
{
    vaddr_t page_dir;
    paddr_t page_dir_phys;
} page_table_t;

typedef struct
{
    vaddr_t virt;
    paddr_t phys;
    mmu_flags_t flags; // translated back to generic flags
    uint8_t levels;    // how many levels deep (4 on x86-64)
    bool present;
    bool huge;         // was this a large/huge page mapping
} mmu_mapping_t;

typedef struct
{
    vaddr_t fault_addr; // CR2 on x86, FAR_EL1 on arm64
    page_table_t page_directory;
    reg_t pc;
    reg_t sp;
    mmu_flags_t entry_flags;
    uint8_t write : 1;
    uint8_t user : 1;
    uint8_t present : 1;   // page not present vs protection violation
    uint8_t exec : 1;      // instruction fetch fault (NX/XD bit)
    uint8_t as_kernel : 1; // indicates if the active cr3 is the kernel
    uint8_t is_cow : 1;
    uint8_t res : 2;
} mmu_fault_info;

typedef bool (*mmu_walk_cb_t)(mmu_mapping_t *mapping, void *ctx);
