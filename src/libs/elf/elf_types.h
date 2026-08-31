/*
 * File: elf_types.h
 * File Created: 28 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "types.h"
#include "config.h"

#include "elf_base_types.h"

#include "elf_header.h"
#include "elf_section.h"

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

    PT_GNU_EH_FRAME = 0x6474E550,
    PT_GNU_STACK = 0x6474E551,
    PT_GNU_RELRO = 0x6474E552,
    PT_GNU_PROPERTY = 0x6474E553,

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

typedef elf32_program_header_t elf32_phdr;

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

typedef elf64_program_header_t elf64_phdr;

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
} elf_program_header_t;

typedef elf_program_header_t elf_phdr;

typedef struct
{
    uint32_t name_size;
    uint32_t content_size;
    uint32_t content_type;
    char *name;
    // after name there is data
} elf_node_t;

typedef elf_node_t elf_nhdr;

typedef struct
{
    int32_t d_tag;

    union
    {
        uint32_t d_val;
        uint32_t d_ptr;
    } d_un;
} elf32_dyn;

typedef struct
{
    int64_t d_tag;

    union
    {
        uint64_t d_val;
        uint64_t d_ptr;
    } d_un;
} elf64_dyn;

enum
{
    DT_NULL = 0,          // ignored Marks the end of the dynamic array
    DT_NEEDED = 1,        // d_val The string table offset of the name of a needed library.
    DT_PLTRELSZ = 2,      // d_val Total size, in bytes, of the relocation entries associated with the procedure linkage table.
    DT_PLTGOT = 3,        // d_ptr Contains an address associated with the linkage table. The specific meaning of this field is processor-dependent.
    DT_HASH = 4,          // d_ptr Address of the symbol hash table, described below.
    DT_STRTAB = 5,        // d_ptr Address of the dynamic string table.
    DT_SYMTAB = 6,        // d_ptr Address of the dynamic symbol table.
    DT_RELA = 7,          // d_ptr Address of a relocation table with Elf64_Rela entries.
    DT_RELASZ = 8,        // d_val Total size, in bytes, of the DT_RELA relocation table.
    DT_RELAENT = 9,       // d_val Size, in bytes, of each DT_RELA relocation entry.
    DT_STRSZ = 10,        // d_val Total size, in bytes, of the string table.
    DT_SYMENT = 11,       // d_val Size, in bytes, of each symbol table entry.
    DT_INIT = 12,         // d_ptr Address of the initialization function.
    DT_FINI = 13,         // d_ptr Address of the termination function.
    DT_SONAME = 14,       // d_val The string table offset of the name of this shared object.
    DT_RPATH = 15,        // d_val The string table offset of a shared library search path string.
    DT_SYMBOLIC = 16,     // ignored The presence of this dynamic table entry modifies the symbol resolution algorithm for references within the library. Symbols defined within the library are used to resolve references before the dynamic linker searches the usual search path.
    DT_REL = 17,          // d_ptr Address of a relocation table with Elf64_Rel entries.
    DT_RELSZ = 18,        // d_val Total size, in bytes, of the DT_REL relocation table.
    DT_RELENT = 19,       // d_val Size, in bytes, of each DT_REL relocation entry.
    DT_PLTREL = 20,       // d_val Type of relocation entry used for the procedure linkage table. The d_val member contains either DT_REL or DT_RELA.
    DT_DEBUG = 21,        // d_ptr Reserved for debugger use.
    DT_TEXTREL = 22,      // ignored The presence of this dynamic table entry signals that the relocation table contains relocations for a non-writable segment.
    DT_JMPREL = 23,       // d_ptr Address of the relocations associated with the procedure linkage table.
    DT_BIND_NOW = 24,     // ignored The presence of this dynamic table entry signals that the dynamic loader should process all relocations for this object before transferring control to the program.
    DT_INIT_ARRAY = 25,   // d_ptr Pointer to an array of pointers to initialization functions.
    DT_FINI_ARRAY = 26,   // d_ptr Pointer to an array of pointers to termination functions.
    DT_INIT_ARRAYSZ = 27, // d_val Size, in bytes, of the array of initialization functions.
    DT_FINI_ARRAYSZ = 28, // d_val Size, in bytes, of the array of termination functions.
    DT_RUN_PATH = 29,

    // Defines a range of dynamic table tags that are reserved for environment-specific use.
    DT_LOOS = 0x60000000,
    DT_GNU_HASH = 0x6FFFFEF5,
    DT_HIOS = 0x6FFFFFFF,

    // Defines a range of dynamic table tags that are reserved for processor-specific use.
    DT_LOPROC = 0x70000000,
    DT_HIPROC = 0x7FFFFFFF,
};

typedef struct
{
    uint32_t st_name;  /* Symbol name */
    uint8_t st_info;   /* Type and Binding attributes */
    uint8_t st_other;  /* Reserved */
    uint16_t st_shndx; /* Section table index */
    uint64_t st_value; /* Symbol value */
    uint64_t st_size;  /* Size of object (e.g., common) */
} elf64_sym;

enum
{
    STB_LOCAL = 0,  // Not visible outside the object file
    STB_GLOBAL = 1, // Global symbol, visible to all object files
    STB_WEAK = 2,   // Global scope, but with lower precedence than global symbols

    // Environment-specific use
    STB_LOOS = 10,
    STB_HIOS = 12,

    // Processor-specific use
    STB_LOPROC = 13,
    STB_HIPROC = 15,
};

enum
{
    STT_NOTYPE = 0,  // No type specified (e.g., an absolute symbol)
    STT_OBJECT = 1,  // Data object
    STT_FUNC = 2,    // Function entry point
    STT_SECTION = 3, // Symbol is associated with a section

    // Environment-specific use
    STT_LOOS = 10,
    STT_HIOS = 12,

    // Processor-specific use
    STT_LOPROC = 13,
    STT_HIPROC = 15,
};

typedef struct
{
    size_t section_count;
    elf_section_header_t sections[CONFIG_LOADER_MAX_SECTIONS];

    char *build_id;
    char *interpreters[CONFIG_LOADER_MAX_INTERPRETERS];
    int interpreter_count;

    elf_section_header_t *interp_entry;
    elf_section_header_t *hash_entry;
    elf_section_header_t *rela_entry;

    int extern_symbol_count;

    int section_str_index;

    size_t node_count;
    elf_section_header_t *nodes[CONFIG_LOADER_MAX_NOTES];

    vaddr_t rela_plt;

    // loading
    size_t program_size;
    size_t program_size_align;
} elf_priv_t;
