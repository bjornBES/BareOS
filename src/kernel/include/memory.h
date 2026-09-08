/*
 * File: memory.h
 * File Created: 30 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <memory_lib.h>

void* kmalloc(size_t size);
int kfree(void* ptr);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);