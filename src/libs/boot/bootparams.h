/*
 * File: bootparams.h
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once
#include <stdint.h>

#define MEMORY_AVAILABLE 1
#define MEMORY_RESERVED 2
#define MEMORY_ACPI_RECLAIMABLE 3
#define MEMORY_NVS 4
#define MEMORY_BADRAM 5

#define MAX_E820_ENTRIES 32
#define MAX_VESA_MODES 64
#define MAX_CMDLINE 128
#define MAX_BOOTLOADER_NAME 32
#define MAX_CPU_BRAND_STRING 48
#define MAX_RESERVED 4096

// ==================== Equipment list flags ====================
// Source: INT 0x11 — Get Equipment List
// Return: AX = equipment flags
// Bit meanings come from AX bitfield
typedef struct
{
    uint8_t hasFpu : 1;               // AX bit 1 (math coprocessor present)
    uint8_t hasCoprocessor : 1;       // AX bit 2 (obsolete, often same as FPU)
    uint8_t floppyFlag : 1;           // AX bit 0 = floppy installed
    uint8_t numFloppies : 2;          // AX bit 6: number of floppies minus 1 if floppyFlag set
    uint8_t reserved : 3;             // fill remaining bits
} __attribute__((packed)) EquipmentFlags;

// ==================== E820 memory map ====================
// Source: INT 0x15, AX=0xE820
// Inputs:
//   EAX = 0xE820
//   EDX = 'SMAP'
//   EBX = continuation
// Output (written to buffer pointed by ES:DI):
//   uint64_t BaseAddr
//   uint64_t Length
//   uint32_t Type
typedef struct
{
    uint64_t addr; // BaseAddr (ES:DI + 0)
    uint64_t size; // Length   (ES:DI + 8)
    uint32_t type; // Type     (ES:DI + 16)
} __attribute__((packed)) E820Entry;

// ==================== VESA BIOS info ====================
// Source: INT 0x10, AX=0x4F00 — Get VBE Controller Info
// Output: ES:DI points to VbeInfoBlock
typedef struct
{
    uint16_t vbeVersion;   // VbeVersion (offset 0x04)
    uint32_t capabilities; // Capabilities (offset 0x06)
    uint16_t totalMemory;  // TotalMemory in 64 KB blocks (offset 0x12)
    // Mode list copied from far pointer at offset 0x0E (VideoModePtr)
    uint16_t modeList[MAX_VESA_MODES];
} __attribute__((packed)) VESABiosInfo;

// ==================== VESA mode info ====================
// Source: INT 0x10, AX=0x4F01 — Get VBE Mode Info
// Input: CX = mode number
// Output: ES:DI -> ModeInfoBlock
typedef struct
{
    // ---- Mode validity & layout ----
    uint16_t modeAttributes; // ModeAttributes (offset 0x00)
    uint8_t memoryModel;     // MemoryModel (offset 0x1B)
    uint8_t reserved0;

    // ---- Resolution & format ----
    uint16_t width;       // XResolution (offset 0x12)
    uint16_t height;      // YResolution (offset 0x14)
    uint8_t bitsPerPixel; // BitsPerPixel (offset 0x19)

    // ---- Linear framebuffer ----
    uint32_t framebuffer; // PhysBasePtr (offset 0x28)
    uint32_t pitch;       // BytesPerScanLine (offset 0x10)

    // ---- Color layout ----
    uint8_t redMaskSize;      // RedMaskSize (offset 0x1F)
    uint8_t redFieldPosition; // RedFieldPosition (offset 0x20)

    uint8_t greenMaskSize;      // GreenMaskSize (offset 0x21)
    uint8_t greenFieldPosition; // GreenFieldPosition (offset 0x22)

    uint8_t blueMaskSize;      // BlueMaskSize (offset 0x23)
    uint8_t blueFieldPosition; // BlueFieldPosition (offset 0x24)

    uint8_t alphaMaskSize;      // RsvdMaskSize (offset 0x25)
    uint8_t alphaFieldPosition; // RsvdFieldPosition (offset 0x26)

    uint16_t mode;
} __attribute__((packed)) VESAMode;

// ==================== PCI BIOS info ====================
// Source: INT 0x1A, AX=0xB101 — PCI BIOS Present
// Output:
//   AH = major version
//   AL = minor version
//   CL = last bus number
//   EDX = "PCI "
typedef struct
{
    uint8_t majorVersion; // AH
    uint8_t minorVersion; // AL
    uint8_t lastBus;      // CL
    uint8_t numDevices;   // counted manually
    uint32_t signature;   // EDX = 'PCI '
} __attribute__((packed)) PCIBiosInfo;

// ==================== Boot drive params ====================
// Source: INT 0x13, AH=0x08 — Get Drive Parameters
// Input: DL = drive number
// Output:
//   CH, CL = cylinders
//   DH = heads
//   CL bits 0–5 = sectors per track
typedef struct
{
    uint8_t driveNumber; // DL
    uint16_t cylinders;  // from CH + CL
    uint8_t heads;       // DH
    uint8_t sectors;     // CL & 0x3F
    uint16_t sectorSize; // from BPB or assumed 512
} __attribute__((packed)) BootDriveParams;

// ==================== CPU info ====================
// Source: CPUID instruction
// Vendor: CPUID EAX=0
// Features: CPUID EAX=1 (EDX/ECX)
// Brand string: CPUID EAX=0x80000002–0x80000004
typedef struct
{
    char vendor[13];   // EBX, EDX, ECX (EAX=0)
    uint32_t features; // EDX (EAX=1)
    uint8_t family;    // EAX bits 8–11
    uint8_t model;     // EAX bits 4–7
    uint8_t stepping;  // EAX bits 0–3
    char brand[MAX_CPU_BRAND_STRING];
} __attribute__((packed)) CPUInfo;

// ==================== Bootloader info ====================
// Source: bootloader internal (not BIOS)
typedef struct
{
    char cmdline[MAX_CMDLINE]; // parsed by bootloader
    char bootloaderName[MAX_BOOTLOADER_NAME];
    uint32_t bootFlags;
} __attribute__((packed)) BootLoaderInfo;

// ==================== RTC / time ====================
// Source: INT 0x1A, AH=0x02 — Read RTC Time
// Output:
//   CH = hours
//   CL = minutes
//   DH = seconds
// Source: INT 0x1A, AH=0x04 — Read RTC Date
// Output:
//   CH = century
//   CL = year
//   DH = month
//   DL = day
typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __attribute__((packed)) RTCInfo;

// ==================== ACPI info ====================
// Source:
//   RSDP: BIOS scan 0xE0000–0xFFFFF for "RSD PTR "
//   LAPIC ID: CPUID EAX=1, EBX[31:24]
typedef struct
{
    uint64_t rsdpAddress;
    uint32_t lapicId;
} __attribute__((packed)) ACPIInfo;

// ==================== Memory info ====================
// Source: INT 0x12 — Get Conventional Memory Size
// Return: AX = KB below 1 MB
// Upper memory derived from E820
typedef struct
{
    uint32_t memLower; // AX from INT 0x12
    uint32_t memUpper; // calculated from E820
} __attribute__((packed)) MemoryInfo;

// ==================== BootParams ====================
// Aggregate structure passed from bootloader to kernel
typedef struct
{
    uint8_t BootDevice; // DL at boot
    uint16_t currentMode;
    uint32_t *pageDirectory;

    EquipmentFlags equipment; // INT 0x11

    // INT 0x15, E820
    uint32_t e820Count;
    E820Entry e820Entries[MAX_E820_ENTRIES];

    // INT 0x10, VESA
    VESABiosInfo vesaBios;
    uint32_t vesaModeCount;
    VESAMode vesaModes[MAX_VESA_MODES];

    // INT 0x1A, PCI BIOS
    PCIBiosInfo pciBios;

    // INT 0x13
    BootDriveParams bootDrive;

    // CPUID
    CPUInfo cpu;

    // Bootloader internal
    BootLoaderInfo bootLoader;

    // INT 0x1A RTC
    RTCInfo rtc;

    // ACPI scan + CPUID
    ACPIInfo acpi;

    // INT 0x12 + E820
    MemoryInfo memory;

    // Future-proofing
    uint8_t reserved[MAX_RESERVED];

} __attribute__((packed)) BootParams;
