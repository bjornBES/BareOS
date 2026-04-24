/*
 * File: ahci.h
 * File Created: 05 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 05 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdint.h>
#include "PCI/pci.h"


typedef enum
{
	FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
	FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
	FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
	FIS_TYPE_DATA = 0x46,	   // Data FIS - bidirectional
	FIS_TYPE_BIST = 0x58,	   // BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} FIS_TYPE;

typedef volatile struct
{
	uint32_t clb;		// 0x00, command list base address, 1K-byte aligned
	uint32_t clbu;
	uint32_t fb;		// 0x08, FIS base address, 256-byte aligned
	uint32_t fbu;
	uint32_t is;		// 0x10, interrupt status
	uint32_t ie;		// 0x14, interrupt enable
	uint32_t cmd;		// 0x18, command and status
	uint32_t rsv0;		// 0x1C, Reserved
	uint32_t tfd;		// 0x20, task file data
	uint32_t sig;		// 0x24, signature
	uint32_t ssts;		// 0x28, SATA status (SCR0:SStatus)
	uint32_t sctl;		// 0x2C, SATA control (SCR2:SControl)
	uint32_t serr;		// 0x30, SATA error (SCR1:SError)
	uint32_t sact;		// 0x34, SATA active (SCR3:SActive)
	uint32_t ci;		// 0x38, command issue
	uint32_t sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	uint32_t fbs;		// 0x40, FIS-based switch control
	uint32_t rsv1[11];	// 0x44 ~ 0x6F, Reserved
	uint32_t vendor[4];	// 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct
{
	// 0x00 - 0x2B, Generic Host Control
	uint32_t cap;		// 0x00, Host capability
	uint32_t ghc;		// 0x04, Global host control
	uint32_t is;		// 0x08, Interrupt status
	uint32_t pi;		// 0x0C, Port implemented
	uint32_t vs;		// 0x10, Version
	uint32_t ccc_ctl;	// 0x14, Command completion coalescing control
	uint32_t ccc_pts;	// 0x18, Command completion coalescing ports
	uint32_t em_loc;	// 0x1C, Enclosure management location
	uint32_t em_ctl;	// 0x20, Enclosure management control
	uint32_t cap2;		// 0x24, Host capabilities extended
	uint32_t bohc;		// 0x28, BIOS/OS handoff control and status

	// 0x2C - 0x9F, Reserved
	uint8_t  rsv[0xA0-0x2C];

	// 0xA0 - 0xFF, Vendor specific registers
	uint8_t  vendor[0x100-0xA0];

	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[32];	// 1 ~ 32
} HBA_MEM;

typedef struct
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_REG_H2D

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 3;	// Reserved
	uint8_t c : 1;		// 1: Command, 0: Control

	uint8_t command;  // Command register
	uint8_t featurel; // Feature register, 7:0

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3;	  // LBA register, 31:24
	uint8_t lba4;	  // LBA register, 39:32
	uint8_t lba5;	  // LBA register, 47:40
	uint8_t featureh; // Feature register, 15:8

	// DWORD 3
	uint8_t countl;	 // Count register, 7:0
	uint8_t counth;	 // Count register, 15:8
	uint8_t icc;	 // Isochronous command completion
	uint8_t control; // Control register

	// DWORD 4
	uint32_t rsv1; // Reserved
} FIS_REG_H2D;

typedef struct
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_REG_D2H

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 2;	// Reserved
	uint8_t i : 1;		// Interrupt bit
	uint8_t rsv1 : 1;	// Reserved

	uint8_t status; // Status register
	uint8_t error;	// Error register

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3; // LBA register, 31:24
	uint8_t lba4; // LBA register, 39:32
	uint8_t lba5; // LBA register, 47:40
	uint8_t rsv2; // Reserved

	// DWORD 3
	uint8_t countl;	 // Count register, 7:0
	uint8_t counth;	 // Count register, 15:8
	uint8_t rsv3[2]; // Reserved

	// DWORD 4
	uint32_t rsv4; // Reserved
} FIS_D2H;

typedef struct
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_DATA

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 4;	// Reserved

	uint8_t rsv1[2]; // Reserved

	// DWORD 1 ~ N
	uint32_t data[1]; // Payload
} FIS_DATA;

typedef struct
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_PIO_SETUP

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 1;	// Reserved
	uint8_t d : 1;		// Data transfer direction, 1 - device to host
	uint8_t i : 1;		// Interrupt bit
	uint8_t rsv1 : 1;

	uint8_t status; // Status register
	uint8_t error;	// Error register

	// DWORD 1
	uint8_t lba0;	// LBA low register, 7:0
	uint8_t lba1;	// LBA mid register, 15:8
	uint8_t lba2;	// LBA high register, 23:16
	uint8_t device; // Device register

	// DWORD 2
	uint8_t lba3; // LBA register, 31:24
	uint8_t lba4; // LBA register, 39:32
	uint8_t lba5; // LBA register, 47:40
	uint8_t rsv2; // Reserved

	// DWORD 3
	uint8_t countl;	  // Count register, 7:0
	uint8_t counth;	  // Count register, 15:8
	uint8_t rsv3;	  // Reserved
	uint8_t e_status; // New value of status register

	// DWORD 4
	uint16_t tc;	 // Transfer count
	uint8_t rsv4[2]; // Reserved
} FIS_PIO_SETUP;

typedef struct
{
	// DWORD 0
	uint8_t fis_type; // FIS_TYPE_DMA_SETUP

	uint8_t pmport : 4; // Port multiplier
	uint8_t rsv0 : 1;	// Reserved
	uint8_t d : 1;		// Data transfer direction, 1 - device to host
	uint8_t i : 1;		// Interrupt bit
	uint8_t a : 1;		// Auto-activate. Specifies if DMA Activate FIS is needed

	uint16_t rsved; // Reserved

	// DWORD 1&2

	uint64_t DMAbufferID; // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
						  // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

	// DWORD 3
	uint32_t rsvd; // More reserved

	// DWORD 4
	uint32_t DMAbufOffset; // Byte offset into buffer. First 2 bits must be 0

	// DWORD 5
	uint32_t TransferCount; // Number of bytes to transfer. Bit 0 must be 0

	// DWORD 6
	uint32_t resvd; // Reserved

} FIS_DMA_SETUP;

typedef struct
{
	FIS_DMA_SETUP DMASetup;
	uint8_t pad0[4];

	FIS_PIO_SETUP PIOSetup;
	uint8_t pad1[12];

	FIS_D2H RegDeviceToHost;
	uint8_t pad2[4];

	uint16_t dev_bits;

	uint8_t ufis[64];

	uint8_t reserved[96];
} HBA_FIS;

typedef struct
{
	uint8_t cfl : 5;
	uint8_t a : 1;
	uint8_t w : 1;
	uint8_t p : 1;
	uint8_t r : 1;
	uint8_t b : 1;
	uint8_t c : 1;
	uint8_t reserved0 : 1;
	uint8_t pmp : 4;

	uint16_t prdtl;

	volatile uint32_t prdbc;

	uint32_t ctba;
	uint32_t ctbau;

	uint32_t reserved1[4];
} HBA_CMD_HEADER;

typedef struct
{
	uint32_t dba;  // Data base address
	uint32_t dbau;  // Data base address
	uint32_t rsv0; // Reserved

	// DW3
	uint32_t dbc : 22; // Byte count, 4M max
	uint32_t rsv1 : 9; // Reserved
	uint32_t i : 1;	   // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct
{
	// 0x00
	uint8_t cfis[64]; // Command FIS

	// 0x40
	uint8_t acmd[16]; // ATAPI command, 12 or 16 bytes

	// 0x50
	uint8_t rsv[48]; // Reserved

	// 0x80
	HBA_PRDT_ENTRY prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef struct
{
    uint16_t general_config;        // 0:  General configuration
    uint16_t num_cylinders;         // 1:  Number of cylinders (obsolete)
    uint16_t specific_config;       // 2:  Specific configuration
    uint16_t num_heads;             // 3:  Number of heads (obsolete)
    uint16_t rsv0[2];               // 4-5: Retired
    uint16_t num_sectors_per_track; // 6:  Sectors per track (obsolete)
    uint16_t vendor0[3];            // 7-9: Vendor specific
    char     serial[20];            // 10-19: Serial number (ASCII, space-padded)
    uint16_t rsv1[2];               // 20-21: Retired
    uint16_t rsv2;                  // 22: Obsolete
    char     firmware[8];           // 23-26: Firmware revision (ASCII, space-padded)
    char     model[40];             // 27-46: Model number (ASCII, space-padded)
    uint16_t max_sectors_per_drq;   // 47: Max sectors per DRQ (bits 7:0)
    uint16_t rsv3;                  // 48: Trusted computing / reserved
    uint16_t capabilities0;         // 49: Capabilities (DMA, LBA support)
    uint16_t capabilities1;         // 50: Capabilities extended
    uint16_t rsv4[2];               // 51-52: Obsolete
    uint16_t field_validity;        // 53: Field validity flags
    uint16_t rsv5[5];               // 54-58: Obsolete
    uint16_t multi_sector_setting;  // 59: Multiple sector setting
    uint32_t lba28_sectors;         // 60-61: Total LBA28 addressable sectors
    uint16_t rsv6;                  // 62: Obsolete
    uint16_t dma_multiword;         // 63: Multiword DMA modes
    uint16_t pio_modes;             // 64: PIO modes supported
    uint16_t min_dma_cycle_time;    // 65: Min multiword DMA cycle time (ns)
    uint16_t rec_dma_cycle_time;    // 66: Recommended multiword DMA cycle time
    uint16_t min_pio_cycle_time;    // 67: Min PIO cycle time without flow control
    uint16_t min_pio_iordy_time;    // 68: Min PIO cycle time with IORDY
    uint16_t rsv7[6];               // 69-74: Reserved
    uint16_t queue_depth;           // 75: Queue depth (bits 4:0)
    uint16_t sata_capabilities;     // 76: SATA capabilities
    uint16_t sata_capabilities2;    // 77: SATA additional capabilities
    uint16_t sata_features_supp;    // 78: SATA features supported
    uint16_t sata_features_en;      // 79: SATA features enabled
    uint16_t major_version;         // 80: ATA major version
    uint16_t minor_version;         // 81: ATA minor version
    uint16_t cmd_set0;              // 82: Command set supported 0
    uint16_t cmd_set1;              // 83: Command set supported 1
    uint16_t cmd_set2;              // 84: Command set supported 2
    uint16_t cmd_set_en0;           // 85: Command set enabled 0
    uint16_t cmd_set_en1;           // 86: Command set enabled 1
    uint16_t cmd_set_en2;           // 87: Command set enabled 2
    uint16_t dma_ultra;             // 88: Ultra DMA modes
    uint16_t rsv8[4];               // 89-92: Reserved / erase times
    uint16_t hw_reset_result;       // 93: Hardware reset result
    uint16_t acoustic;              // 94: Acoustic management
    uint16_t rsv9[5];               // 95-99: Reserved
    uint64_t lba48_sectors;         // 100-103: Total LBA48 addressable sectors
    uint16_t rsv10[2];              // 104-105: Reserved
    uint16_t physical_logical;      // 106: Physical/logical sector size
    uint16_t rsv11[9];              // 107-115: Reserved
    uint16_t logical_sector_size[2];// 116-117: Logical sector size (words)
    uint16_t rsv12[8];              // 118-125: Reserved
    uint16_t last_lun;              // 126: Reserved
    uint16_t rsv13;                 // 127: Reserved
    uint16_t security_status;       // 128: Security status
    uint16_t vendor1[31];           // 129-159: Vendor specific
    uint16_t cfa_power;             // 160: CFA power mode
    uint16_t rsv14[7];              // 161-167: Reserved
    uint16_t form_factor;           // 168: Device form factor
    uint16_t rsv15[3];              // 169-171: Reserved
    char     media_serial[60];      // 172-191: Current media serial (ASCII)
    uint16_t sct_cmd_transport;     // 206: SCT command transport
    uint16_t rsv16[5];              // 207-211: Reserved
    uint16_t alignment;             // 209: Alignment of logical in physical
    uint16_t rsv17[6];              // 210-215: Reserved
    uint16_t nv_cache_caps;         // 214: NV cache capabilities
    uint32_t nv_cache_size;         // 215-216: NV cache size (logical blocks)
    uint16_t nv_cache_opts;         // 217: NV cache options
    uint16_t rsv18[3];              // 218-220: Reserved
    uint16_t transport_major;       // 221: Transport major version
    uint16_t transport_minor;       // 222: Transport minor version
    uint16_t rsv19[31];             // 223-253: Reserved
    uint16_t integrity;             // 255: Integrity word (checksum)
} __attribute__((packed)) sata_identify_packet;

typedef struct
{
    uint16_t drive;
} sata_private_data;


void ahci_initialize(pci_device_id *pdev);
