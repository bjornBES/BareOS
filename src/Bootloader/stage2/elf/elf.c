/*
 * File: elf.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 01 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "elf.h"
#include "fs/fat.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"

extern uint64_t kernel_entry;

void LogKernelSegment(uint8_t* segmentStart, uint32_t memSize, uint32_t entryOffset)
{
    printf("Kernel segment loaded: [%p - %p], size=%u bytes\n", 
           segmentStart, segmentStart + memSize - 1, memSize);

    uint8_t* entryPtr = segmentStart + entryOffset;
    printf("Entry point at %p, first 16 bytes: ", entryPtr);
    for (int i = 0; i < 16; i++)
    {
        printf("%X ", entryPtr[i]);
    }
    printf("\n");
}

bool ELF_Read64(Partition *part, const char *path, void **entryPoint)
{
    printf("ELF: Opening '%s'\n", path);

    FAT_File *fd = FAT_Open(part, path);
    if (!fd)
    {
        printf("ELF: open failed\n");
        return false;
    }

    ELF_header64 elfHeader;
    if (FAT_Read(part, fd, sizeof(elfHeader), &elfHeader) != sizeof(elfHeader))
    {
        printf("ELF: header read failed\n");
        FAT_Close(fd);
        return false;
    }

    /* Basic validation */
    if (elfHeader.Magic[0] != 0x7F ||
        elfHeader.Magic[1] != 'E' ||
        elfHeader.Magic[2] != 'L' ||
        elfHeader.Magic[3] != 'F')
    {
        printf("ELF: bad magic\n");
        FAT_Close(fd);
        return false;
    }
    
    /* Return the VIRTUAL entry point */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    *entryPoint = (void *)elfHeader.ProgramEntryPosition;
    kernel_entry = elfHeader.ProgramEntryPosition;
    printf("ELF: entry VADDR = 0x%x + 0x%x\n", (kernel_entry) >> 32, kernel_entry);
    #pragma GCC diagnostic pop
    uint8_t *dest;
    int elfSize = 0;

    /* Load each PT_LOAD segment */
    printf("ELF: program header table %u\n", elfHeader.ProgramHeaderTableEntryCount);
    for (uint16_t i = 0; i < elfHeader.ProgramHeaderTableEntryCount; i++)
    {
        ELF_program_header64 progarmHeader;
        uint64_t ph_pos =
            elfHeader.ProgramHeaderTablePosition +
            i * elfHeader.ProgramHeaderTableEntrySize;

        printf("ELF: file dis postion = 0x%x\n", fd->Position);
        if (!FAT_Seek(part, fd, ph_pos))
        {
            printf("ELF: ph_pos base = 0x%x\n", elfHeader.ProgramHeaderTablePosition);
            printf("ELF: ph_pos index = 0x%x\n", i * elfHeader.ProgramHeaderTableEntrySize);
            printf("ELF: PH seek failed\n");
            FAT_Close(fd);
            return false;
        }
        printf("ELF: file dis postion = 0x%x\n", fd->Position);

        if (FAT_Read(part, fd, sizeof(progarmHeader), &progarmHeader) != sizeof(progarmHeader))
        {
            printf("ELF: PH read failed\n");
            FAT_Close(fd);
            return false;
        }

        printf("program header type = %u\n", progarmHeader.Type);

        if (progarmHeader.Type != ELF_PROGRAM_TYPE_LOAD)
            continue;

        /* Translate VADDR → physical */
        if (progarmHeader.VirtualAddress < KERNEL_VMA)
        {
            printf("ELF: kernel virt addr = 0x%x\n", progarmHeader.VirtualAddress);
            printf("ELF: kernel phys addr = 0x%x\n", progarmHeader.PhysicalAddress);
            printf("ELF: segment below kernel VMA\n");
            FAT_Close(fd);
            return false;
        }

        uint64_t phys = progarmHeader.VirtualAddress - KERNEL_VMA + KERNEL_LMA;

        dest = (uint8_t *)(uintptr_t)phys;

        printf("ELF: load seg %u\n", i);
        printf("     VADDR 0x%x\n", progarmHeader.VirtualAddress);
        printf("     PADDR 0x%x/0x%x\n", phys, progarmHeader.PhysicalAddress);
        printf("     file size %u mem size %u\n", progarmHeader.FileSize, progarmHeader.MemorySize);
        printf("     dest 0x%x \n", dest);

        /* Zero entire segment (BSS included) */
        memset(dest, 0, progarmHeader.MemorySize);

        /* Load file-backed portion */
        if (!FAT_Seek(part, fd, progarmHeader.Offset))
        {
            printf("ELF: segment seek failed\n");
            FAT_Close(fd);
            return false;
        }

        uint64_t remaining = progarmHeader.FileSize;
        uint8_t *p = dest;

        while (remaining > 0)
        {
            uint32_t chunk =
                remaining > MEMORY_LOAD_SIZE
                    ? MEMORY_LOAD_SIZE
                    : remaining;

            if (FAT_Read(part, fd, chunk, p) != chunk)
            {
                printf("ELF: segment read failed\n");
                FAT_Close(fd);
                return false;
            }

            p += chunk;
            remaining -= chunk;
            elfSize += chunk;
        }
    }

    FAT_Close(fd);
    printf("ELF: kernel loaded successfully\n");
    LogKernelSegment(dest, elfSize, 0);
    return true;
}


