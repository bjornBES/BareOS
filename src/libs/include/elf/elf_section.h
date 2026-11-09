/*
 * File: elf_section.h
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


typedef enum elf_section_type
{
    // Section header table entry unused.
    SHT_NULL = 0,

    // Program data.
    SHT_PROGBITS = 1,

    // Symbol table.
    SHT_SYMTAB = 2,

    // String table.
    SHT_STRTAB = 3,

    // Relocation entries with addends
    SHT_RELA = 4,

    // Symbol hash table.
    SHT_HASH = 5,

    // Dynamic linking information.
    SHT_DYNAMIC = 6,

    // Notes
    SHT_NOTE = 7,

    // Program space with no data (bss)
    SHT_NOBITS = 8,

    // Relocation entries, no addends
    SHT_REL = 9,

    // Reserved
    SHT_SHLIB = 0x0A,

    // Dynamic linker symbol table
    SHT_DYNSYM = 0x0B,
    /*
        // Array of constructors
        SHT_INIT_ARRAY = 0x0E,

        // Array of destructors
        SHT_FINI_ARRAY = 0x0F,

        // Array of pre-constructors
        SHT_PREINIT_ARRAY = 0x10,

        // Section group
        SHT_GROUP = 0x11,

        // Extended section indices
        SHT_SYMTAB_SHNDX = 0x12,

        // Number of defined types
        SHT_NUM = 0x13, */

    // Start OS-specific
    SHT_LOOS = 0x60000000,

    SHT_GNU_HASH = 0x6FFFFFF6,

    SHT_LOPROC = 0x70000000,

    SHT_HIPROC = 0x7FFFFFFF,

    SHT_LOUSER = 0x80000000,

    SHT_HIUSER = 0xFFFFFFFF,
} elf_section_type_t;

typedef enum elf_section_flags
{
    SHF_WRITE = 0x1,

    SHF_ALLOC = 0x2,

    SHF_EXECINSTR = 0x4,

    SHF_MASKPROC = 0xF0000000,
} elf_section_flags_t;

typedef struct
{
    elf32_word sh_name;
    elf32_word type;
    elf32_word flags;
    elf32_addr virtual_address;
    elf32_off offset;
    elf32_word size;
    elf32_word link;
    elf32_word info;
    elf32_word address_align;
    elf32_word entry_size;
} elf32_section_header_t;

typedef struct
{
    elf64_word sh_name;
    elf64_word type;
    elf64_xword flags;
    elf64_addr virtual_address;
    elf64_off offset;
    elf64_xword size;
    elf64_word link;
    elf64_word info;
    elf64_xword address_align;
    elf64_xword entry_size;
} elf64_section_header_t;

typedef struct
{
    elf64_word sh_name;
    elf64_word type;
    elf64_xword flags;
    elf64_addr virtual_address;
    elf64_off offset;
    elf64_xword size;
    elf64_word link;
    elf64_word info;
    elf64_xword address_align;
    elf64_xword entry_size;
} elf_section_header_t;

typedef struct
{
    char name[CONFIG_LOADER_MAX_SECTION_NAME_LENGTH];
    elf_section_header_t section;
} elf_full_section_header_t;
