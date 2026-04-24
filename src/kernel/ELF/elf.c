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
#include "memory/paging/paging.h"

#include "kernel.h"

#include "libs/malloc.h"
#include "libs/memory.h"

#define MODULE "ELF"

extern void hexdump(void *ptr, int len);
#ifdef __x86_64__
int ELF_read64(fd_t fd, process *proc, loader *loader, ELF_header64 elf_header)
{
    log_debug(MODULE, "ELF_read(%u, %p, %p)", fd, proc, loader);
    log_debug(MODULE, "proc (%p) = {%u}", proc, proc->entry);
    uint8_t buffer[1024];

    // time to load the program
    uint8_t *dest;
    log_debug(MODULE, "Header table count %u", elf_header.program_header_table_entry_count);

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

        log_debug(MODULE, "program header type = %u", elf_prog_header.type);
        log_debug(MODULE, "program header flags = %x", elf_prog_header.flags);

        if (elf_prog_header.type != ELF_PROGRAM_TYPE_LOAD)
        {
            continue;
        }
        if (elf_prog_header.flags & 1)
        {
            proc->load_base = (virt_addr)elf_prog_header.virtual_address;
            proc->load_end = (virt_addr)elf_prog_header.virtual_address;
            proc->phys_load_base = paging_alloc_frame();
        }
        uint64_t offset = elf_prog_header.virtual_address - (uint64_t)proc->load_base;
        phys_addr phys = paging_alloc_frame_region(proc->phys_load_base, (size_t)elf_prog_header.memory_size) + offset;
        proc->load_end += elf_prog_header.memory_size;
        dest = (uint8_t *)elf_prog_header.virtual_address;
        log_debug(MODULE, "ELF: load seg %u", i);
        log_debug(MODULE, "     VADDR 0x%p", elf_prog_header.virtual_address);
        log_debug(MODULE, "     PADDR 0x%p", phys);
        log_debug(MODULE, "     file size %u mem size %u", elf_prog_header.file_size, elf_prog_header.memory_size);
        log_debug(MODULE, "     dest %p", dest);
        
        paging_print_out = false;
        paging_map_region(kernel_page, (virt_addr)elf_prog_header.virtual_address, phys, elf_prog_header.memory_size, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        paging_map_region(proc->page_dir_virt, (virt_addr)elf_prog_header.virtual_address, phys, elf_prog_header.memory_size, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        paging_print_out = true;
        memset(dest, 0, elf_prog_header.memory_size);

        VFS_seek(fd, elf_prog_header.offset);

        VFS_read(fd, dest, elf_prog_header.file_size);

        hexdump(dest, 16);
    }

    paging_free_region(kernel_page, proc->load_base, proc->load_end - proc->load_base);

    // the the VIRTUAL entry point
    proc->entry = (virt_addr)elf_header.program_entry_position;
    log_debug(MODULE, "pep = %p entry = %p", elf_header.program_entry_position, proc->entry);

    return RETURN_GOOD;
}
#endif

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
        return ELF_read64(fd, proc, loader, *(ELF_header64*)(void*)&elf_header);
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
        phys_addr *phys = paging_alloc_and_map_region(proc->page_dir_virt, (virt_addr)(uint32_64)elf_prog_header.virtual_address, elf_prog_header.memory_size, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);

        proc->load_base = (virt_addr)(uint32_64)elf_prog_header.virtual_address;
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
    log_debug("ELF", "init ELF");
    loader *load = malloc(sizeof(loader));
    load->name = "ELF";
    load->probe = ELF_probe;
    load->load = ELF_read;
    Loader_register_loader(load);
}
