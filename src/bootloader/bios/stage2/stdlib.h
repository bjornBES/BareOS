/*
 * File: stdlib.h
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *));
