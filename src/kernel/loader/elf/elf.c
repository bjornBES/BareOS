/*
 * File: elf.c
 * File Created: 04 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 04 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"
#include "elf_types.h"

#include "vfs/vfs.h"

#include "asm/page_arch.h"

#include "mm/vma/vma.h"

#include "loader/loader.h"

#include "debug/debug.h"

#include "memory.h"

#include "init.h"

#include "align.h"

#define MODULE "elf"

size_t elf_read_file(vfs_node_t *file, void *buffer, size_t buffer_size, size_t count, off_t offset)
{
    // ENTER_FUNC(MODULE, "%u, %p, %u, %u, %u", file, buffer, buffer_size, count, offset);
    vfs_seek(file, offset, SEEK_SET);
    ssize_t size;
    vfs_read(file, buffer, offset, buffer_size * count, &size);
    return size;
}

status_t elf_read_program_header(process_t *proc, file_ref_t *file, elf_program_header_t *elf_prog_header, elf_priv_t *priv)
{
    vaddr_t dest = (vaddr_t)elf_prog_header->virtual_address;
    priv->program_size += elf_prog_header->memory_size;
    size_t size_in_bytes = PAGE_ALIGN_UP(elf_prog_header->memory_size);
    trace_debug(MODULE, "proc_size (%u) > proc_size_align (%u)", priv->program_size, priv->program_size_align);
    trace_debug(MODULE, "proc_size (%u) > proc_size_align + size_in_bytes (%u)", priv->program_size, priv->program_size_align + size_in_bytes);

    priv->program_size_align += size_in_bytes;
    trace_info(MODULE, "loading proc->vma = %p", proc->vma);
    // vaddr_t offset = GET_PAGE_OFFSET(dest);

    mmu_flags_t flags;
    vma_type_t type;
    if (elf_prog_header->type == PT_LOAD)
    {
        if ((elf_prog_header->flags & PF_EXEC) == PF_EXEC)
        {
            type = VMA_PROC_TEXT;
            flags = text_flags;
        }
        else if (FLAG_IS_SET(elf_prog_header->flags, PF_WRITE))
        {
            type = VMA_PROC_DATA;
            flags = data_flags;
        }
        else if (elf_prog_header->flags == PF_READ)
        {
            type = VMA_PROC_TEXT;
            flags = data_flags;
        }
    }
    // flags.present = 0;
    vma_t *vma = do_mmap_eager(proc->vma, dest, size_in_bytes, type, flags);
    vma->file_offset = elf_prog_header->offset;
    vma->file_size = elf_prog_header->file_size;
    vma->file = file;
    file->refcount++;

    // trace_info(MODULE, "PADDR 0x%p\n", phys->start);
    trace_info(MODULE, "dest %p\n", dest);
    trace_info(MODULE, "size 0x%x/0x%x\n", priv->program_size, priv->program_size_align);

    memset((void *)dest, 0, elf_prog_header->memory_size);
    // only bss dose not have this
    if (elf_prog_header->file_size != 0)
    {
        size_t bytes_read = elf_read_file(file->fd, (void *)dest, elf_prog_header->file_size, 1, elf_prog_header->offset);
        trace_info(MODULE, "expected %u did %d", elf_prog_header->file_size, bytes_read);
        if (bytes_read != (size_t)elf_prog_header->file_size)
        {
            trace_err(MODULE, "didn't read enough bytes expected %u did %d", elf_prog_header->file_size, bytes_read);
            return KERRNO_UNSUCCESS;
        }
    }

    // {
    //     hexdump((void *)dest, 16, 16);
    // }

    return KERRNO_SUCCESSES;
}

status_t elf_read_program(vfs_node_t *file, process_t *proc, elf_header_t *header, elf_priv_t *priv, loader_t *loader)
{
    uint16_t prog_header_count = header->e_phnum;
    uint16_t prog_header_entry_size = header->e_phentsize;
    uint64_t prog_header_header_table_offset = header->e_phoff;

    proc->loader_data.entry = (vaddr_t)header->e_entry;

    void *buffer = kzmalloc(prog_header_count * prog_header_entry_size);
    file_ref_t *file_ref = kzmalloc(sizeof(file_ref_t));
    file_ref->fd = file;

    size_t bytes_read = elf_read_file(file, buffer, prog_header_entry_size, prog_header_count, prog_header_header_table_offset);
    if (bytes_read < prog_header_count * prog_header_entry_size)
    {
        // something is wrong
        trace_err(MODULE, "Didn't read program header from file");
        return KERRNO_UNSUCCESS;
    }

    trace_debug(MODULE, "program header table postion = %u", prog_header_header_table_offset);
    trace_debug(MODULE, "program header table entry size = %u", prog_header_entry_size);

    for (size_t i = 0; i < prog_header_count; i++)
    {
        trace_debug(MODULE, "program header index = %u", i);
        elf_program_header_t elf_prog_header;
        if (header->bitness == ELF_CLASS_64)
        {
            elf64_program_header_t *entry64 = (elf64_program_header_t *)(buffer + (i * sizeof(elf64_program_header_t)));
            elf_prog_header.type = entry64->type;
            elf_prog_header.offset = entry64->offset;
            elf_prog_header.virtual_address = entry64->virtual_address;
            elf_prog_header.physical_address = entry64->physical_address;
            elf_prog_header.file_size = entry64->file_size;
            elf_prog_header.memory_size = entry64->memory_size;
            elf_prog_header.flags = entry64->flags;
            elf_prog_header.align = entry64->align;
            // memcpy(&elf_prog_header, entry64, sizeof(elf64_program_header_t));
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

        trace_info(MODULE, "ELF: load seg %u", i);
        trace_info(MODULE, "\ttype 0x%x flags 0x%x", elf_prog_header.type, elf_prog_header.flags);
        trace_info(MODULE, "\toffset 0x%x", elf_prog_header.offset);
        trace_info(MODULE, "\tvaddr %p", elf_prog_header.virtual_address);
        trace_info(MODULE, "\tfile size %u mem size %u", elf_prog_header.file_size, elf_prog_header.memory_size);
        trace_info(MODULE, "\tfile size 0x%x mem size 0x%x", elf_prog_header.file_size, elf_prog_header.memory_size);
        trace_info(MODULE, "\talign 0x%x", elf_prog_header.align);
        trace_info(MODULE, "\tphdr @ %p", &elf_prog_header);

        // if (elf_prog_header.type == PT_DYNAMIC)
        // {
        //     elf_read_file(file, buffer, elf_prog_header.file_size, 1, elf_prog_header.offset);
        //     elf_dym_read(&elf_prog_header, file, proc, header, buffer, priv, loader);
        //     linker_run(file, priv->interpreters[priv->interpreter_count - 1], proc, loader);
        //     continue;
        // }
        // if (elf_prog_header.type == PT_INTERP)
        // {
        //     priv->interpreters[priv->interpreter_count] = malloc(elf_prog_header.file_size);
        //     elf_read_file(file, priv->interpreters[priv->interpreter_count], elf_prog_header.file_size, 1, elf_prog_header.offset);
        //     priv->interpreter_count++;
        //     continue;
        // }

        if (elf_prog_header.type != PT_LOAD)
        {
            continue;
        }

        if (elf_prog_header.memory_size == 0)
        {
            continue;
        }

        elf_read_program_header(proc, file_ref, &elf_prog_header, priv);
    }

    trace_info(MODULE, "entry = 0x%x", proc->loader_data.entry);
    return KERRNO_SUCCESSES;
}

int elf_read(vfs_node_t *file, process_t *proc, loader_t *loader, elf_header_t *header)
{
    ENTER_FUNC("%p, %p, %p, %p", file, proc, loader, header);

    elf_priv_t *priv = loader->priv;

    status_t ret;

    // status_t ret = elf_get_sections(file, proc, header, priv);
    // if (ret)
    // {
    // return ret;
    // }
    // trace_info(MODULE, "got sections");
    // hexdump(priv->sections, priv->section_count * sizeof(elf_section_header_t), sizeof(elf_section_header_t));

    // trace_info(MODULE, "reading sections");
    // elf_read_notes(file, proc, header, priv);

    trace_info(MODULE, "loading program");
    ret = elf_read_program(file, proc, header, priv, loader);
    if (ret)
    {
        return ret;
    }
    return KERRNO_SUCCESSES;
}

status_t elf_load(vfs_node_t *file, process_t *proc, loader_t *loader)
{
    uint8_t buffer[1024];
    elf32_header_t *header = (elf32_header_t *)buffer;

    elf_read_file(file, buffer, 1024, 1, 0);

    elf_header_t global_header;

    loader->priv = kzmalloc(sizeof(elf_priv_t));

    if (header->bitness == ELF_CLASS_32)
    {
        // return ELF32_read(fd, proc, loader, header);
    }
    else if (header->bitness == ELF_CLASS_64)
    {
        to_elf_header(&global_header, buffer);
        return elf_read(file, proc, loader, &global_header);
    }
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

status_t elf_probe(uint8_t *buffer, loader_t *loader)
{
    elf32_header_t *header = (elf32_header_t *)buffer;

    trace_debug(MODULE, "probe with %02x%c%c%c", header->magic[0], header->magic[1], header->magic[2], header->magic[3]);

    if ((uint64_t)header->magic == ELF_MAGIC)
    {
        return KERRNO_UNSUCCESS;
    }

    return KERRNO_SUCCESSES;
}

loader_t elf_loader = {
    .name = "elf",
    .probe = elf_probe,
    .load = elf_load,
};

status_t elf_init()
{
    return loader_register(&elf_loader);
}

LOADER_INITCALL(elf_init)
