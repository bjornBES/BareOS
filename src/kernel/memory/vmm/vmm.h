/*
 * File: vmm.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "vmm_types.h"
#include "task/process_types.h"

#define USER_STACK_MAX      (8 * 1024 * 1024)   // 8MB hard limit

void vma_grow_down(vma_memory_t *mm, vma_t *vma);
void vma_grow_up(vma_memory_t *mm, vma_t *vma);

// find the VMA containing addr — the #PF handler's first call
vma_t *vma_find(vma_memory_t *mm, virt_addr addr);

// insert sorted by start
void vma_insert(vma_memory_t *mm, vma_t *new);
vma_memory_t *vma_clone(vma_memory_t *src, process_t *proc);
void vma_free(process_t *proc);

// mmap syscall bottom half
vma_t *do_mmap(vma_memory_t *mm, virt_addr addr, size_t size, vma_type_t type, memory_flags_t flags);

// munmap — unmaps pages and removes the VMA entry
int do_munmap(vma_memory_t *mm, virt_addr addr, size_t size);

void vma_init(process_t *proc);