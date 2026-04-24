/*
 * File: x86.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

#define ASMCALL __attribute__((cdecl))

extern uint8_t menu_key;

typedef struct {
    uint8_t size;
    uint8_t res;
    uint16_t count;
    seg_off_t memory;
    uint64_t lba;
} extensions_dap;

void ASMCALL x86_outb(uint16_t port, uint8_t value);
uint8_t ASMCALL x86_inb(uint16_t port);

bool ASMCALL x86_Disk_GetDriveParams(uint8_t drive,
                                     uint8_t* driveTypeOut,
                                     uint16_t* cylindersOut,
                                     uint16_t* sectorsOut,
                                     uint16_t* headsOut);

bool ASMCALL x86_Disk_Reset(uint8_t drive);

bool ASMCALL x86_Disk_Read(uint8_t drive,
                           uint16_t cylinder,
                           uint16_t sector,
                           uint16_t head,
                           uint8_t count,
                           void* lowerDataOut);
int ASMCALL x86_ExtensionSupport(uint8_t drive);
bool ASMCALL x86_Disk_Read_Extended(uint8_t drive, extensions_dap* dap);
bool ASMCALL x86_VESASupported(void* result);
bool ASMCALL x86_GetVESAEntry(uint16_t mode, void* result);
bool ASMCALL x86_PCIInitCheck(uint8_t* PCIchar, uint8_t* protectedModeEntry, uint16_t* PCIInterfaceLevel, uint8_t* lastPCIBus);
void ASMCALL SetVGAMode(uint8_t mode);
int ASMCALL x86_SetVESAMode(uint16_t mode);

void ASMCALL JumpToKernel(void* bootparms);
bool ASMCALL X86_checkForKeys();
void ASMCALL x86_EnterLongMode();

typedef struct 
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t ACPI;

} E820MemoryBlock;

enum E820MemoryBlockType 
{
    E820_USABLE = 1,
    E820_RESERVED = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS = 4,
    E820_BAD_MEMORY = 5,
};

int __attribute__((cdecl)) x86_E820GetNextBlock(E820MemoryBlock* block, uint32_t* continuationId);
