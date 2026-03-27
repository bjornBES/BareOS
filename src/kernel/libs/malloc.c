/*
 * File: malloc.c
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

// credit to: https://github.com/levex/osdev for the implantation

#include "malloc.h"
#include "debug/debug.h"

#include "memory.h"

#define MODULE "malloc"

#define MAX_PAGE_ALIGNED_ALLOCS 32

virt_addr last_alloc = 0;
virt_addr heap_end = 0;
virt_addr heap_begin = 0;
uint64_t memory_used = 0;
size_t heap_size;
void heap_init(virt_addr heap_start, size_t _heap_size)
{
	heap_size = _heap_size;
	last_alloc = heap_start + 0x1000;
	heap_begin = last_alloc;
	heap_end = heap_begin + heap_size;
	
	// void* heap_begin_phys = paging_get_physical(kernel_page, (void*)heap_begin);
	// log_debug(MODULE, "Kernel heap phys at p%p -> v%p", heap_begin_phys, heap_begin);
	// frame_alloc_region((void*)heap_begin_phys, (void*)(heap_begin_phys + heap_size));

	log_debug(MODULE, "Kernel heap starts at %x", heap_begin);
}

void mmInit()
{
	// void* heap_begin_phys = paging_get_physical(kernel_page, (void*)heap_begin);
    // paging_map_region(kernel_page, (void*)heap_begin, heap_begin_phys, heap_size, -1);
	// memset((char *)heap_begin, 0, heap_size);
}

void mmPrintStatus()
{
	log_debug(MODULE, "Memory used: %d bytes", memory_used);
	log_debug(MODULE, "Memory free: %d bytes", heap_end - heap_begin - memory_used);
	log_debug(MODULE, "Heap size: %d bytes", heap_end - heap_begin);
	log_debug(MODULE, "Heap start: 0x%x", heap_begin);
	log_debug(MODULE, "Heap end: 0x%x", heap_end);
}

void mmPrintBlocks()
{
	log_debug(MODULE, "=== Heap Blocks ===");
	virt_addr mem = heap_begin;
	int block_index = 0;

	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;

		if (!a->size)
			break; // no more blocks

		log_debug(MODULE, "Block %d: addr=0x%x size=%u status=%s",
				  block_index, (uint64_t)(mem + sizeof(alloc_t)),
				  a->size, a->status ? "allocated" : "free");

		// move to next block
		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 4; // your allocator adds padding
		block_index++;
	}
	log_debug(MODULE, "=== End of Blocks ===");
}

void free(void *mem)
{
	alloc_t *alloc = (mem - sizeof(alloc_t));
	memory_used -= alloc->size + sizeof(alloc_t);
	alloc->status = 0;
}

void *malloc(size_t size)
{
	if (!size)
		return 0;

	/* Loop through blocks and find a block sized the same or bigger */
	virt_addr mem = heap_begin;
	while (mem < last_alloc)
	{
		alloc_t *a = (alloc_t *)mem;
		/* If the alloc has no size, we have reaced the end of allocation */
		// log_debug(MODULE, "mem=0x%x a={.status=%d, .size=%d}", mem, a->status, a->size);
		if (!a->size)
			goto nalloc;
		/* If the alloc has a status of 1 (allocated), then add its size
		 * and the sizeof alloc_t to the memory and continue looking.
		 */
		if (a->status)
		{
			mem += a->size;
			mem += sizeof(alloc_t);
			mem += 4;
			continue;
		}
		/* If the is not allocated, and its size is bigger or equal to the
		 * requested size, then adjust its size, set status and return the location.
		 */
		if (a->size >= size)
		{
			/* Set to allocated */
			a->status = 1;

			// log_debug(MODULE, "Allocated %d bytes from 0x%x to 0x%x", size, mem + sizeof(alloc_t), mem + sizeof(alloc_t) + size);
			memset(mem + sizeof(alloc_t), 0, size);
			memory_used += size + sizeof(alloc_t);
			return (char *)(mem + sizeof(alloc_t));
		}
		/* If it isn't allocated, but the size is not good, then
		 * add its size and the sizeof alloc_t to the pointer and
		 * continue;
		 */
		mem += a->size;
		mem += sizeof(alloc_t);
		mem += 4;
	}

nalloc:;
	if (last_alloc + size + sizeof(alloc_t) >= heap_end)
	{
		// set_task(0);
		KernelPanic(MODULE, "Cannot allocate %d bytes! Out of memory.", size);
	}
	alloc_t *alloc = (alloc_t *)last_alloc;
	alloc->status = 1;
	alloc->size = size;

	last_alloc += size;
	last_alloc += sizeof(alloc_t);
	last_alloc += 4;
	// log_debug(MODULE, "Allocated %d bytes from 0x%x to 0x%x", size, (uint32_t)alloc + sizeof(alloc_t), last_alloc);
	memory_used += size + 4 + sizeof(alloc_t);
	memset((char *)((uint64_t)alloc + sizeof(alloc_t)), 0, size);
	return (char *)((uint64_t)alloc + sizeof(alloc_t));
}

void *calloc(size_t num, size_t size)
{
	size_t total_size = num * size;
	void *ptr = malloc(total_size);
	if (ptr)
	{
		memset(ptr, 0, size);
		return ptr;
	}
	return NULL;
}
void *realloc(void *ptr, size_t size)
{
	if (!ptr)
	{
		return malloc(size); // If ptr is NULL, realloc behaves like malloc
	}

	if (size == 0)
	{
		free(ptr); // If size is 0, realloc behaves like free
		return NULL;
	}

	// Since we are not handling memory copying in this simple version,
	// this part would require more advanced management for full support.
	return malloc(size); // For now, just treat realloc as malloc.
}

void *kmalloc_phys(size_t size, void **virt_out)
{
/* 	void *virt = malloc(size);
	memset(virt, 0, size);
	*virt_out = virt;
	void *phys = paging_get_physical(kernel_page, virt);
	log_debug(MODULE, "p%p = kmalloc_phys(%u, v%p)", phys, size, virt);
	return phys; */
	return NULL;
}
void *kcalloc_phys(size_t num, size_t size, void **virt_out)
{
/* 	void *virt = calloc(num, size);
	memset(virt, 0, size * num);
	*virt_out = virt;
	return paging_get_physical(kernel_page, virt); */
	return NULL;
}
