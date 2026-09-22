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

#include "mm/allocator/memory_allocator.h"
#include "mm/pmm/pmm.h"
#include "mm/memdefs.h"

#include "asm/mmu_arch.h"

#include "debug/debug.h"

#include "memory.h"

#define MODULE "memory_allocator"

#define STATUS_ALLOCATED 0x01

vaddr_t last_alloc = 0;
vaddr_t heap_end = 0;
vaddr_t heap_begin = 0;
uint64_t memory_used = 0;
size_t heap_size;

status_t alloc_new_page()
{
	ENTER_FUNC("", 0);
    paddr_t phys = pmm_alloc_frame();
    vaddr_t virt = heap_end;  // next virtual page after current end
	mmu_arch_enable_prints();
    size_t size_mapped = mmu_arch_map(&kernel_page, virt, phys, kernel_data_flags);
	mmu_arch_disable_prints();
	if (!(size_mapped > KERRNO_ERRORS_END))
	{
		return size_mapped;
	}
	trace_debug(MODULE, "mapped %p..%p for %u page(s)", virt, phys, PAGE_COUNT(size_mapped));
	
    heap_size++;
	trace_debug(MODULE, "%p + (%u * %x) = %p", heap_begin, heap_size, size_mapped, heap_begin + (heap_size * size_mapped));
    heap_end = heap_begin + (heap_size * size_mapped);
	memset((void*)virt, 0, size_mapped);
	return KERRNO_SUCCESSES;
}

void allocator_init()
{
	trace_info(MODULE, "init allocator");
	heap_begin = MEMORY_HEAP_VIRT_BASE;
	heap_end = heap_begin;
	heap_size = 0;
	last_alloc = heap_begin;
	memory_used = 0;

	alloc_new_page();

	allocator_print_status();

	trace_info(MODULE, "Kernel heap starts at %p", heap_begin);
}

void allocator_print_status()
{
	trace_debug(MODULE, "Memory used: %d bytes", memory_used);
	trace_debug(MODULE, "Memory free: %d bytes", heap_end - heap_begin - memory_used);
	trace_debug(MODULE, "Heap size: %d bytes", heap_end - heap_begin);
	trace_debug(MODULE, "Heap start: %p", heap_begin);
	trace_debug(MODULE, "Heap end: %p", heap_end);
	trace_debug(MODULE, "last_alloc: %p", last_alloc);
}

void allocator_print_blocks()
{
	trace_debug(MODULE, "=== Heap Blocks ===");
	vaddr_t mem = heap_begin;
	int block_index = 0;

	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;

		if (!a->size)
			break; // no more blocks

		trace_debug(MODULE, "Block %d: addr=%p size=%u status=%s(%08b)",
				  block_index, (mem + sizeof(alloc_t)),
				  a->size, a->status ? "allocated" : "free", a->status);

		// move to next block
		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 0x10;
		block_index++;
	}
	trace_debug(MODULE, "=== End of Blocks ===");
}

status_t kfree(void *mem)
{
	if (mem == NULL)
	{
		KERRNO_RETURN(KERRNO_BAD_VALUE, "ptr was null");
	}
	alloc_t *alloc = (mem - sizeof(alloc_t));
	memory_used -= alloc->size + sizeof(alloc_t);
	alloc->status = 0;
	return KERRNO_SUCCESSES;
}

void *kmalloc(size_t size)
{
	if (!size)
	{
		KERRNO_NO_RETURN(KERRNO_BAD_VALUE, "size is zero");
		return NULL;
	}

	vaddr_t mem = heap_begin;
	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;
		// trace_debug(MODULE, "mem=%p-%p", mem, mem + a->size + sizeof(alloc_t));
		// trace_debug(MODULE, "mem=%p a={.status=%d, .size=%d}", mem, a->status, a->size);

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
			// trace_debug(MODULE, "RE: Allocated %d bytes from %p to %p", size, a, a + sizeof(alloc_t));
			// trace_debug(MODULE, "RE: Allocated %d bytes from %p to %p", size, a + sizeof(alloc_t), a + sizeof(alloc_t) + size);
			memory_used += size + 0x10 + sizeof(alloc_t);
			return (void *)(mem + sizeof(alloc_t));
		}

		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 0x10;
		if (mem >= heap_end)
		{
			if (alloc_new_page() != KERRNO_SUCCESSES)
			{
				KERRNO_NO_RETURN(KERRNO_OOM, "Out of memory");
				return NULL;
			}
		}
	}
	
	if (last_alloc + size + sizeof(alloc_t) >= heap_end)
	{
		while (last_alloc + size + sizeof(alloc_t) >= heap_end)
		{
			if (alloc_new_page() != KERRNO_SUCCESSES)
			{
				KERRNO_NO_RETURN(KERRNO_OOM, "Out of memory");
				return NULL;
			}
		}
		// KERNEL_PANIC(MODULE, "Cannot allocate %d bytes! Out of memory.", size);
	}
	// pmm_print_info_verbose();

	alloc_t *alloc = (alloc_t *)last_alloc;
	alloc->status |= STATUS_ALLOCATED;
	alloc->size = size;

	last_alloc += size;
	last_alloc += sizeof(alloc_t);
	last_alloc += 0x10;

	// trace_debug(MODULE, "Allocated %d bytes from %p to %p", size, alloc, alloc + sizeof(alloc_t));
	// trace_debug(MODULE, "Allocated %d bytes from %p to %p", size, alloc + sizeof(alloc_t), alloc + sizeof(alloc_t) + size);

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
int free(void* ptr)
{
	if (ptr == NULL)
	{
		KERRNO_RETURN(KERRNO_BAD_VALUE, "ptr was null");
	}
    return kfree(ptr);
}
void* calloc(size_t num, size_t size)
{
    return kcalloc(num, size);
}
void* realloc(void* ptr, size_t size)
{
    return krealloc(ptr, size);
}