bool ELF_Read(Partition *part, const char *path, void **entryPoint)
{
    printf("ELF: Opening '%s'\n", path);

    FAT_File *fd = FAT_Open(part, path);
    if (!fd)
    {
        printf("ELF: open failed\n");
        return false;
    }

    ELF_header elfHeader;
    if (FAT_Read(part, fd, sizeof(elfHeader), &elfHeader) != sizeof(elfHeader))
    {
        printf("ELF: header read failed\n");
        FAT_Close(fd);
        return false;
    }

    /* Basic validation */
    if (elfHeader.Magic[0] != 0x7F ||
        elfHeader.Magic[1] != 'E' ||
        elfHeader.Magic[2] != 'L' ||
        elfHeader.Magic[3] != 'F')
    {
        printf("ELF: bad magic\n");
        FAT_Close(fd);
        return false;
    }
    
    if (elfHeader.Bitness == ELF_BITNESS_64BIT)
    {
        FAT_Close(fd);
        return ELF_Read64(part, path, entryPoint);
    }

    printf("ELF: entry VADDR = 0x%x\n", elfHeader.ProgramEntryPosition);

    /* Return the VIRTUAL entry point */
    *entryPoint = (void *)(uintptr_t)elfHeader.ProgramEntryPosition + 0xFFFFFFFF00000000;
    uint8_t *dest;
    int elfSize = 0;

    /* Load each PT_LOAD segment */
    printf("ELF: program header table %u\n", elfHeader.ProgramHeaderTableEntryCount);
    for (uint16_t i = 0; i < elfHeader.ProgramHeaderTableEntryCount; i++)
    {
        ELF_program_header progarmHeader;
        uint32_t ph_pos =
            elfHeader.ProgramHeaderTablePosition +
            i * elfHeader.ProgramHeaderTableEntrySize;

        printf("ELF: file dis postion = 0x%x\n", fd->Position);
        if (!FAT_Seek(part, fd, ph_pos))
        {
            printf("ELF: ph_pos base = 0x%x\n", elfHeader.ProgramHeaderTablePosition);
            printf("ELF: ph_pos index = 0x%x\n", i * elfHeader.ProgramHeaderTableEntrySize);
            printf("ELF: PH seek failed\n");
            FAT_Close(fd);
            return false;
        }
        printf("ELF: file dis postion = 0x%x\n", fd->Position);

        if (FAT_Read(part, fd, sizeof(progarmHeader), &progarmHeader) != sizeof(progarmHeader))
        {
            printf("ELF: PH read failed\n");
            FAT_Close(fd);
            return false;
        }

        printf("program header type = %u\n", progarmHeader.Type);

        if (progarmHeader.Type != ELF_PROGRAM_TYPE_LOAD)
            continue;

        /* Translate VADDR → physical */
        if (progarmHeader.VirtualAddress < KERNEL_VMA)
        {
            printf("ELF: kernel virt addr = 0x%x\n", progarmHeader.VirtualAddress);
            printf("ELF: kernel phys addr = 0x%x\n", progarmHeader.PhysicalAddress);
            printf("ELF: segment below kernel VMA\n");
            FAT_Close(fd);
            return false;
        }

        uint32_t phys = progarmHeader.VirtualAddress - KERNEL_VMA + KERNEL_LMA;

        dest = (uint8_t *)(uintptr_t)phys;

        printf("ELF: load seg %u\n", i);
        printf("     VADDR 0x%x\n", progarmHeader.VirtualAddress);
        printf("     PADDR 0x%x\n", phys);
        printf("     file size %u mem size %u\n", progarmHeader.FileSize, progarmHeader.MemorySize);
        printf("     dest 0x%x \n", dest);

        /* Zero entire segment (BSS included) */
        memset(dest, 0, progarmHeader.MemorySize);

        /* Load file-backed portion */
        if (!FAT_Seek(part, fd, progarmHeader.Offset))
        {
            printf("ELF: segment seek failed\n");
            FAT_Close(fd);
            return false;
        }

        uint32_t remaining = progarmHeader.FileSize;
        uint8_t *p = dest;

        while (remaining > 0)
        {
            uint32_t chunk =
                remaining > MEMORY_LOAD_SIZE
                    ? MEMORY_LOAD_SIZE
                    : remaining;

            if (FAT_Read(part, fd, chunk, p) != chunk)
            {
                printf("ELF: segment read failed\n");
                FAT_Close(fd);
                return false;
            }

            p += chunk;
            remaining -= chunk;
            elfSize += chunk;
        }
    }

    FAT_Close(fd);
    printf("ELF: kernel loaded successfully\n");
    // LogKernelSegment(dest, elfSize, 0);
    return true;
}

