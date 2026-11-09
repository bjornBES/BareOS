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

#include "mmut.h"

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
/// @return The physical address of the mapping or an kerrno number
/// @retval KERRNO_NOT_ALLOWED: virt or size are not valid e.g. They are too large or not page aligned
/// @retval KERRNO_BAD_VALUE: flags, size or table are not valid
/// @retval KERRNO_BAD_ADDRESS: virt or virt+size exceeds the virtual space of the CPU
/// @retval KERRNO_PERMISSION_DENIED: flags requests for execution with write (W^X)
/// @retval KERRNO_POSIX_ENOMEM: The PMM is out of memory
paddr_t mmu_alloc_and_map(page_table_t *table, vaddr_t virt, mmu_flags_t flags);

/// @brief Map a contiguous virtual region to newly allocated physical frames with the given flags
/// the contiguous region is virt and virt+size, the physical frames will be contiguous, the mapping will be done with mmu_arch_map
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual start address of the contiguous regions that needs to be mapped to the contiguous physical frames
/// @param[in] size The size (in bytes) that will be mapped this will be aligned to the nearest PAGE_SIZE by the function
/// @param[in] flags The flags of the leaf entry
/// @return The physical address of the mapped region or an kerrno number
/// @retval KERRNO_NOT_ALLOWED: virt or size are not valid e.g. They are too large or not page aligned
/// @retval KERRNO_BAD_VALUE: flags, size or table are not valid
/// @retval KERRNO_BAD_ADDRESS: virt or virt+size exceeds the virtual space of the CPU
/// @retval KERRNO_PERMISSION_DENIED: flags requests for execution with write (W^X)
/// @retval KERRNO_POSIX_ENOMEM: The PMM is out of memory
paddr_t mmu_alloc_and_map_region(page_table_t *table, vaddr_t virt, size_t size, mmu_flags_t flags);

/// @brief Map a contiguous physical region to a contiguous virtual address with the given flags
/// the contiguous region is phys to phys+size and virt and virt+size, the mapping will be done with mmu_arch_map
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual start address of the contiguous regions that needs to be mapped to the physical contiguous addresses
/// @param[in] phys The physical start address of the contiguous regions that needs to be mapped to the virtual contiguous addresses
/// @param[in] size The size (in bytes) that will be mapped this will be aligned to the nearest PAGE_SIZE by the function
/// @param[in] flags The flags of the leaf entry
/// @return The size of the mapped region or an kerrno number
/// @retval KERRNO_NOT_ALLOWED: phys, virt or size are not valid e.g. They are too large or not page aligned
/// @retval KERRNO_BAD_VALUE: flags, size or table are not valid
/// @retval KERRNO_BAD_ADDRESS: virt or virt+size exceeds the virtual space of the CPU
/// @retval KERRNO_PERMISSION_DENIED: flags requests for execution with write (W^X)
/// @retval KERRNO_POSIX_ENOMEM: The PMM is out of memory
size_t mmu_map_region(page_table_t *table, vaddr_t virt, paddr_t phys, size_t size, mmu_flags_t flags);

// Unmap [virt, virt + size) and free the backing frames.
int mmu_free_region(page_table_t *table, vaddr_t virt, size_t size);
