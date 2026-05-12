/*
 * File: elf.c
 * File Created: 15 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 May 2026 16:08:06
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"

#include "task/loader.h"
#include "device/device.h"
#include "task/process.h"

#include "debug/debug.h"
#include "memory/paging/paging.h"
#include "memory/vmm/vmm.h"

#include "kernel.h"

#include "libs/malloc.h"
#include "libs/memory.h"
#include "libs/string.h"

#define MODULE "ELF"

extern void hexdump(void *ptr, int len);

process_api_t ELF_getABI(int ELF_abi, int endianness, int bitness)
{
    if (ELF_abi == ELF_ABU_SysV)
    {
        if (bitness == ELF_BITNESS_64BIT)
        {
            return API_SYSV64;
        }
        return API_SYSV32;
    }
    return Undefined;
}

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
        VFS_seek(fd, elf_header.section_header_table_position + section_name_index * section_entry_size);
        VFS_read(fd, &section_name_entry, sizeof(ELF_section_header64));
        char *strtable = malloc(section_name_entry.size);
        {
            VFS_seek(fd, section_name_entry.offset);
            VFS_read(fd, strtable, section_name_entry.size);
        }
        for (size_t i = 0; i < section_count; i++)
        {
            ELF_section_header64 elf_section_entry;
            VFS_seek(fd, elf_header.section_header_table_position + i * section_entry_size);
            int bytes_read = VFS_read(fd, buffer, sizeof(ELF_section_header64));
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
            section->base = (virt_addr)elf_section_entry.virtual_address;
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
    uint8_t *dest;
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
        VFS_seek(fd, prog_header_offset);

        int bytes_read = VFS_read(fd, buffer, sizeof(ELF_program_header64));
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
        dest = (virt_addr)elf_prog_header.virtual_address;
        proc_size += elf_prog_header.memory_size;
        size_t size_in_bytes = ALIGN_2_UP(elf_prog_header.memory_size, PAGE_SIZE);
        if (proc_size >= proc_size_align)
        {
            proc_size_align += size_in_bytes;

            log_info(MODULE, "loading proc->vma = %p", proc->vma);
            phys = do_mmap(proc->vma, dest, elf_prog_header.memory_size, VMA_ANONYMOUS, text_flags);
        }

        fprintf(VFS_FD_DEBUG, "PADDR 0x%p\n", phys->start);
        fprintf(VFS_FD_DEBUG, "dest %p\n", dest);
        fprintf(VFS_FD_DEBUG, "size 0x%x/0x%x\n", proc_size, proc_size_align);

        memset(dest, 0, elf_prog_header.memory_size);
        // only bss dose not have this
        if (elf_prog_header.file_size != 0)
        {
            VFS_seek(fd, elf_prog_header.offset);
            bytes_read = VFS_read(fd, dest, elf_prog_header.file_size);
            log_info(MODULE, "expected %u did %d", elf_prog_header.file_size, bytes_read);
            if (bytes_read != elf_prog_header.file_size)
            {
                log_err(MODULE, "didn't read enough bytes expected %u did %d", elf_prog_header.file_size, bytes_read);
            }
        }

        hexdump(dest, 16);
    }

    fprintf(VFS_FD_DEBUG, "procress size = 0x%x/0x%x\n", proc_size, proc_size_align);
    // the the VIRTUAL entry point
    proc->entry = (virt_addr)elf_header.program_entry_position;
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
    VFS_seek(fd, 0);
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
    size_t bytes_read = VFS_read(fd, buffer, header_size);
    if (bytes_read < header_size)
    {
        // something is wrong
        log_err(MODULE, "Didn't read the header from file read %u bytes", bytes_read);
        return RETURN_ERROR;
    }
    memcpy(&elf_header, buffer, header_size);

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
        VFS_seek(fd, prog_header_offset);

        bytes_read = VFS_read(fd, buffer, sizeof(ELF_program_header));
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
        phys_addr *phys = paging_alloc_and_map_region(proc->page_dir, (virt_addr)(uint32_64)elf_prog_header.virtual_address, elf_prog_header.memory_size, text_flags);

        // proc->load_base = (virt_addr)(uint32_64)elf_prog_header.virtual_address;
        dest = (uint8_t *)paging_phys_to_virt(phys);
        log_debug(MODULE, "ELF: load seg %u", i);
        log_debug(MODULE, "     VADDR 0x%x", elf_prog_header.virtual_address);
        log_debug(MODULE, "     PADDR 0x%x", phys);
        log_debug(MODULE, "     file size %u mem size %u", elf_prog_header.file_size, elf_prog_header.memory_size);
        log_debug(MODULE, "     dest 0x%x", dest);

        memset(dest, 0, elf_prog_header.memory_size);

        VFS_seek(fd, elf_prog_header.offset);

        VFS_read(fd, dest, elf_prog_header.file_size);
    }

    proc->entry = (virt_addr)(uint32_64)elf_header.program_entry_position;
    fprintf(VFS_FD_DEBUG, "============== END_ELF ==============");
    return RETURN_GOOD;
}

int ELF_probe(uint8_t *buffer, loader *loader)
{
    ELF_header *header = (ELF_header *)buffer;
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
