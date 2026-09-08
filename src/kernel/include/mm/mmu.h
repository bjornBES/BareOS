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
#include "type_arch.h"

typedef struct page_table
{
    vaddr_t page_dir;
    paddr_t page_dir_phys;
} page_table_t;

typedef struct mmu_flags
{
    // indicates the entry needs to be present
    uint8_t present     : 1;

    // indicates the entry needs to be readable
    uint8_t read        : 1;

    // indicates the entry needs to be writable
    uint8_t write       : 1;

    // indicates the entry needs to be executable
    uint8_t exec        : 1;

    // indicates the entry is a user mode entry
    uint8_t user        : 1;

    // indicates that the entry shouldn't be cached
    uint8_t cached      : 1;

    // indicates that the entry shouldn't be flushed when flushing pages
    uint8_t global      : 1;

    /* VMA policy */

    // indicates that the VMA entry is going down in the address space
    uint8_t grows_down  : 1;

    // indicates that the entry is marked with copy on write
    uint8_t cow         : 1;

    // indicates that the entry is not yet allocate in the pmm
    uint8_t allocatable : 1;

    /* MMU policy not used by the kernel*/

    // indicates that the entry is a large entry if the mmu has support for it
    // note: Arch-owned: only arch-specific mapping code sets or interprets this. Generic kernel code must always pass this as 0 for kernel mappings.
    uint8_t large : 1;

    uint8_t reserved1    : 5;
} mmu_flags_t;

extern mmu_flags_t flags_none;

// kernel .text
extern mmu_flags_t kernel_text_flags;

// kernel .data / heap
extern mmu_flags_t kernel_data_flags;

// smp trampoline mapping
extern mmu_flags_t trampoline_flags;

// vdso segment
extern mmu_flags_t vdso_text_flags;

// kernel stack
extern mmu_flags_t kernel_stack_flags;

// user .text
extern mmu_flags_t text_flags;

// user stack
extern mmu_flags_t stack_flags;

// user .data / heap
extern mmu_flags_t data_flags;

// user .rodata
extern mmu_flags_t rodata_flags;

// MMIO
extern mmu_flags_t mmio_flags;

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
    uint8_t write     : 1;

    // indicates a user fault
    uint8_t user      : 1;

    // page not present vs protection violation
    uint8_t present   : 1;

    // indicates a instruction fetch fault
    uint8_t fetch     : 1;

    // indicates if the active page directory is the kernel
    uint8_t as_kernel : 1;

    // indicates if the faulting page/vma had cow enabled
    uint8_t is_cow    : 1;

    uint8_t res       : 2;
} mmu_fault_info;

int mmu_page_fault_handler(intr_frame_t *regs, mmu_fault_info *info);

void mmu_copy_contents(paddr_t src, paddr_t dst);

/// @brief Map a virtual address to a allocated free frame with the given flags
/// the contiguous region virt and virt+size, the function will only allocate one physical frames, the mapping will be done with mmu_arch_map
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual start address of the contiguous regions that needs to be mapped to the physical contiguous addresses
/// @param[in] phys The physical start address of the contiguous regions that needs to be mapped to the virtual contiguous addresses
/// @param[in] size The size (in bytes) that will be mapped this will be aligned to the nearest PAGE_SIZE by the function
/// @param[in] flags The flags of the leaf entry
/// @return The physical address of the mapping or an errno number
/// @retval EPERM: virt or size are not valid e.g. They are too large or not page aligned
/// @retval EINVAL: flags, size or table are not valid
/// @retval EFAULT: virt or virt+size exceeds the virtual space of the CPU
/// @retval EACCES: flags requests for execution with write (W^X)
/// @retval ENOMEM: The PMM is out of memory
paddr_t mmu_alloc_and_map(page_table_t *table, vaddr_t virt, mmu_flags_t flags);

/// @brief Map a contiguous virtual region to newly allocated physical frames with the given flags
/// the contiguous region is virt and virt+size, the physical frames will be contiguous, the mapping will be done with mmu_arch_map
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual start address of the contiguous regions that needs to be mapped to the contiguous physical frames
/// @param[in] size The size (in bytes) that will be mapped this will be aligned to the nearest PAGE_SIZE by the function
/// @param[in] flags The flags of the leaf entry
/// @return The physical address of the mapped region or an errno number
/// @retval EPERM: virt or size are not valid e.g. They are too large or not page aligned
/// @retval EINVAL: flags, size or table are not valid
/// @retval EFAULT: virt or virt+size exceeds the virtual space of the CPU
/// @retval EACCES: flags requests for execution with write (W^X)
/// @retval ENOMEM: The PMM is out of memory
paddr_t mmu_alloc_and_map_region(page_table_t *table, vaddr_t virt, size_t size, mmu_flags_t flags);

/// @brief Map a contiguous physical region to a contiguous virtual address with the given flags
/// the contiguous region is phys to phys+size and virt and virt+size, the mapping will be done with mmu_arch_map
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual start address of the contiguous regions that needs to be mapped to the physical contiguous addresses
/// @param[in] phys The physical start address of the contiguous regions that needs to be mapped to the virtual contiguous addresses
/// @param[in] size The size (in bytes) that will be mapped this will be aligned to the nearest PAGE_SIZE by the function
/// @param[in] flags The flags of the leaf entry
/// @return The size of the mapped region or an errno number
/// @retval EPERM: phys, virt or size are not valid e.g. They are too large or not page aligned
/// @retval EINVAL: flags, size or table are not valid
/// @retval EFAULT: virt or virt+size exceeds the virtual space of the CPU
/// @retval EACCES: flags requests for execution with write (W^X)
/// @retval ENOMEM: The PMM is out of memory
size_t mmu_map_region(page_table_t *table, vaddr_t virt, paddr_t phys, size_t size, mmu_flags_t flags);

// Unmap [virt, virt + size) and free the backing frames.
int mmu_free_region(page_table_t *table, vaddr_t virt, size_t size);
