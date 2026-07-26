/*
 * File: process.h
 * File Created: 26 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 26 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>
#include "kernel/asm/ivt/ivt.h"
#include "task/process_types.h"

void process_arch_build_return_frame(intr_frame_t *frame, process_t *proc);