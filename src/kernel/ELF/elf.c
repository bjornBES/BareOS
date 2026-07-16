/*
 * File: elf.c
 * File Created: 15 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 07 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"
#include <elf/elf_types.h>

#include "task/loader.h"
#include "device/device.h"
#include "task/process.h"

#include "debug/debug.h"
#include "mm/mmu/mmu.h"
#include "mm/vmm/vmm.h"

#include "kernel.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "stdio.h"

#define MODULE "ELF"

extern void hexdump(void *ptr, int len);

process_abi_t ELF_getABI(int ELF_abi, int endianness, int bitness)
{
    if (ELF_abi == ELF_ABU_SysV)
    {
        if (bitness == ELF_CLASS_64)
        {
            return ABI_SYSV64;
        }
        return ABI_SYSV32;
    }
    return Undefined;
}

size_t elf_read_file(fd_t fd, void *buffer, size_t buffer_size, size_t count, off_t offset)
{
    ENTER_FUNC(MODULE, "%u, %p, %u, %u, %u", fd, buffer, buffer_size, count, offset);
    fseek(fd, offset, SEEK_SET);
    return fread(buffer, buffer_size, count, fd);
}

int ELF_get_sections(fd_t fd, process_t *proc, void *header, uint8_t class)
{
    ENTER_FUNC(MODULE, "%u, %p, %p, %u", fd, proc, header, class);
    uint16_t section_count = 0;
    uint16_t section_name_index = 0;
    uint16_t section_entry_size = 0;
    uint64_t section_header_table_position = 0;
    if (class == ELF_CLASS_64)
    {
        elf64_header_t *elf_header = (elf64_header_t *)header;
        section_count = elf_header->section_header_table_entry_count;
        section_name_index = elf_header->section_names_index;
        section_entry_size = elf_header->section_header_table_entry_size;
        section_header_table_position = elf_header->section_header_table_position;
    }
    else
    {
        elf32_header_t *elf_header = (elf32_header_t *)header;
        section_count = elf_header->section_header_table_entry_count;
        section_name_index = elf_header->section_names_index;
        section_entry_size = elf_header->section_header_table_entry_size;
        section_header_table_position = elf_header->section_header_table_position;
    }

    uint8_t *buffer = malloc(section_entry_size * section_count);
    int bytes_read = elf_read_file(fd, buffer, section_entry_size, section_count, section_header_table_position);
    log_debug(MODULE, "read %u bytes into %p", bytes_read, buffer);

    log_debug(MODULE, "str table at offset 0x%x", section_name_index * section_entry_size);
    elf3264_section_header_t str_entry;
    if (class == ELF_CLASS_64)
    {
        elf64_section_header_t *entry64 = (elf64_section_header_t *)(buffer + (section_name_index * section_entry_size));
        log_debug(MODULE, "str_entry @ %p, str_entry64 @ %p", &str_entry, entry64);
        memcpy(&str_entry, entry64, section_entry_size);
    }
    else
    {
        elf32_section_header_t *entry32 = (elf32_section_header_t *)(buffer + (section_name_index * section_entry_size));
        str_entry.name = entry32->name;
        str_entry.type = entry32->type;
        str_entry.flags = entry32->flags;
        str_entry.virtual_address = (uint64_t)entry32->virtual_address;
        str_entry.offset = (uint64_t)entry32->offset;
        str_entry.size = (uint64_t)entry32->size;
        str_entry.link = entry32->link;
        str_entry.info = entry32->info;
        str_entry.address_align = (uint64_t)entry32->address_align;
        str_entry.entry_size = (uint64_t)entry32->entry_size;
    }

    char *str_table = malloc(str_entry.size);
    log_debug(MODULE, "str table @ %p with size %u or %u", str_table, str_entry.size, str_entry.entry_size);
    elf_read_file(fd, str_table, str_entry.size, 1, str_entry.offset);

    for (uint16_t i = 0; i < section_count; i++)
    {
        uint8_t *buffer = malloc(section_entry_size);
        elf_read_file(fd, buffer, section_entry_size, 1, section_header_table_position + (i * section_entry_size));
        elf3264_section_header_t entry;
        if (class == ELF_CLASS_64)
        {
            elf64_section_header_t *entry64 = (elf64_section_header_t *)buffer;
            memcpy(&entry, entry64, section_entry_size);
        }
        else
        {
            elf32_section_header_t *entry32 = (elf32_section_header_t *)buffer;
            entry.name = entry32->name;
            entry.type = entry32->type;
            entry.flags = entry32->flags;
            entry.virtual_address = (uint64_t)entry32->virtual_address;
            entry.offset = (uint64_t)entry32->offset;
            entry.size = (uint64_t)entry32->size;
            entry.link = entry32->link;
            entry.info = entry32->info;
            entry.address_align = (uint64_t)entry32->address_align;
            entry.entry_size = (uint64_t)entry32->entry_size;
        }

        if (entry.name == 0)
        {
            continue;
        }

        if ((entry.flags & SHF_ALLOC) == 0)
        {
            continue;
        }

        char *section_name = (char *)&str_table[entry.name];
        log_info(MODULE, "section %u name = %s type = 0x%x flags = 0x%x", i, section_name, entry.type, entry.flags);

        log_debug(MODULE, "proc->section_count = %u", proc->section_count);
        int current_index = proc->section_count;
        process_section_t *section = &proc->sections[current_index];
        log_debug(MODULE, "section @ %p", section);
        section->base = (vaddr_t)entry.virtual_address;
        section->size = entry.size;
        if (!strcmp(section_name, ".text"))
        {
            section->type = PROC_TYPE_TEXT;
            section->flags |= SECTION_FLAG_EXEC;
        }
        else if (!strcmp(section_name, ".data"))
        {
            section->type = PROC_TYPE_DATA;
            section->flags |= SECTION_FLAG_WRITE;
        }
        else if (!strcmp(section_name, ".rodata"))
        {
            section->type = PROC_TYPE_RODATA;
        }
        else if (!strcmp(section_name, ".bss"))
        {
            section->type = PROC_TYPE_BSS;
        }
        else
        {
            continue;
        }
        section->flags |= SECTION_FLAG_ALLOCATED;
        proc->section_count++;
    }
    free(str_table);
    free(buffer);
    return RETURN_GOOD;
}

int ELF_read_program(fd_t fd, process_t *proc, elf64_header_t *header, uint8_t class)
{
    vaddr_t dest;

    size_t proc_size = 0;
    size_t proc_size_align = 0;

    uint16_t section_count = 0;
    uint16_t section_entry_size = 0;
    uint64_t section_header_table_position = 0;
    if (class == ELF_CLASS_64)
    {
        elf64_header_t *elf_header = (elf64_header_t *)header;
        section_count = elf_header->program_header_table_entry_count;
        section_entry_size = elf_header->program_header_table_entry_size;
        section_header_table_position = elf_header->program_header_table_position;
        proc->entry = (vaddr_t)elf_header->program_entry_position;
    }
    else
    {
        elf32_header_t *elf_header = (elf32_header_t *)header;
        section_count = elf_header->program_header_table_entry_count;
        section_entry_size = elf_header->program_header_table_entry_size;
        section_header_table_position = elf_header->program_header_table_position;
        proc->entry = (vaddr_t)elf_header->program_entry_position;
    }

    void *buffer = malloc(section_count * section_entry_size);

    int bytes_read = elf_read_file(fd, buffer, section_entry_size, section_count, section_header_table_position);
    if (bytes_read < section_count * section_entry_size)
    {
        // something is wrong
        log_err(MODULE, "Didn't read program header from file");
        return RETURN_ERROR;
    }

    log_debug(MODULE, "program header table postion = %u", section_header_table_position);
    log_debug(MODULE, "program header table entry size = %u", section_entry_size);

    for (size_t i = 0; i < section_count; i++)
    {
        log_debug(MODULE, "program header index = %u", i);
        elf3264_program_header_t elf_prog_header;
        if (class == ELF_CLASS_64)
        {
            elf64_program_header_t *entry64 = (elf64_program_header_t *)(buffer + (i * sizeof(elf64_program_header_t)));
            memcpy(&elf_prog_header, entry64, sizeof(elf64_program_header_t));
        }
        else
        {
            elf32_program_header_t *entry32 = (elf32_program_header_t *)(buffer + (i * sizeof(elf32_program_header_t)));
            elf_prog_header.type = entry32->type;
            elf_prog_header.offset = (uint64_t)entry32->offset;
            elf_prog_header.virtual_address = (uint64_t)entry32->virtual_address;
            elf_prog_header.physical_address = (uint64_t)entry32->physical_address;
            elf_prog_header.file_size = (uint64_t)entry32->file_size;
            elf_prog_header.memory_size = (uint64_t)entry32->memory_size;
            elf_prog_header.flags = entry32->flags;
            elf_prog_header.align = (uint64_t)entry32->align;
        }

        fprintf(VFS_FD_DEBUG, "ELF: load seg %u\n", i);
        fprintf(VFS_FD_DEBUG, "\ttype 0x%x flags 0x%x\n", elf_prog_header.type, elf_prog_header.flags);
        fprintf(VFS_FD_DEBUG, "\toffset 0x%x\n", elf_prog_header.offset);
        fprintf(VFS_FD_DEBUG, "\tvaddr %p\n", elf_prog_header.virtual_address);
        fprintf(VFS_FD_DEBUG, "\tfile size %u mem size %u\n", elf_prog_header.file_size, elf_prog_header.memory_size);
        fprintf(VFS_FD_DEBUG, "\talign 0x%x\n", elf_prog_header.align);

        if (elf_prog_header.type != PT_LOAD)
        {
            continue;
        }

        if (elf_prog_header.memory_size == 0)
        {
            continue;
        }
        dest = (vaddr_t)elf_prog_header.virtual_address;
        proc_size += elf_prog_header.memory_size;
        size_t size_in_bytes = PAGE_ALIGN_UP(elf_prog_header.memory_size);
        if (proc_size >= proc_size_align)
        {
            proc_size_align += size_in_bytes;
            log_info(MODULE, "loading proc->vma = %p", proc->vma);
            vaddr_t offset = GET_PAGE_OFFSET(dest);

            if ((elf_prog_header.flags & PF_EXEC) == PF_EXEC)
            {
                do_mmap_eager(proc->vma, dest, size_in_bytes + offset, VMA_PROC_TEXT, text_flags);
            }
            else
            {
                do_mmap_eager(proc->vma, dest, size_in_bytes + offset, VMA_PROC_DATA, data_flags);
            }
        }

        // fprintf(VFS_FD_DEBUG, "PADDR 0x%p\n", phys->start);
        fprintf(VFS_FD_DEBUG, "dest %p\n", dest);
        fprintf(VFS_FD_DEBUG, "size 0x%x/0x%x\n", proc_size, proc_size_align);

        memset((void *)dest, 0, elf_prog_header.memory_size);
        // only bss dose not have this
        if (elf_prog_header.file_size != 0)
        {
            bytes_read = elf_read_file(fd, (void *)dest, elf_prog_header.file_size, 1, elf_prog_header.offset);
            log_info(MODULE, "expected %u did %d", elf_prog_header.file_size, bytes_read);
            if (bytes_read != elf_prog_header.file_size)
            {
                log_err(MODULE, "didn't read enough bytes expected %u did %d", elf_prog_header.file_size, bytes_read);
                return RETURN_ERROR;
            }
        }

        hexdump((void *)dest, 16);
    }

    fprintf(VFS_FD_DEBUG, "procress size = 0x%x/0x%x\n", proc_size, proc_size_align);
    fprintf(VFS_FD_DEBUG, "entry = 0x%x\n", proc->entry);
    return RETURN_GOOD;
}

int ELF64_read(fd_t fd, process_t *proc, loader *loader, elf64_header_t *header)
{
    ENTER_FUNC(MODULE, "%u, %p, %p, %p", fd, proc, loader, header);

    int state = ELF_get_sections(fd, proc, header, header->bitness);
    if (state != RETURN_GOOD)
    {
        return state;
    }
    log_info(MODULE, "got sections");

    log_info(MODULE, "loading program");
    state = ELF_read_program(fd, proc, header, header->bitness);
    if (state != RETURN_GOOD)
    {
        return state;
    }
    return RETURN_GOOD;
}

int ELF_read(fd_t fd, process_t *proc, loader *loader)
{
    uint8_t buffer[1024];
    elf32_header_t *header = (elf32_header_t *)buffer;

    fseek(fd, 0, SEEK_SET);
    elf_read_file(fd, buffer, 1024, 1, 0);

    if (header->bitness == ELF_CLASS_32)
    {
        // return ELF32_read(fd, proc, loader, header);
    }
    else if (header->bitness == ELF_CLASS_64)
    {
        return ELF64_read(fd, proc, loader, (elf64_header_t *)buffer);
    }
    log_err(MODULE, "Invalid elf class");
    return RETURN_ERROR;
}

/*
#ifdef __x86_64__
int ELF_read64(fd_t fd, process_t *proc, loader *loader, ELF_header64 elf_header)
{
    log_debug(MODULE, "ELF_read(%u, %p, %p)", fd, proc, loader);
    log_debug(MODULE, "proc (%p) = {%u}", proc, proc->entry);
    log_info(MODULE, "loading proc->vma = %p", proc->vma);
    uint8_t buffer[1024];

    // get the sections
    {
        uint16_t section_count = elf_header.section_header_table_entry_count;
        uint16_t section_name_index = elf_header.section_names_index;
        uint16_t section_entry_size = elf_header.section_header_table_entry_size;

        ELF_section_header64 section_name_entry;
        fseek(fd, elf_header.section_header_table_position + section_name_index * section_entry_size, SEEK_SET);
        fread(&section_name_entry, sizeof(ELF_section_header64), 1, fd);
        char *strtable = malloc(section_name_entry.size);
        {
            fseek(fd, section_name_entry.offset, SEEK_SET);
            fread(strtable, section_name_entry.size, 1, fd);
        }
        for (size_t i = 0; i < section_count; i++)
        {
            ELF_section_header64 elf_section_entry;
            fseek(fd, elf_header.section_header_table_position + i * section_entry_size, SEEK_SET);
            int bytes_read = fread(buffer, sizeof(ELF_section_header64), 1, fd);
            if (bytes_read < sizeof(ELF_section_header64))
            {
                // something is wrong
                log_err(MODULE, "Didn't read program section %u from file", i);
                return RETURN_ERROR;
            }
            memcpy(&elf_section_entry, buffer, sizeof(ELF_section_header64));

            if (elf_section_entry.name == 0)
            {
                continue;
            }

            if ((elf_section_entry.flags & ELF_SECTION_FLAGS_ALLOC) == 0)
            {
                continue;
            }

            char *section_name = (char *)&strtable[elf_section_entry.name];
            log_info(MODULE, "section %u name = %s type = 0x%x flags = 0x%x", i, section_name, elf_section_entry.type, elf_section_entry.flags);

            int current_index = proc->section_count;
            process_section_t *section = &proc->sections[current_index];
            section->base = (vaddr_t)elf_section_entry.virtual_address;
            section->size = elf_section_entry.size;
            if (!strcmp(section_name, ".text"))
            {
                section->type = PROC_TYPE_TEXT;
                section->flags |= SECTION_FLAG_EXEC;
            }
            else if (!strcmp(section_name, ".data"))
            {
                section->type = PROC_TYPE_DATA;
                section->flags |= SECTION_FLAG_WRITE;
            }
            else if (!strcmp(section_name, ".rodata"))
            {
                section->type = PROC_TYPE_RODATA;
            }
            else if (!strcmp(section_name, ".bss"))
            {
                section->type = PROC_TYPE_BSS;
            }
            else
            {
                continue;
            }
            section->flags |= SECTION_FLAG_ALLOCATED;
            proc->section_count++;
        }
        free(strtable);
    }
    log_info(MODULE, "got sections");

    log_info(MODULE, "loading proc->vma = %p", proc->vma);

    log_info(MODULE, "loading program");
    // time to load the program
    vaddr_t dest;
    log_debug(MODULE, "Header table count %u", elf_header.program_header_table_entry_count);

    vma_t *phys;
    size_t proc_size = 0;
    size_t proc_size_align = 0;
    for (size_t i = 0; i < elf_header.program_header_table_entry_count; i++)
    {
        ELF_program_header64 elf_prog_header;
        uint32_t prog_header_offset = elf_header.program_header_table_position + i * elf_header.program_header_table_entry_size;

        log_debug(MODULE, "program header table postion = %u", elf_header.program_header_table_position);
        log_debug(MODULE, "program header table entry size = %u", elf_header.program_header_table_entry_size);
        log_debug(MODULE, "program header offset = %u", prog_header_offset);
        fseek(fd, prog_header_offset, SEEK_SET);

        int bytes_read = fread(buffer, sizeof(ELF_program_header64), 1, fd);
        if (bytes_read < sizeof(ELF_program_header64))
        {
            // something is wrong
            log_err(MODULE, "Didn't read program header %u from file", i);
            return RETURN_ERROR;
        }
        memcpy(&elf_prog_header, buffer, sizeof(ELF_program_header64));

        fprintf(VFS_FD_DEBUG, "ELF: load seg %u\n", i);
        fprintf(VFS_FD_DEBUG, "\ttype 0x%x flags 0x%x\n", elf_prog_header.type, elf_prog_header.flags);
        fprintf(VFS_FD_DEBUG, "\toffset 0x%x\n", elf_prog_header.offset);
        fprintf(VFS_FD_DEBUG, "\tvaddr %p\n", elf_prog_header.virtual_address);
        fprintf(VFS_FD_DEBUG, "\tfile size %u mem size %u\n", elf_prog_header.file_size, elf_prog_header.memory_size);
        fprintf(VFS_FD_DEBUG, "\talign 0x%x\n", elf_prog_header.align);

        if (elf_prog_header.type != ELF_PROGRAM_TYPE_LOAD)
        {
            continue;
        }

        if (elf_prog_header.memory_size == 0)
        {
            continue;
        }
        dest = (vaddr_t)elf_prog_header.virtual_address;
        proc_size += elf_prog_header.memory_size;
        size_t size_in_bytes = ALIGN_2_UP(elf_prog_header.memory_size, PAGE_SIZE);
        if (proc_size >= proc_size_align)
        {
            proc_size_align += size_in_bytes;

            log_info(MODULE, "loading proc->vma = %p", proc->vma);
            if ((elf_prog_header.flags & ELF_PROGRAM_FLAGS_EXEC) == ELF_PROGRAM_FLAGS_EXEC)
            {
                phys = do_mmap(proc->vma, dest, elf_prog_header.memory_size, VMA_PROC_TEXT, text_flags);
            }
            else
            {
                phys = do_mmap(proc->vma, dest, elf_prog_header.memory_size, VMA_PROC_DATA, text_flags);
            }
        }

        fprintf(VFS_FD_DEBUG, "PADDR 0x%p\n", phys->start);
        fprintf(VFS_FD_DEBUG, "dest %p\n", dest);
        fprintf(VFS_FD_DEBUG, "size 0x%x/0x%x\n", proc_size, proc_size_align);

        memset((void *)dest, 0, elf_prog_header.memory_size);
        // only bss dose not have this
        if (elf_prog_header.file_size != 0)
        {
            fseek(fd, elf_prog_header.offset, SEEK_SET);
            bytes_read = fread((void *)dest, elf_prog_header.file_size, 1, fd);
            log_info(MODULE, "expected %u did %d", elf_prog_header.file_size, bytes_read);
            if (bytes_read != elf_prog_header.file_size)
            {
                log_err(MODULE, "didn't read enough bytes expected %u did %d", elf_prog_header.file_size, bytes_read);
            }
        }

        hexdump((void *)dest, 16);
    }

    fprintf(VFS_FD_DEBUG, "procress size = 0x%x/0x%x\n", proc_size, proc_size_align);
    // the the VIRTUAL entry point
    proc->entry = (vaddr_t)elf_header.program_entry_position;
    log_debug(MODULE, "pep = %p entry = %p", elf_header.program_entry_position, proc->entry);

    fprintf(VFS_FD_DEBUG, "============== END_ELF ==============\n");
    return RETURN_GOOD;
}
#endif

int ELF_read(fd_t fd, process_t *proc, loader *loader)
{
    fprintf(VFS_FD_DEBUG, "============== ELF ==============\n");
    log_debug(MODULE, "ELF_read(%u, %p, %p)", fd, proc, loader);
    log_debug(MODULE, "proc (%p) = {%u}", proc, proc->entry);
    uint8_t buffer[1024];
    ELF_header elf_header;
    fseek(fd, 0, SEEK_SET);
    // something is fucked in the read function tried to fix it
    // with a seek but have not tested it
    // i fucking hate loader and the ELF format
    // just why?
    //
    // - BjornBEs, 16-03-2026 at 11:15 pm
    size_t header_size = sizeof(ELF_header);
#ifdef __x86_64__
    header_size += sizeof(ELF_header64);
#endif
    size_t bytes_read = fread(buffer, header_size, 1, fd);
    if (bytes_read < header_size)
    {
        // something is wrong
        log_err(MODULE, "Didn't read the header from file read %u bytes", bytes_read);
        return RETURN_ERROR;
    }
    memcpy(&elf_header, buffer, header_size);
    hexdump(&elf_header, header_size);

    if (elf_header.type != ELF_TYPE_EXECUTABLE)
    {
        log_err(MODULE, "ELF file is not executable");
        return RETURN_FAILED;
    }

    if (elf_header.bitness == ELF_BITNESS_64BIT)
    {
#ifdef __x86_64__
        return ELF_read64(fd, proc, loader, *(ELF_header64 *)(void *)&elf_header);
#else
        log_err(MODULE, "program is a 64 bit elf that can't be run on a 32 bit system");
        return RETURN_ERROR;
#endif
    }
    log_debug(MODULE, "got header as with %u bytes", bytes_read);

    // the the VIRTUAL entry point
    log_debug(MODULE, "before entry at 0x%x", elf_header.program_entry_position);
    log_debug(MODULE, "proc = %p", proc);
    log_debug(MODULE, "after entry at 0x%x", elf_header.program_entry_position);

    // time to load the program
    uint8_t *dest;

    for (size_t i = 0; i < elf_header.program_header_table_entry_count; i++)
    {
        ELF_program_header elf_prog_header;
        uint32_t prog_header_offset = elf_header.program_header_table_position + i * elf_header.program_header_table_entry_size;

        log_debug(MODULE, "program header table postion = %u", elf_header.program_header_table_position);
        log_debug(MODULE, "program header table entry size = %u", elf_header.program_header_table_entry_size);
        log_debug(MODULE, "program header offset = %u", prog_header_offset);
        fseek(fd, prog_header_offset, SEEK_SET);

        bytes_read = fread(buffer, sizeof(ELF_program_header), 1, fd);
        if (bytes_read < sizeof(ELF_program_header))
        {
            // something is wrong
            log_err(MODULE, "Didn't read program header %u from file", i);
            return RETURN_ERROR;
        }
        memcpy(&elf_prog_header, buffer, sizeof(ELF_program_header));

        log_debug(MODULE, "program header type = %u", elf_prog_header.type);

        if (elf_prog_header.type != ELF_PROGRAM_TYPE_LOAD)
        {
            continue;
        }
        vaddr_t virt_address = (vaddr_t)elf_prog_header.virtual_address;
        paddr_t phys = mmu_alloc_and_map_region(proc->page_dir, virt_address, elf_prog_header.memory_size, text_flags);

        // proc->load_base = (vaddr_t)(uint32_64)elf_prog_header.virtual_address;
        dest = (uint8_t *)virt_address;
        log_debug(MODULE, "ELF: load seg %u", i);
        log_debug(MODULE, "     VADDR 0x%x", virt_address);
        log_debug(MODULE, "     PADDR 0x%x", phys);
        log_debug(MODULE, "     file size %u mem size %u", elf_prog_header.file_size, elf_prog_header.memory_size);
        log_debug(MODULE, "     dest 0x%x", dest);

        memset(dest, 0, elf_prog_header.memory_size);

        fseek(fd, elf_prog_header.offset, SEEK_SET);

        fread(dest, elf_prog_header.file_size, 1, fd);
    }

    proc->entry = (vaddr_t)elf_header.program_entry_position;
    fprintf(VFS_FD_DEBUG, "============== END_ELF ==============");
    return RETURN_GOOD;
}
*/

int ELF_probe(uint8_t *buffer, loader *loader)
{
    elf32_header_t *header = (elf32_header_t *)buffer;
    log_debug(MODULE, "probe with %02x%c%c%c", header->magic[0], header->magic[1], header->magic[2], header->magic[3]);

    if (header->magic[0] != 0x7F ||
        header->magic[1] != 'E' ||
        header->magic[2] != 'L' ||
        header->magic[3] != 'F')
    {
        return RETURN_FAILED;
    }

    return RETURN_GOOD;
}

void ELF_init()
{
    log_debug("ELF", "init ELF");
    loader *load = malloc(sizeof(loader));
    load->name = "ELF";
    load->probe = ELF_probe;
    load->load = ELF_read;
    Loader_register_loader(load);
}
