/*
 * File: elf.h
 * File Created: 15 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 15:59:10
 * Modified By: BjornBEs
 * -----
 */

#define ELF_MAGIC ("\x7F" \
                   "ELF")

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t magic[4];
    uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
    uint8_t endianness; // 1 = little endian, 2 = big endian
    uint8_t ELF_header_version;
    uint8_t ABI;
    uint8_t _Padding[8];
    uint16_t type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t instruction_set;
    uint32_t ELF_version;
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
} __attribute__((packed)) ELF_header;

#ifdef __x86_64__
typedef struct
{
    uint8_t magic[4];
    uint8_t bitness;    // 1 = 32 bit, 2 = 64 bit
    uint8_t endianness; // 1 = little endian, 2 = big endian
    uint8_t ELF_header_version;
    uint8_t ABI;
    uint8_t _Padding[8];
    uint16_t type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t instruction_set;
    uint32_t ELF_version;
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
} __attribute__((packed)) ELF_header64;
#endif

enum ELF_bitness
{
    ELF_BITNESS_32BIT = 1,
    ELF_BITNESS_64BIT = 2,
};

enum ELF_endianness
{
    ELF_ENDIANNESS_LITTLE = 1,
    ELF_ENDIANNESS_BIG = 2,
};

enum ELF_ABI
{
    ELF_ABU_SysV = 0x00,
    ELF_ABU_Linux = 0x03,
    ELF_ABU_Solaris = 0x06,
    ELF_ABU_FreeBSD = 0x09,
    ELF_ABU_OpenBSD = 0x0C,
    ELF_ABU_OpenVMS = 0x0D,
};

enum ELF_instruction_set
{
    ELF_INSTRUCTION_SET_NONE = 0,
    ELF_INSTRUCTION_SET_X86 = 3,
    ELF_INSTRUCTION_SET_ARM = 0x28,
    ELF_INSTRUCTION_SET_X64 = 0x3E,
    ELF_INSTRUCTION_SET_ARM64 = 0xB7,
    ELF_INSTRUCTION_SET_RISCV = 0xF3,
};

enum ELF_type
{
    ELF_TYPE_RELOCATABLE = 1,
    ELF_TYPE_EXECUTABLE = 2,
    ELF_TYPE_SHARED = 3,
    ELF_TYPE_CORE = 4,
};

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
} ELF_program_header;

#ifdef __x86_64__
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
} ELF_program_header64;
#endif

enum ELF_program_type
{
    // Program header table entry unused.
    ELF_PROGRAM_TYPE_NULL = 0,

    // Loadable segment.
    ELF_PROGRAM_TYPE_LOAD = 1,

    // Dynamic linking information.
    ELF_PROGRAM_TYPE_DYNAMIC = 2,

    // Interpreter information.
    ELF_PROGRAM_TYPE_INTERP = 3,

    // Auxiliary information.
    ELF_PROGRAM_TYPE_NOTE = 4,

    // Reserved
    ELF_PROGRAM_TYPE_SHLIB = 5,

    // Segment containing program header table itself.
    ELF_PROGRAM_TYPE_PHDR = 6,

    // Thread-Local Storage template.
    ELF_PROGRAM_TYPE_TLS = 7,

    // Reserved inclusive range. Operating system specific.
    ELF_PROGRAM_TYPE_LOOS = 0x60000000,
    ELF_PROGRAM_TYPE_HIOS = 0x6FFFFFFF,

    // Reserved inclusive range. Processor specific.
    ELF_PROGRAM_TYPE_LOPROC = 0x70000000,
    ELF_PROGRAM_TYPE_HIPROC = 0x7FFFFFFF,
};

enum ELF_program_flags
{
    ELF_PROGRAM_FLAGS_EXEC = 0x01,

    ELF_PROGRAM_FLAGS_WRITE = 0x02,

    ELF_PROGRAM_FLAGS_READ = 0x04,

    ELF_PROGRAM_FLAGS_LOAD = 0x08,
};

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
} ELF_section_header;

#ifdef __x86_64__
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
} ELF_section_header64;
#endif

enum ELF_section_type
{
    // Section header table entry unused.
    ELF_SECTION_TYPE_NULL = 0,

    // Program data.
    ELF_SECTION_TYPE_PROGBITS = 1,

    // Symbol table.
    ELF_SECTION_TYPE_SYNTAB = 2,

    // String table.
    ELF_SECTION_TYPE_STRTAB = 3,

    // Relocation entries with addends
    ELF_SECTION_TYPE_RELA = 4,

    // Symbol hash table.
    ELF_SECTION_TYPE_HASH = 5,

    // Dynamic linking information.
    ELF_SECTION_TYPE_DYNAMIC = 6,

    // Notes
    ELF_SECTION_TYPE_NOTE = 7,

    // Program space with no data (bss)
    ELF_SECTION_TYPE_NOBITS = 8,

    // Relocation entries, no addends
    ELF_SECTION_TYPE_REL = 9,

    // Reserved
    ELF_SECTION_TYPE_SHLIB = 0x0A,

    // Dynamic linker symbol table
    ELF_SECTION_TYPE_DYNSYM = 0x0B,

    // Array of constructors
    ELF_SECTION_TYPE_INIT_ARRAY = 0x0E,

    // Array of destructors
    ELF_SECTION_TYPE_FINI_ARRAY = 0x0F,

    // Array of pre-constructors
    ELF_SECTION_TYPE_PREINIT_ARRAY = 0x10,

    // Section group
    ELF_SECTION_TYPE_GROUP = 0x11,

    // Extended section indices
    ELF_SECTION_TYPE_SYMTAB_SHNDX = 0x12,

    // Number of defined types
    ELF_SECTION_TYPE_NUM = 0x13,

    // Start OS-specific
    ELF_SECTION_TYPE_LOOS = 0x60000000,
};

enum ELF_section_flags
{
    // Writable
    ELF_SECTION_FLAGS_WRITE = 0x1,

    // Occupies memory during execution
    ELF_SECTION_FLAGS_ALLOC = 0x2,

    // Executable
    ELF_SECTION_FLAGS_EXECINSTR = 0x4,

    // Might be merged
    ELF_SECTION_FLAGS_MERGE = 0x10,

    // Contains null-terminated strings
    ELF_SECTION_FLAGS_STRINGS = 0x20,

    // 'sh_info' contains SHT index
    ELF_SECTION_FLAGS_INFO_LINK = 0x40,

    // Preserve order after combining
    ELF_SECTION_FLAGS_LINK_ORDER = 0x80,

    // Non-standard OS specific handling required
    ELF_SECTION_FLAGS_OS_NONCONFORMING = 0x100,

    // Section is member of a group
    ELF_SECTION_FLAGS_GROUP = 0x200,

    // Section hold thread-local data
    ELF_SECTION_FLAGS_TLS = 0x400,

    // OS-specific
    ELF_SECTION_FLAGS_MASKOS = 0x0FF00000,

    // Processor-specific
    ELF_SECTION_FLAGS_MASKPROC = 0xF0000000,
};

void ELF_init();
