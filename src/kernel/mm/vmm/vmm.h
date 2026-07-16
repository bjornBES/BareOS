/*
 * File: vmm.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vmm_types.h"
#include "task/process_types.h"

#define USER_STACK_MAX      (8 * 1024 * 1024)   // 8MB hard limit

// int vma_grow_down(vma_memory_t *mm, vma_t *vma);
// int vma_grow_up(vma_memory_t *mm, vma_t *vma);

int vma_grow(vma_memory_t *mm, vma_t *vma);
int vma_shrink(vma_memory_t *mm, vma_t *vma);

// find the VMA containing addr — the #PF handler's first call
vma_t *vma_find(vma_memory_t *mm, vaddr_t addr);

int vma_inside(vma_memory_t *mm, vaddr_t addr);

vaddr_t vma_find_free(vma_memory_t *mm, vaddr_t hint, size_t len, vaddr_t search_start, vaddr_t search_end);

// Splits vma into two VMAs at split_addr.
// Precondition: vma->start < split_addr < vma->end
// Returns pointer to the NEW vma covering [split_addr, old_end).
// The original vma is shrunk in place to [old_start, split_addr).
vma_t *vma_split(vma_memory_t *mm, vma_t *vma, vaddr_t split_addr);


// insert sorted by start
void vma_insert(vma_memory_t *mm, vma_t *new);
vma_memory_t *vma_clone(process_t *parent, process_t *proc);
void vma_free(process_t *proc);
void vma_destroy(process_t *proc);

// mmap syscall bottom half
vma_t *do_mmap(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags);
vma_t *do_mmap_eager(vma_memory_t *mm, vaddr_t addr, size_t size, vma_type_t type, mmu_flags_t flags);

// munmap — unmaps pages and removes the VMA entry
int do_munmap(vma_memory_t *mm, vaddr_t addr, size_t size);

void vma_init(process_t *proc);