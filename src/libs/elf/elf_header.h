/*
 * File: elf_header.h
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"
#include "elf_base_types.h"
#include "defs.h"

#define EI_NIDENT 16

typedef enum elf_bitness
{
    ELF_CLASS_NONE = 0,
    ELF_CLASS_32 = 1,
    ELF_CLASS_64 = 2,
} elf_bitness_t;

typedef enum elf_endianness
{
    ELF_DATA_NONE = 0,
    ELF_DATA_LITTLE = 1,
    ELF_DATA_BIG = 2,
} elf_endianness_t;

typedef enum elf_abi
{
    ELF_ABI_SysV = 0x00,
    ELF_ABI_Linux = 0x03,
    ELF_ABI_Solaris = 0x06,
    ELF_ABI_FreeBSD = 0x09,
    ELF_ABI_OpenBSD = 0x0C,
    ELF_ABI_OpenVMS = 0x0D,
} elf_abi_t;

#define ELF_MAGIC (0x7F000000ULL | 0x450000ULL | 0x4c00ULL | 0x46ULL)
#define ELF_MAGIC_MASK (0xFFFFFFFF00000000ULL)

#define ELF_CONFIG(bitness, endianness, version, abi) ((0x00000000464c457f) | (bitness << 32+8) | (endianness << 32+16) | (version << 32+24) | (abi << 32+32))

#define ELF64_LITTLE(abi) ELF_CONFIG(ELF_CLASS_64, ELF_DATA_LITTLE, 1, abi)
#define ELF32_LITTLE(abi) ELF_CONFIG(ELF_CLASS_32, ELF_DATA_LITTLE, 1, abi)

typedef enum elf_type
{
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    ET_DYN = 3,
    ET_CORE = 4,
    ET_LOPROC = 0xFF00,
    ET_HIPROC = 0xFFFF,
} elf_type_t;

typedef enum elf_machine
{
    EM_NONE = 0,
    EM_M32 = 1,
    EM_SPARC = 2,
    EM_386 = 3,
    EM_68K = 4,
    EM_88K = 5,
    EM_486 = 6,
    EM_860 = 7,
    EM_MIPS = 8,
    EM_ARM = 0x28,
    EM_X64_64 = 0x3E,
    EM_AARCH64 = 0xB7,
    EM_RISCV = 0xF3,
} elf_machine_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t magic[4];
            uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
            uint8_t endianness; // 1 = little endian, 2 = big endian
            uint8_t elf_header_version;
            uint8_t ABI;
            uint8_t _Padding[8];
        };

        uint8_t e_ident[EI_NIDENT];

        struct
        {
            uint64_t e_ident64;
            uint64_t padding;
        };
        struct
        {
            uint32_t magic32;
            uint32_t e_ident32;
            uint64_t padding_1;
        };
    };

    elf32_half e_type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    elf32_half e_machine;
    elf32_word e_version;
    elf32_addr e_entry;     // program_entry_position
    elf32_off e_phoff;      // program_header_table_position
    elf32_off e_shoff;      // section_header_table_position
    elf32_word e_flags;
    elf32_half e_ehsize;    // header_size
    elf32_half e_phentsize; // program_header_table_entry_size
    elf32_half e_phnum;     // program_header_table_entry_count
    elf32_half e_shentsize; // section_header_table_entry_size
    elf32_half e_shnum;     // section_header_table_entry_count
    elf32_half e_shstrndx;  // section_names_index
} PACKED elf32_header_t; // Elf32_Ehdr

typedef struct
{
    union
    {
        struct
        {
            uint8_t magic[4];
            uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
            uint8_t endianness; // 1 = little endian, 2 = big endian
            uint8_t elf_header_version;
            uint8_t ABI;
            uint8_t _Padding[8];
        };

        uint8_t e_ident[EI_NIDENT];

        struct
        {
            uint64_t e_ident64;
            uint64_t padding;
        };
    };

    elf64_half e_type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    elf64_half e_machine;
    elf64_word e_version;
    elf64_addr e_entry;     // program_entry_position
    elf64_off e_phoff;      // program_header_table_position
    elf64_off e_shoff;      // section_header_table_position
    elf64_word e_flags;
    elf64_half e_ehsize;    // header_size
    elf64_half e_phentsize; // program_header_table_entry_size
    elf64_half e_phnum;     // program_header_table_entry_count
    elf64_half e_shentsize; // section_header_table_entry_size
    elf64_half e_shnum;     // section_header_table_entry_count
    elf64_half e_shstrndx;  // section_names_index
} PACKED elf64_header_t; // Elf64_Ehdr

typedef struct
{
    union
    {
        struct
        {
            uint8_t magic[4];
            uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
            uint8_t endianness; // 1 = little endian, 2 = big endian
            uint8_t elf_header_version;
            uint8_t ABI;
            uint8_t _Padding[8];
        };

        uint8_t e_ident[EI_NIDENT];

        struct
        {
            uint64_t e_ident64;
            uint64_t padding;
        };
    };

    elf64_half e_type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    elf64_half e_machine;
    elf64_word e_version;
    elf64_addr e_entry;     // program_entry_position
    elf64_off e_phoff;      // program_header_table_position
    elf64_off e_shoff;      // section_header_table_position
    elf64_word e_flags;
    elf64_half e_ehsize;    // header_size
    elf64_half e_phentsize; // program_header_table_entry_size
    elf64_half e_phnum;     // program_header_table_entry_count
    elf64_half e_shentsize; // section_header_table_entry_size
    elf64_half e_shnum;     // section_header_table_entry_count
    elf64_half e_shstrndx;  // section_names_index
} PACKED elf_header_t;
