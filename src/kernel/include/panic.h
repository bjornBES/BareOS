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

#define KERNEL_PANIC(sub_sys, ...) kernel_panic(sub_sys, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void kernel_panic(const char *sub_sys, char *file, int line, const char *function, char *fmt, ...);