/*
 * File: elf.h
 * File Created: 02 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"

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
    ELF_ABU_SysV = 0x00,
    ELF_ABU_Linux = 0x03,
    ELF_ABU_Solaris = 0x06,
    ELF_ABU_FreeBSD = 0x09,
    ELF_ABU_OpenBSD = 0x0C,
    ELF_ABU_OpenVMS = 0x0D,
} elf_abi_t;

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

typedef enum elf_instruction_set
{
    EM_NONE = 0,
    EM_M32 = 1,
    EM_SPARC = 2,
    EM_386 = 3,
    EM_68K = 4,
    EM_88K = 5,
    EM_860 = 7,
    EM_MIPS = 8,
    EM_ARM = 0x28,
    EM_X64 = 0x3E,
    EM_ARM64 = 0xB7,
    EM_RISCV = 0xF3,
} elf_instruction_set_t;

typedef struct
{
    uint8_t magic[4];
    uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
    uint8_t endianness; // 1 = little endian, 2 = big endian
    uint8_t elf_header_version;
    uint8_t ABI;
    uint8_t _padding[8];
    uint16_t type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t instruction_set;
    uint32_t elf_version;
    uint32_t program_entry_position;
    uint32_t program_header_table_position;
    uint32_t section_header_table_position;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;
    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;
    uint16_t section_names_index;
} __attribute__((packed)) elf32_header_t;

typedef struct
{
    uint8_t magic[4];
    uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
    uint8_t endianness; // 1 = little endian, 2 = big endian
    uint8_t elf_header_version;
    uint8_t ABI;
    uint8_t _Padding[8];
    uint16_t type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t instruction_set;
    uint32_t elf_version;
    uint64_t program_entry_position;
    uint64_t program_header_table_position;
    uint64_t section_header_table_position;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;
    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;
    uint16_t section_names_index;
} __attribute__((packed)) elf64_header_t;

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

    SHF_MASKPROC = 0xf0000000,
} elf_section_flags_t;

typedef struct
{
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t virtual_address;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t address_align;
    uint32_t entry_size;
} elf32_section_header_t;

typedef struct
{
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint64_t virtual_address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t address_align;
    uint64_t entry_size;
} elf64_section_header_t;

typedef struct
{
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint64_t virtual_address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t address_align;
    uint64_t entry_size;
} elf3264_section_header_t;

typedef struct
{
} elf32_symbol_t;

typedef struct
{
} elf64_symbol_t;

typedef enum elf_program_type
{
    // Program header table entry unused.
    PT_NULL = 0,

    // Loadable segment.
    PT_LOAD = 1,

    // Dynamic linking information.
    PT_DYNAMIC = 2,

    // Interpreter information.
    PT_INTERP = 3,

    // Auxiliary information.
    PT_NOTE = 4,

    // Reserved
    PT_SHLIB = 5,

    // Segment containing program header table itself.
    PT_PHDR = 6,

    // Thread-Local Storage template.
    PT_TLS = 7,

    // Reserved inclusive range. Processor specific.
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7FFFFFFF,
} elf_program_type_t;

typedef enum elf_program_flags
{
    PF_EXEC = 0x01,

    PF_WRITE = 0x02,

    PF_READ = 0x04,
} elf_program_flags_t;

typedef struct
{
    uint32_t type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t memory_size;
    uint32_t flags;
    uint32_t align;
} elf32_program_header_t;

typedef struct
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t align;
} elf64_program_header_t;

typedef struct
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t align;
} elf3264_program_header_t;