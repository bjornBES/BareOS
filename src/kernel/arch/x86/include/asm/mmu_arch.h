/*
 * File: mmu_arch.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once


/// @defgroup contract_arch_x86_mmu
/// @brief the contract functions that the kernel will call to use the cpu's mmu.

/// @ingroup contract_arch_x86_mmu
/// @{
    
#include "page_arch.h"
#include "mm/mmu.h"
#include <types.h>
#include <boot/params.h>

extern page_table_t kernel_page;

/// @brief Will make the kernel mappings in kernel_page and will also load then
///
/// The mappings need to cover the kernel and the PMM.
/// The kernel needs to be mapped from the physical address bp->kernel_phys_base,
/// to the virtual address bp->kernel_virt_base.
/// The PMM needs to be mapped from the physical address pmm_phys_start
/// to the virtual address 0xFFFF800000000000.
///
/// @param[in] bp The bootparams from the bootloader.
void mmu_arch_init(boot_params_t *bp);

// --- Address Space ---

/// @brief create a new empty address space, returns physical address of top level table
/// @return The new page table or an errno number
/// @retval ENOMEM: The PMM is out of memory
page_table_t *mmu_arch_create_table();

/// @brief destroy an address space and free all arch-owned paging structures
/// @param[in] table The table to destroy
/// @return 0 on successes or an errno number
/// @retval EINVAL: table is not valid
int mmu_arch_destroy_table(page_table_t *table);

/// @brief switch to the given table
/// @param[in] table The table to load
/// @return 0 on successes or an errno number
/// @retval EINVAL: table is not valid
int mmu_arch_load_table(page_table_t *table);

/// @brief get currently active address space
/// @param[out] table The currently active page table
/// @return 0 on successes or an errno number
/// @retval EINVAL: table is not valid
int mmu_arch_current_table(page_table_t *table);

// --- Kernel Mapping ---

/// @brief map kernel pages into a userspace table (for higher half kernel)
/// @param[in] table The table that the kernel table will be copied into.
/// @return 0 on successes or an errno number
/// @retval EINVAL: table is not valid
int mmu_arch_copy_from_kernel(page_table_t *table);

/// @brief map pages from src into the table dst
/// @param[in] dst The destination table
/// @param[in] src The source table
/// @return 0 on successes or an errno number
/// @retval EINVAL: dst or src is not valid
int mmu_arch_copy_from(page_table_t *dst, page_table_t *src);

/// @brief map a single physical page to a virtual address with given flags
/// creates intermediate tables as needed
///
/// @param[in] table The page table to make the mapping into
/// @param[in] virt The virtual address that needs to be mapped to a physical address
/// @param[in] phys The physical address that needs to be mapped to a virtual address
/// @param[in] flags The flags of the leaf entry
/// @return The size of the mapped page size or an errno number
/// @retval EPERM: phys or virt is not valid e.g. They are too large or not page aligned
/// @retval EINVAL: flags or table are not valid
/// @retval EFAULT: virt exceeds the virtual space of the CPU
/// @retval EACCES: flags requests for execution with write (W^X)
/// @retval ENOMEM: The PMM is out of memory
size_t mmu_arch_map(page_table_t *table, vaddr_t virt, paddr_t phys, mmu_flags_t flags);

/// @brief unmap a single virtual page and will also free the physical frames
/// @param[in] table The page table where the entries will be freed from
/// @param[in] virt The virtual address that will be freed
/// @return the physical address of the mapped page on successes or an errno number
/// @retval EPERM: virt is not valid e.g. It is too large or not page aligned
/// @retval EINVAL: table is not valid
/// @retval EFAULT: virt exceeds the virtual space of the CPU
/// @retval ENOENT: no entry exists at that virtual address
paddr_t mmu_arch_unmap(page_table_t *table, vaddr_t virt);

/// @brief change flags on an existing mapping without remapping
/// @param[in] table The page table where the entry is in
/// @param[in] virt The virtual address that the entry is mapped to
/// @param[in] flags The flags that will change the old once
/// @return 0 on successes or an errno number
/// @retval EPERM: virt is not valid e.g. It is too large or not page aligned
/// @retval EINVAL: flags or table are not valid
/// @retval EFAULT: virt exceeds the virtual space of the CPU
/// @retval EACCES: flags requests for execution with write (W^X)
/// @retval ENOENT: no entry exists at that virtual address
int mmu_arch_protect(page_table_t *table, vaddr_t virt, mmu_flags_t flags);

/// @brief walk the page table and finds the physical address mapped at virt
/// @param[in] table The page table where the entry is in
/// @param[in] virt The virtual address to be translated
/// @return the physical address mapped at virt on successes or an errno number
/// @retval EPERM: virt is not valid e.g. It is too large or not page aligned
/// @retval EINVAL: table is not valid
/// @retval EFAULT: virt exceeds the virtual space of the CPU
/// @retval ENOENT: no entry exists at that virtual address
paddr_t mmu_arch_virt_to_phys(page_table_t *table, vaddr_t virt);

/// @brief checks if the entry mapped to the virtual address is present
/// @param[in] table The page table where the entry is in
/// @param[in] virt The virtual address of the entry
/// @return 0 on successes or an errno number
/// @retval EPERM: virt is not valid e.g. It is too large or not page aligned
/// @retval EINVAL: table are not valid
/// @retval EFAULT: virt exceeds the virtual space of the CPU
/// @retval ENOENT: entry does not exist in the given table
int mmu_arch_is_present(page_table_t *table, vaddr_t virt);

// --- TLB ---

/// @brief flushes the tbl entry at virt
/// @param[in] virt the virtual address to reload
void mmu_arch_flush_page(vaddr_t virt);

/// @brief flushes all the tbl entries
void mmu_arch_flush_all();

// --- Diagnostic ---

/// @brief disables logs from the mmu
void mmu_arch_disable_prints();
/// @brief enables logs from the mmu
void mmu_arch_enable_prints();

/// @brief sets the disable logs with disable
/// @param[in] disable true=disable logs, false=enable logs
void mmu_arch_set_prints(bool disable);

/// @brief Checks if prints are disabled
/// @retval true if prints are disabled
/// @retval false if prints are enabled
bool mmu_arch_is_prints_disable();

// int mmu_arch_query(page_table_t *table, vaddr_t virt, mmu_mapping_t *out);
// void mmu_arch_walk(page_table_t *table, mmu_walk_cb_t cb, void *ctx);

/// @brief Gets the faulting address in a fault
/// @return the faulting virtual address
vaddr_t mmu_arch_get_faulting_address();

/// @brief walk the page table and return the virtual address mapped at phys
/// @param[in] table The page table where the entry is in
/// @param[in] phys The physical address to be translated
/// @return the virtual address mapped at virt on successes or an errno number
/// @retval EPERM: phys is not valid e.g. It is too large or not page aligned
/// @retval EINVAL: table is not valid
/// @retval ENOENT: no entry exists at that physical address
vaddr_t mmu_arch_phys_to_virt(page_table_t *table, paddr_t phys);

/// @brief Will print all non-zero entries to the debug output
/// @param[in] table The page table to dump 
void paging_print_tree(page_table_t *table);

/// @}