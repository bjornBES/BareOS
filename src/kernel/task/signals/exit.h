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

int do_exit(uint32_t code, process *proc);
