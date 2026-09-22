/*
 * File: thread_arch.h
 * File Created: 21 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "thread/threadt.h"

#include <types.h>

status_t thread_arch_ctx_init(thread_t *thread, thread_func_t entry, vaddr_t kstack, uintptr_t arg);