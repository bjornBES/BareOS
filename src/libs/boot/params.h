/*
 * File: params.h
 * File Created: 23 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "config.h"

#include "boot/arch_params.h"
#include "video/framebuffer.h"
#include "acpi/bp_acpi.h"
#include "bp_memory.h"
#include "bp_config.h"

#define BOOT_PARAMS_MAGIC   0x42415245 // "BARE"
#define BOOT_PARAMS_VERSION 1u

typedef struct boot_params
{
    uint32_t magic;
    uint32_t struct_version;

    unsigned long long kernel_entry_point;
    unsigned long long kernel_phys_base;
    unsigned long long kernel_virt_base;
    size_t kernel_size;

    uint32_t boot_flags;
    char cmd_line[MAX_CMDLINE];
    char bootloader_name[MAX_BOOTLOADER_NAME];

    struct
    {
        uint32_t count;
        memory_entry_t entries[MAX_MEMORY_ENTRIES];
    } memory;

    struct
    {
        uint32_t count;
        framebuffer_t entries[MAX_VIDEO_MODES];
    } video;

    #if CONFIG_ENABLE_SMP == 1
    struct
    {
        uint8_t core_bringup[512];
    } smp;
    #endif

    bp_acpi_t acpi;

    arch_params_t arch;
} boot_params_t;
