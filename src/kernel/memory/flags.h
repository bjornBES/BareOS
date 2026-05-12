/*
 * File: flags.h
 * File Created: 07 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "kernel.h"
#include "kernel/paging/paging.h"

typedef struct memory_flags
{
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t exec : 1;
    uint8_t user : 1;
    uint8_t cached : 1;
    uint8_t global : 1;
    uint8_t grows_down : 1;
    uint8_t _pad : 1;
} memory_flags_t;

// ELF .text segment
static const memory_flags_t text_flags = {.read = 1, .exec = 1, .user = 1};
static const memory_flags_t kernel_text_flags = {.read = 1, .exec = 1, .user = 0};
static const memory_flags_t vdso_text_flags = {.read = 1, .exec = 1, .user = 1};

// user stack
static const memory_flags_t stack_flags = {.read = 1, .write = 1, .user = 1, .grows_down = 1, .cached = 0, .exec = 0};

// user .data / heap
static const memory_flags_t data_flags = {.read = 1, .write = 1, .user = 1, .cached = 0, .exec = 0};

// kernel .data / heap
static const memory_flags_t kernel_data_flags = {.read = 1, .write = 1, .user = 0, .cached = 0, .exec = 0};

// MMIO
static const memory_flags_t mmio_flags = {.read = 1, .write = 1, .cached = 0};

// kernel mapping
static const memory_flags_t kern_flags = {.read = 1, .write = 1, .global = 1};

// smp trampoline mapping
static const memory_flags_t trampoline_flags = {.read = 1, .write = 1, .exec = 1, .global = 1};
