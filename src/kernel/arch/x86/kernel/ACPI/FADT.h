/*
 * File: FADT.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ACPI.h"

typedef struct
{
    SDT_header h;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    // field used in acpi 1.0; no longer in use, for compatibility only
    uint8_t reserved;

    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4_bios_req;
    uint8_t pstate_control;
    uint32_t pm_1a_event_block;
    uint32_t pm_1b_event_block;
    uint32_t pm_1a_control_block;
    uint32_t pm_1b_control_block;
    uint32_t pm_2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe_0_block;
    uint32_t gpe_1_block;
    uint8_t pm_1_event_length;
    uint8_t pm_1_control_length;
    uint8_t pm_2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe_0_length;
    uint8_t gpe_1_length;
    uint8_t gpe_1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    // reserved in acpi 1.0; used since acpi 2.0+
    uint16_t boot_architecture_flags;

    uint8_t reserved2;
    uint32_t flags;

    generic_address_structure reset_reg;

    uint8_t reset_value;
    uint8_t reserved3[3];

    // 64bit pointers - available on acpi 2.0+
    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    generic_address_structure x_pm_1a_event_block;
    generic_address_structure x_pm_1b_event_block;
    generic_address_structure x_pm_1a_control_block;
    generic_address_structure x_pm_1b_control_block;
    generic_address_structure x_pm2_control_block;
    generic_address_structure x_pm_timer_block;
    generic_address_structure x_gpe_0_block;
    generic_address_structure x_gpe_1_block;
} __attribute__((packed)) FADT;

void FADT_init();
void FADT_reset();
void FADT_shutdown();