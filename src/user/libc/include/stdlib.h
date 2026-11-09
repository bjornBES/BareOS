/*
 * File: stdlib.h
 * File Created: 17 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

void* malloc(size_t size);

void* calloc(size_t num, size_t size);

void* realloc(void* ptr, size_t size);

void free(void* ptr);
