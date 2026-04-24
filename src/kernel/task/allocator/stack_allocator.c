/*
 * File: stack_allocator.c
 * File Created: 18 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stack_allocator.h"
#include "memory/paging/paging.h"
#include "debug/debug.h"
#include "kernel.h"

virt_addr stack_alloc_init(process *proc) {
    virt_addr stack_base = USER_STACK_TOP - USER_STACK_INITIAL;

    void *frame = (void*)paging_alloc_frame();
    // log_debug ("STACK", "got frame %p", frame);
    // log_debug ("STACK", "paging in process page dir");
    // log_debug ("STACK", "mapping %p -> %p", frame, stack_base);
    paging_map_page(proc->page_dir_virt, (void*)stack_base, (void*)frame, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

    proc->stack_top  = (virt_addr)USER_STACK_TOP;
    proc->stack_size = USER_STACK_INITIAL;

    // ESP points to top, 16-byte aligned, leave 4 bytes as a soft guard
    return (virt_addr)USER_STACK_TOP - sizeof(uint32_t);
}

int stack_grow(process *proc) {
    if (proc->stack_size + PAGE_SIZE > USER_STACK_MAX)
        return RETURN_FAILED;

    virt_addr new_page = proc->stack_top - proc->stack_size - PAGE_SIZE;

    phys_addr frame = paging_alloc_frame();
    paging_map_page(proc->page_dir_virt, (virt_addr)new_page, frame, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

    proc->stack_size += PAGE_SIZE;
    return RETURN_GOOD;
}

void stack_free(process *proc) {
    virt_addr stack_base = (virt_addr)proc->stack_top - proc->stack_size;

    for (virt_addr addr = stack_base; addr < proc->stack_top; addr += PAGE_SIZE)
    {
        phys_addr frame = paging_get_physical(proc->page_dir_virt, addr);
        paging_unmap_page(proc->page_dir_virt, addr);
        paging_free_frame(frame);
    }

    proc->stack_top  = 0;
    proc->stack_size = 0;
}
