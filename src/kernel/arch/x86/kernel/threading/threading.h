/*
 * File: threading.c
 * File Created: 02 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel/registers.h"

registers *thread_create_regs(void *stack_pointer, void *entry);
registers *thread_create_user_regs(void *sp, void *entry, uint64_t user_rsp);
registers *thread_create_regs_from(void *dest_sp, registers *src_regs);
void *thread_get_function(void *stack_pointer);