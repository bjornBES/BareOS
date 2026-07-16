/*
 * File: bp_bootloader.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "asm/boot/bp_config.h"
#include "types.h"

typedef struct bp_bootloader
{
    uint32_t boot_flags;
    char cmd_line[MAX_CMDLINE];                 // MAX_CMDLINE = 128
    char bootloader_name[MAX_BOOTLOADER_NAME];  // MAX_BOOTLOADER_NAME = 32
} __attribute__((packed)) bp_bootloader_t;