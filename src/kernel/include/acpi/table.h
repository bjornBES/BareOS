/*
 * File: table.h
 * File Created: 10 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <defs.h>
#include <types.h>

typedef struct sdt_header
{
    union
    {
        char signature_char[4];
        uint32_t signature;
    };
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} PACKED sdt_header_t;

#define ADDRESS_SPACE_MEMORY 0
#define ADDRESS_SPACE_IO 1
#define ADDRESS_SPACE_PCI 2
#define ADDRESS_SPACE_EMBEDDED_CONTROLLER 3
#define ADDRESS_SPACE_PCI_DEVICE_BAR 6
#define ADDRESS_SPACE_OEM 0x80

typedef struct
{
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} PACKED generic_address_structure;

sdt_header_t *table_get_table(uint32_t signature);

/// @brief 
/// @param header 
/// @return 
int table_verify_checksum(sdt_header_t *header);

void table_set_base(vaddr_t base);