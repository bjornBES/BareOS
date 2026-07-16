/*
 * File: ctx.h
 * File Created: 01 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include "kernel/ctx.h"
#include <types.h>

typedef struct intr_frame intr_frame_t;

typedef struct cpu_ctx
{
    union
    {
        vaddr_t sp;
        intr_frame_t *regs;
    } frame;
} cpu_ctx_t;
