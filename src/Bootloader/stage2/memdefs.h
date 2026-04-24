/*
 * File: memdefs.h
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 12 Apr 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

extern char __fat_driver;

// 0x00000000 - 0x000003FF - interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

// 0x00000500 - 0x00000600 - Partitions

// 0x00000600 - 0x00006600 - Bootparams
#define MEMORY_BOOTPARAMS_ADDR ((void *)0x00000600)
#define MEMORY_BOOTPARAMS_SIZE MAX_BOOTPARAMS_SIZE

// 0x00006600 - 0x00008000 - stack

// 0x00007C00 - 0x00008000 - Bootsector (MBR and VBR)

// 0x00008000 - 0x00040000 - stage2

// 0x00040000 - 0x00050000 - FAT driver
#define MEMORY_FAT_ADDR ((void *)&__fat_driver)
#define MEMORY_FAT_SIZE 0x00010000

// 0x00050000 - 0x00080000 - free

// 0x00080000 - 0x0009FFFF - Extended BIOS data area
// 0x000A0000 - 0x000C7FFF - Video
// 0x000C8000 - 0x000FFFFF - BIOS

// current

#define MEMORY_KERNEL_ADDR ((void *)0x00100000)
#define MEMORY_LOAD_SIZE 0x00030000