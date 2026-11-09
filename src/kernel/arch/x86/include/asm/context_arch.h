/*
 * File: context_arch.h
 * File Created: 22 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 22 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "contextt_arch.h"

status_t context_arch_set_stack(context_t *ctx, vaddr_t new_stack);

// switch from one thread context to another
void context_arch_switch(vaddr_t to);

void context_dump(context_t *ctx);