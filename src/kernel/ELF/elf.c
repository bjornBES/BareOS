/*
 * File: elf.c
 * File Created: 15 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"

#include "task/loader.h"
#include "device/device.h"
#include "task/process.h"

#include "debug/debug.h"
#include "paging/paging.h"

#include "kernel.h"
#include "malloc.h"

#include <memory.h>

#define MODULE "ELF"

int ELF_read(fd_t fd, process *proc, loader *loader)
{
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
    size_t bytes_read = VFS_read(fd, buffer, sizeof(ELF_header));
    if (bytes_read < sizeof(ELF_header))
    {
        // something is wrong
        log_err(MODULE, "Didn't read the header from file read %u bytes", bytes_read);
        return RETURN_ERROR;
    }
    memcpy(&elf_header, buffer, sizeof(ELF_header));
    log_debug(MODULE, "got header as with %u bytes", bytes_read);

    // the the VIRTUAL entry point
    log_debug(MODULE, "before entry at 0x%x", elf_header.program_entry_position);
    log_debug(MODULE, "proc = %p", proc);
    log_debug(MODULE, "after entry at 0x%x", elf_header.program_entry_position);

    // time to load the program
    uint8_t *dest;
    int elfSize = 0;

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
        void *phys = paging_alloc_and_map_region(proc->page_dir_phys, (void *)elf_prog_header.virtual_address, elf_prog_header.memory_size, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

        proc->load_base = elf_prog_header.virtual_address;
        dest = (uint8_t *)phys_to_virt(phys);
        log_debug(MODULE, "ELF: load seg %u", i);
        log_debug(MODULE, "     VADDR 0x%x", elf_prog_header.virtual_address);
        log_debug(MODULE, "     PADDR 0x%x", phys);
        log_debug(MODULE, "     file size %u mem size %u", elf_prog_header.file_size, elf_prog_header.memory_size);
        log_debug(MODULE, "     dest 0x%x", dest);

        memset(dest, 0, elf_prog_header.memory_size);

        VFS_seek(fd, elf_prog_header.offset);

        VFS_read(fd, dest, elf_prog_header.file_size);
    }

    proc->entry = elf_header.program_entry_position;

    return RETURN_GOOD;
}

int ELF_probe(uint8_t *buffer, loader *loader)
{
    ELF_header *header = (ELF_header *)buffer;

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
    loader *load = malloc(sizeof(loader));
    load->name = "ELF";
    load->probe = ELF_probe;
    load->load = ELF_read;
    Loader_register_loader(load);
}
