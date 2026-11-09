/*
 * File: vma.h
 * File Created: 05 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "vmat.h"

#include <types.h>

status_t do_mmap(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags, vma_t **out_vma);
vma_t *do_mmap_eager(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags);
status_t vma_allocate(vma_memory_t **out_vma);
status_t vma_clone(vma_memory_t *dest, vma_memory_t *src);
status_t vma_free(vma_memory_t *mm);