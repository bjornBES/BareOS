/*
 * File: bootparams.h
 * File Created: 30 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 19 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <types.h>

#include "asm/boot/bootparams.h"
#include "asm/boot/bp_config.h"
#include "asm/boot/bp_memory.h"

#include "boot_stage/bp_stage.h"
#include "video/bp_video.h"
#include "acpi/bp_acpi.h"
#include "time/bp_time.h"
#include "bootloader/bp_bootloader.h"

typedef struct boot_params
{
    uint64_t kernel_address;
    uint8_t boot_device;     // DL at boot
    uint16_t current_mode;
    uint32_t page_directory; // bootload paging32 page directory phys address
    time_t boot_time;

    bp_stage_t stage;
    bp_memory_t memory;
    bp_bootloader_t bootloader;
    bp_video_t video;
    bp_acpi_t acpi;


    uint8_t cpu_core_trampoline[TRAMPOLINE_SIZE]; // TRAMPOLINE_SIZE = 255
} __attribute__((packed)) boot_params_t;
