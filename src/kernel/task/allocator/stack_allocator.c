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
#include "arch/x86/paging/paging.h"
#include "debug/debug.h"
#include "kernel.h"

uint32_t stack_alloc_init(process *proc) {
    uint32_t stack_base = USER_STACK_TOP - USER_STACK_INITIAL;

    // void *frame = (void*)frame_alloc_frame();
    // log_debug ("STACK", "got frame %p", frame);
    // log_debug ("STACK", "paging in process page dir");
    // log_debug ("STACK", "mapping %p -> %p", frame, stack_base);
    // paging_map_page(proc->page_dir_phys, (void*)stack_base, (void*)frame, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

    proc->stack_top  = USER_STACK_TOP;
    proc->stack_size = USER_STACK_INITIAL;

    // ESP points to top, 16-byte aligned, leave 4 bytes as a soft guard
    return USER_STACK_TOP - sizeof(uint32_t);
}

int stack_grow(process *proc) {
    if (proc->stack_size + PAGE_SIZE > USER_STACK_MAX)
        return RETURN_FAILED;

    uint32_t new_page = proc->stack_top - proc->stack_size - PAGE_SIZE;

    // uint32_t frame = frame_alloc_frame();
    // paging_map_page(proc->page_dir_phys, (void*)new_page, (void*)frame, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

    proc->stack_size += PAGE_SIZE;
    return RETURN_GOOD;
}

void stack_free(process *proc) {
    uint32_t stack_base = proc->stack_top - proc->stack_size;

    for (uint32_t addr = stack_base; addr < proc->stack_top; addr += PAGE_SIZE)
    {
        // uint32_t frame = (uint32_t)paging_get_physical(proc->page_dir_virt, (void*)addr);
        // paging_unmap_page(proc->page_dir_phys, (void*)addr);
        // frame_free_frame(frame);
    }

    proc->stack_top  = 0;
    proc->stack_size = 0;
}
