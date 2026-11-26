/*
 * File: panic.h
 * File Created: 31 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define KERNEL_PANIC(sub_sys, fmt, ...) kernel_panic(__FILE__, __LINE__, __FUNCTION__, sub_sys ": " fmt, ##__VA_ARGS__)

bool panic_on_this_cpu();
bool panic_in_progress();
bool panic_on_other_cpu();

void kernel_panic(char *file, int line, const char *function, char *fmt, ...);