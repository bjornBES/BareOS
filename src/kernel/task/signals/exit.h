/*
 * File: exit.h
 * File Created: 15 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "task/process.h"
#include "task/signal.h"

#define EXIT_CODE_MEMORY

void do_exit(uint32_t code, process_t *proc);
