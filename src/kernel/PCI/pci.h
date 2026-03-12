/*
 * File: pci.h
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include <stdint.h>
#include <boot/bootparams.h>

#pragma once

#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06

typedef struct {
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t card_bus_cis;
	uint16_t subsystem_id;
	uint16_t subsystem_vendor_id;
	uint32_t rom_base_address;
	uint8_t reserved[3];
	uint8_t capabilities_pointer;
	uint8_t reserved2[4];
	uint8_t max_latency;
	uint8_t min_grant;
	uint8_t interrupt_pin;
	uint8_t interrupt_line;
} __attribute__((packed)) pci_header0;

typedef struct {
	uint32_t bar1;
	uint32_t bar2;

	uint8_t secondary_latency_timer;
	uint8_t subordinate_bus_number;
	uint8_t secondary_bus_number;
	uint8_t primary_bus_number;
	uint16_t secondary_status;
	uint8_t io_limit;
	uint8_t io_base;
	uint16_t memory_limit;
	uint16_t memory_base;
	uint16_t prefetchable_memory_limit;
	uint16_t prefetchable_memory_base;
	uint32_t prefetchable_base_upper32;
	uint32_t prefetchable_limit_upper32;
	uint16_t io_limit_upper16;
	uint16_t io_base_upper16;
	uint8_t reserved[3];
	uint8_t capabilities_ptr;
	uint32_t rom_base_address;
	uint16_t bridge_control_register;
	uint8_t interrupt_pin;
	uint8_t interrupt_line;
} __attribute__((packed)) pci_header1;

typedef union
{
    pci_header0 header0;
    pci_header1 header1;
    uint8_t bytes[48];
} pci_header;

typedef struct
{
    uint8_t bus;
    uint8_t slot;
    uint8_t function;

    uint16_t device_id;
    uint16_t vendor_id;
    
    uint16_t status;
    uint16_t command;
    
    uint8_t class_code;
    uint8_t sub_class;
    uint8_t prog_if;
    uint8_t revision_id;
    
    uint8_t bist;
    uint8_t header_type;
    uint8_t latency_timer;
    uint8_t cache_line_size;
    
    pci_header header;
} __attribute__((packed)) pci_device_id;

typedef struct
{
    pci_device_id* id;
    char* name;
} pci_driver;

void pci_init_devices();
void pci_init(PCI_bios_info bios_info);