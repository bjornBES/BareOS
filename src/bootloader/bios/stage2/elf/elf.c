/*
 * File: elf.c
 * File Created: 27 Aug 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Aug 2026
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"
#include "fat/fat.h"

#include <elf/elf_header.h>
#include <elf/elf_types.h>

#include "memdefs.h"

#include "minmax.h"
#include "memory.h"
#include "stdio.h"

extern void hexdump(void *ptr, int len);

int elf64_read(partition_t *part, fat_file_t *file, boot_params_t *bp)
{
    elf64_header_t header;
    file->position = 0;
    if (fat_read(part, file, sizeof(elf64_header_t), &header) != sizeof(elf64_header_t))
    {
        printf("ELF: header read failed\n");
        fat_close(part, file);
        return 1;
    }

    bp->kernel_entry_point = header.e_entry;
    // printf("ELF: kernel_entry_point = %llp\n", header.e_entry);
    // printf("ELF: kernel_entry_point = %llp\n", bp->kernel_entry_point);

    size_t number_of_ph = header.e_phnum;
    uint16_t ph_size = header.e_phentsize;
    uint64_t ph_offset = header.e_phoff;

    uint8_t *dest;

    for (size_t i = 0; i < number_of_ph; i++)
    {
        elf64_program_header_t elf_ph;
        off_t ph_position = ph_offset + i * ph_size;

        if (fat_seek(part, file, ph_position) != 0)
        {
            printf("ELF: ph_pos base = 0x%x\n", ph_offset);
            printf("ELF: ph_pos index = 0x%x\n", i * ph_size);
            printf("ELF: PH seek failed\n");
            fat_close(part, file);
            return 1;
        }
        if (fat_read(part, file, ph_size, &elf_ph) != ph_size)
        {
            printf("ELF: PH read failed\n");
            fat_close(part, file);
            return 1;
        }

        printf("ELF: %u seg %u\n", elf_ph.type, i);
        printf("\ttype 0x%x flags 0x%x\n", elf_ph.type, elf_ph.flags);
        printf("\toffset 0x%x\n", elf_ph.offset);
        printf("\tvaddr 0x%llp\n", elf_ph.virtual_address);
        printf("\tpaddr 0x%llp\n", elf_ph.physical_address);
        printf("\tfile size %u mem size %u\n", elf_ph.file_size, elf_ph.memory_size);
        printf("\tfile size 0x%x mem size 0x%x\n", elf_ph.file_size, elf_ph.memory_size);
        printf("\talign 0x%x\n", elf_ph.align);
        printf("\tphdr @ 0x%p\n", &elf_ph);

        if (elf_ph.type != PT_LOAD)
        {
            continue;
        }

        if ((elf_ph.flags & PF_EXEC) == PF_EXEC && bp->kernel_phys_base == 0)
        {
            // printf("ELF: kernel_entry_point = %llp\n", bp->kernel_entry_point);
            bp->kernel_phys_base = elf_ph.physical_address;
            bp->kernel_virt_base = elf_ph.virtual_address;
            // printf("ELF: kernel_entry_point = %llp\n", bp->kernel_entry_point);
        }

        bp->kernel_size += elf_ph.memory_size;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        dest = (uint8_t *)((uint64_t*)elf_ph.physical_address);
#pragma GCC diagnostic pop
        memset(dest, 0, elf_ph.memory_size);

        if (fat_seek(part, file, elf_ph.offset) != 0)
        {
            printf("ELF: segment seek failed\n");
            fat_close(part, file);
            return 1;
        }

        uint64_t remaining = elf_ph.file_size;
        uint8_t *p = (void *)dest;

        while (remaining > 0)
        {
            uint32_t chunk = min(remaining, MEMORY_LOAD_SIZE);
            printf("chunk = %u\n", chunk);

            if (fat_read(part, file, chunk, p) != chunk)
            {
                printf("ELF: segment read failed\n");
                fat_close(part, file);
                return false;
            }

            // hexdump(p, chunk);

            p += chunk;
            remaining -= chunk;
        }
    }

    fat_close(part, file);
    printf("ELF: kernel loaded successfully\n");
    // hexdump((void *)bp->kernel_phys_base, 16);
    return 0;
}

int elf_read_kernel(partition_t *part, boot_params_t *bp)
{
    fat_file_t *kernel = fat_open(part, "boot/kernel.elf");
    if (kernel == NULL)
    {
        // the kernel does not exist in the disk
        printf("ELF: error boot/kernel.elf isn't on the given partition\n");
        return 1;
    }

    elf32_header_t header;
    fat_read(part, kernel, EI_NIDENT, &header);

    if (header.magic32 == 0x464C457F)
    {
        if (header.e_ident32 == (0x00010102))
        {
            return elf64_read(part, kernel, bp);
        }
        else
        {
            printf("kernel is a elf32\n");
            printf("Elf32 is not implemented TODO\n");
            // TODO write the elf32 part
            fat_close(part, kernel);
            return 1;
        }
    }

    return 0;
}
