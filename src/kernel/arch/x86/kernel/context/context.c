/*
 * File: context.c
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#include "asm/context_arch.h"
#include "asm/frame_arch.h"

#include "kerrno.h"

status_t context_arch_set_stack(context_t *ctx, vaddr_t new_stack)
{
    ctx->regs->sp = new_stack;
    return KERRNO_SUCCESSES;
}

void context_dump(context_t *ctx)
{
    frame_arch_dump_frame(ctx->regs);
}
