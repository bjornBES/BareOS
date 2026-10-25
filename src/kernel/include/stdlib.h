/*
 * File: stdlib.h
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *));