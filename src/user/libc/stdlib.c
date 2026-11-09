/*
 * File: stdlib.c
 * File Created: 26 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "stdlib.h"

#include "allocator/memory_allocator.h"

void* malloc(size_t size)
{
    return impl_malloc(size);
}

void* calloc(size_t num, size_t size)
{
    return impl_calloc(num, size);
}

void* realloc(void* ptr, size_t size)
{
    return impl_realloc(ptr, size);
}

void free(void* ptr)
{
    impl_free(ptr);
}

