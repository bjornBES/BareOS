/*
 * File: table.h
 * File Created: 30 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
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
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) sdt_header;

typedef struct
{
    sdt_header header;
    uint32_t entries[];
} __attribute__((packed)) rsdt;
typedef struct
{
    sdt_header header;
    uint64_t entries[];
} __attribute__((packed)) XSDT;

sdt_header *table_get_table(uint32_t signature);
bool table_verify_checksum(sdt_header *header);
void table_set_rsdt(paddr_t base);
