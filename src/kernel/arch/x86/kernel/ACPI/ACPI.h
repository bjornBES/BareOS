/*
 * File: ACPI.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <boot/bootparams.h>

#include "kernel.h"

#define ADDRESS_SPACE_MEMORY 0
#define ADDRESS_SPACE_IO 1
#define ADDRESS_SPACE_PCI 2

typedef struct
{
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__((packed)) generic_address_structure;

typedef struct
{
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;

    struct
    {
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } __attribute__((packed)) XSDP;
} __attribute__((packed)) RSDP;


typedef struct
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) SDT_header;

typedef struct
{
    SDT_header header;
    uint32_t entries[];
} __attribute__((packed)) RSDT;
typedef struct
{
    SDT_header header;
    uint64_t entries[];
} __attribute__((packed)) XSDT;

SDT_header *ACPI_get_table(const char *signature);

void ACPI_init(ACPI_info info);
