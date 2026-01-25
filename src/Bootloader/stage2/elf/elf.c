#include "elf.h"
#include "fs/fat.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"

#define KERNEL_VMA 0x00100000
#define KERNEL_LMA 0x00100000

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

bool ELF_Read(Partition *part, const char *path, void **entryPoint)
{
    printf("ELF: Opening '%s'\n", path);

    FAT_File *fd = FAT_Open(part, path);
    if (!fd)
    {
        printf("ELF: open failed\n");
        return false;
    }

    ELFHeader eh;
    if (FAT_Read(part, fd, sizeof(eh), &eh) != sizeof(eh))
    {
        printf("ELF: header read failed\n");
        FAT_Close(fd);
        return false;
    }

    /* Basic validation */
    if (eh.Magic[0] != 0x7F ||
        eh.Magic[1] != 'E' ||
        eh.Magic[2] != 'L' ||
        eh.Magic[3] != 'F')
    {
        printf("ELF: bad magic\n");
        FAT_Close(fd);
        return false;
    }

    printf("ELF: entry VADDR = 0x%x\n", eh.ProgramEntryPosition);

    /* Return the VIRTUAL entry point */
    *entryPoint = (void *)(uintptr_t)eh.ProgramEntryPosition;
    uint8_t *dest;
    int elfSize = 0;

    /* Load each PT_LOAD segment */
    for (uint16_t i = 0; i < eh.ProgramHeaderTableEntryCount; i++)
    {
        ELFProgramHeader ph;
        uint32_t ph_pos =
            eh.ProgramHeaderTablePosition +
            i * eh.ProgramHeaderTableEntrySize;

        if (!FAT_Skip(part, fd, ph_pos - fd->Position))
        {
            printf("ELF: PH seek failed\n");
            FAT_Close(fd);
            return false;
        }

        if (FAT_Read(part, fd, sizeof(ph), &ph) != sizeof(ph))
        {
            printf("ELF: PH read failed\n");
            FAT_Close(fd);
            return false;
        }

        if (ph.Type != ELF_PROGRAM_TYPE_LOAD)
            continue;

        /* Translate VADDR → physical */
        if (ph.PhysicalAddress < KERNEL_VMA)
        {
            printf("ELF: segment below kernel VMA\n");
            FAT_Close(fd);
            return false;
        }

        uint32_t phys = ph.PhysicalAddress - KERNEL_VMA + KERNEL_LMA;

        dest = (uint8_t *)(uintptr_t)phys;

        printf("ELF: load seg %u\n", i);
        printf("     VADDR 0x%x\n", ph.PhysicalAddress);
        printf("     PADDR 0x%x\n", phys);
        printf("     filesz %u memsz %u\n", ph.FileSize, ph.MemorySize);

        /* Zero entire segment (BSS included) */
        memset(dest, 0, ph.MemorySize);

        /* Load file-backed portion */
        if (!FAT_Skip(part, fd, ph.Offset - fd->Position))
        {
            printf("ELF: segment seek failed\n");
            FAT_Close(fd);
            return false;
        }

        uint32_t remaining = ph.FileSize;
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

