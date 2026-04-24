/*
 * File: malloc.c
 * File Created: 07 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "malloc.h"
#include "debug/debug.h"

#include "libs/memory.h"
#include "memory/allocator/memory_allocator.h"

#define MODULE "malloc"

void heap_init()
{
	allocator_init();
}

void free(void *mem)
{
	kfree(mem);
}

void *malloc(size_t size)
{
	return kmalloc(size);
}

void *calloc(size_t num, size_t size)
{
	return kcalloc(num, size);
}
void *realloc(void *ptr, size_t size)
{
	return krealloc(ptr, size);
}

void *kmalloc_phys(size_t size, void **virt_out)
{
	void *virt = kmalloc(size);
	log_debug(MODULE, "malloc returned %p", virt);
	memset(virt, 0, size);
	*virt_out = virt;
	void *phys = paging_get_physical(kernel_page, virt);
	log_debug(MODULE, "p%p = kmalloc_phys(%u, v%p)", phys, size, virt);
	return phys;
}
void *kcalloc_phys(size_t num, size_t size, void **virt_out)
{
	void *virt = kcalloc(num, size);
	memset(virt, 0, size * num);
	*virt_out = virt;
	return paging_get_physical(kernel_page, virt);
}
