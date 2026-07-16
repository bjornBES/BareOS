/*
 * File: bootparams.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>

#include "bp_config.h"

#define MEMORY_AVAILABLE 1
#define MEMORY_RESERVED 2
#define MEMORY_ACPI_RECLAIMABLE 3
#define MEMORY_NVS 4
#define MEMORY_BADRAM 5

// ==================== VESA BIOS info ====================
// Source: INT 0x10, AX=0x4F00 — Get VBE Controller Info
// Output: ES:DI points to VbeInfoBlock
typedef struct
{
    uint16_t vbe_version;  // VbeVersion (offset 0x04)
    uint32_t capabilities; // Capabilities (offset 0x06)
    uint16_t total_memory; // TotalMemory in 64 KB blocks (offset 0x12)
    // Mode list copied from far pointer at offset 0x0E (VideoModePtr)
    uint16_t modeList[MAX_VIDEO_MODES];
} __attribute__((packed)) VESA_bios_info;

// ==================== boot_params ====================
// Aggregate structure passed from bootloader to kernel
typedef union
{
    // INT 0x10, VESA
    VESA_bios_info vesa_bios;

} arch_boot_params_t;
