/*
 * File: pid.h
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "pidt.h"

#include <types.h>

extern pid_block_t init_pid_block;

status_t pid_allocate(process_t *proc);
