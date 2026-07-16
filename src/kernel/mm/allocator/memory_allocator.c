/*
 * File: memory_allocator.c
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

// credit to: https://github.com/levex/osdev for the core idea
// https://github.com/levex/osdev/blob/master/memory/malloc.c

#include "memory_allocator.h"
#include "debug/debug.h"
#include "kernel/memory.h"

#include "mm/pmm.h"
#include "mm/memdefs.h"
#include "mm/mmu/mmu.h"

#define MODULE "memory_allocator"

#define STATUS_ALLOCATED 0x01

vaddr_t last_alloc = 0;
vaddr_t heap_end = 0;
vaddr_t heap_begin = 0;
uint64_t memory_used = 0;
size_t heap_size;

void alloc_new_page()
{
    paddr_t phys = pmm_alloc_heap_frame();
    vaddr_t virt = heap_end;  // next virtual page after current end
	log_debug(MODULE, "mapping %p..%p for 4 page", virt, phys);
    mmu_map_region(&kernel_page, virt, phys, HEAP_FRAME, kernel_data_flags);
	
    heap_size++;
	log_debug(MODULE, "%p + (%u * %x) = %p", heap_begin, heap_size, HEAP_FRAME, heap_begin + (heap_size * HEAP_FRAME));
    heap_end = heap_begin + (heap_size * HEAP_FRAME);
	memset((void*)virt, 0, HEAP_FRAME);
}

void allocator_init()
{
	log_info(MODULE, "init allocator");
	heap_begin = MEMORY_HEAP_VIRT_BASE;
	heap_end = heap_begin;
	heap_size = 0;
	last_alloc = heap_begin;
	memory_used = 0;

	alloc_new_page();

	allocator_print_status();

	log_info(MODULE, "Kernel heap starts at %p", heap_begin);
}

void allocator_print_status()
{
	log_debug(MODULE, "Memory used: %d bytes", memory_used);
	log_debug(MODULE, "Memory free: %d bytes", heap_end - heap_begin - memory_used);
	log_debug(MODULE, "Heap size: %d bytes", heap_end - heap_begin);
	log_debug(MODULE, "Heap start: %p", heap_begin);
	log_debug(MODULE, "Heap end: %p", heap_end);
	log_debug(MODULE, "last_alloc: %p", last_alloc);
}

void allocator_print_blocks()
{
	log_debug(MODULE, "=== Heap Blocks ===");
	vaddr_t mem = heap_begin;
	int block_index = 0;

	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;

		if (!a->size)
			break; // no more blocks

		log_debug(MODULE, "Block %d: addr=%p size=%u status=%s(%08b)",
				  block_index, (mem + sizeof(alloc_t)),
				  a->size, a->status ? "allocated" : "free", a->status);

		// move to next block
		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 0x10;
		block_index++;
	}
	log_debug(MODULE, "=== End of Blocks ===");
}

void kfree(void *mem)
{
	alloc_t *alloc = (mem - sizeof(alloc_t));
	memory_used -= alloc->size + sizeof(alloc_t);
	alloc->status = 0;
}

void *kmalloc(size_t size)
{
	if (!size)
	{
		return 0;
	}

	vaddr_t mem = heap_begin;
	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;
		// log_debug(MODULE, "mem=%p-%p", mem, mem + a->size + sizeof(alloc_t));
		// log_debug(MODULE, "mem=%p a={.status=%d, .size=%d}", mem, a->status, a->size);

		if (!a->size)
		{
			break;
		}

		if (a->status & STATUS_ALLOCATED)
		{
			mem += a->size;
			mem += sizeof(alloc_t);
			mem += 0x10;
			continue;
		}

		if (a->size >= size && !(a->status & STATUS_ALLOCATED))
		{
			a->status |= STATUS_ALLOCATED;
			// log_debug(MODULE, "RE: Allocated %d bytes from %p to %p", size, a, a + sizeof(alloc_t));
			// log_debug(MODULE, "RE: Allocated %d bytes from %p to %p", size, a + sizeof(alloc_t), a + sizeof(alloc_t) + size);
			memory_used += size + 0x10 + sizeof(alloc_t);
			return (void *)(mem + sizeof(alloc_t));
		}

		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 0x10;
		if (mem >= heap_end)
		{
			alloc_new_page();
		}
	}
	
	if (last_alloc + size + sizeof(alloc_t) >= heap_end)
	{
		while (last_alloc + size + sizeof(alloc_t) >= heap_end)
		{
			alloc_new_page();
		}
		// KernelPanic(MODULE, "Cannot allocate %d bytes! Out of memory.", size);
	}
	// pmm_print_info_verbose();

	alloc_t *alloc = (alloc_t *)last_alloc;
	alloc->status |= STATUS_ALLOCATED;
	alloc->size = size;

	last_alloc += size;
	last_alloc += sizeof(alloc_t);
	last_alloc += 0x10;

	// log_debug(MODULE, "Allocated %d bytes from %p to %p", size, alloc, alloc + sizeof(alloc_t));
	// log_debug(MODULE, "Allocated %d bytes from %p to %p", size, alloc + sizeof(alloc_t), alloc + sizeof(alloc_t) + size);

	memory_used += size + 0x10 + sizeof(alloc_t);
	return (char *)((vaddr_t)alloc + sizeof(alloc_t));
}

void *kcalloc(size_t num, size_t size)
{
	size_t total_size = num * size;
	void *ptr = kmalloc(total_size);
	if (ptr)
	{
		memset(ptr, 0, total_size);
		return ptr;
	}
	return NULL;
}
void *krealloc(void *ptr, size_t size)
{
	void *new_ptr = kmalloc(size);
	if (new_ptr)
	{
		alloc_t *old = (alloc_t *)((vaddr_t)ptr - sizeof(alloc_t));
		memcpy(new_ptr, ptr, old->size < size ? old->size : size);
		kfree(ptr);
	}
	return new_ptr;
}

void* malloc(size_t size)
{
    return kmalloc(size);
}
void free(void* ptr)
{
    kfree(ptr);
}
void* calloc(size_t num, size_t size)
{
    return kcalloc(num, size);
}
void* realloc(void* ptr, size_t size)
{
    return krealloc(ptr, size);
}

void *kmalloc_phys(size_t size, void **virt_out)
{
	void *virt = kmalloc(size);
	log_debug(MODULE, "malloc returned %p", virt);
	memset(virt, 0, size);
	*virt_out = virt;
	paddr_t phys = mmu_arch_virt_to_phys(&kernel_page, (vaddr_t)virt);
	log_debug(MODULE, "p%p = kmalloc_phys(%u, v%p)", phys, size, virt);
	return (void*)phys;
}
void *kcalloc_phys(size_t num, size_t size, void **virt_out)
{
	void *virt = kcalloc(num, size);
	memset(virt, 0, size * num);
	*virt_out = virt;
	return (void*)mmu_arch_virt_to_phys(&kernel_page, (vaddr_t)virt);
}

