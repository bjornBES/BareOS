/*
 * File: mmu.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct mmu_flags
{
    // indicates the entry needs to be present
    uint8_t present : 1;

    // indicates the entry needs to be readable
    uint8_t read : 1;

    // indicates the entry needs to be writable
    uint8_t write : 1;

    // indicates the entry needs to be executable
    uint8_t exec : 1;

    // indicates the entry is a user mode entry
    uint8_t user : 1;

    // indicates that the entry shouldn't be cached
    uint8_t cached : 1;

    // indicates that the entry shouldn't be flushed when flushing pages
    uint8_t global : 1;

    /* VMA policy */

    // indicates that the VMA entry is going down in the address space
    uint8_t grows_down : 1;
    
    // indicates that the entry is marked with copy on write
    uint8_t cow : 1;
    
    // indicates that the entry is not yet allocate in the pmm
    uint8_t allocatable : 1;
    
    uint8_t reserved : 6;
} mmu_flags_t;

typedef struct
{
    // the virtual address of the entry
    vaddr_t virt;
    
    // target physical address of that entry
    paddr_t phys;
    
    // translated back to generic flags
    mmu_flags_t flags;
    
    // how many level deep is this entry
    uint8_t levels;
} mmu_mapping_t;

typedef struct
{
    // the faulting address
    vaddr_t fault_addr;
    
    // the page directory that was on
    page_table_t page_directory;

    // the pc at fault time
    reg_t pc;
    
    // the sp at fault time
    reg_t sp;

    // the flags on the faulting page
    mmu_flags_t entry_flags;
    
    // indicates a write fault
    uint8_t write : 1;

    // indicates a user fault
    uint8_t user : 1;

    // page not present vs protection violation
    uint8_t present : 1;
    
    // indicates a instruction fetch fault
    uint8_t fetch : 1;
    
    // indicates if the active page directory is the kernel
    uint8_t as_kernel : 1;
    
    // indicates if the faulting page/vma had cow enabled
    uint8_t is_cow : 1;
    
    uint8_t res : 2;
} mmu_fault_info;
