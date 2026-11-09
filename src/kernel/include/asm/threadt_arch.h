/*
 * File: threadt_arch.h
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct thread_arch
{
    vaddr_t fs_base;
    vaddr_t gs_base;
} thread_arch_t;
