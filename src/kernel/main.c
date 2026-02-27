/*
 * File: main.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Feb 2026
 * Modified By: BjornBEs
 * -----
 */


#include <stdio.h>
#include <stdint.h>
#include <boot/bootparams.h>
#include <memory.h>
#include <IO.h>

#include "arch/i686/i686.h"
#include "hal/hal.h"
#include "paging/paging.h"

#include "debug.h"

void crash_me();

void main(BootParams* bootParams)
{
    clear();
    printf("Hello world from Kernel\n");
    
    HALInit();
    
    page_directory = bootParams->pageDirectory;

    enableInterrupts();


    ASM_INT2();

    BootLoaderInfo BLInfo = bootParams->bootLoader;
    printf("Bootloader %s Info\n", BLInfo.bootloaderName);
    printf("Flags = %d\n", BLInfo.bootFlags);
    printf("Cl args = %s\n", BLInfo.cmdline);
    
    printf("E820 Entries %u count\n", bootParams->e820Count);
    E820Entry *page_table = NULL;
    {
        char* memoryType[5];
        memoryType[0] = "AVAILABLE";
        memoryType[1] = "RESERVED";
        memoryType[2] = "ACPI_RECLAIMABLE";
        memoryType[3] = "NVS";
        memoryType[4] = "BADRAM";
        #define _4GB 4294967292
        for (size_t i = 0; i < bootParams->e820Count; i++)
        {
            E820Entry entry = bootParams->e820Entries[i];
            log_debug("MAIN", "\t%u: 0x%llx 0x%llx %u %s", i, entry.addr, entry.size, entry.type, memoryType[entry.type - 1]);
            if (entry.type == MEMORY_AVAILABLE && page_table == NULL)
            {
                if (entry.addr < _4GB && entry.size >= 1024 * 1024) // 1 Mb
                {
                    log_debug("main", "found entry for page table %u", i);
                    page_table = &bootParams->e820Entries[i];
                }
                else
                {
                    // for 64 bit systems
                }
            }
        }
        #undef _4GB
    }

    
    ASM_INT2();
    log_debug("main", "page table address 0x%llx", page_table->addr);
    uint32_t address = (uint32_t)page_table->addr;
    init_paging((void*)address);
    
    uint16_t mode = bootParams->currentMode;
    VESAMode *vesaMode = NULL;
    log_debug("main", "vesa count = %u", bootParams->vesaModeCount);
    for (size_t i = 0; i < bootParams->vesaModeCount; i++)
    {
        VESAMode* element = &bootParams->vesaModes[i];
        log_debug("main", "is %i == %i", element->mode, mode);
        if (element->mode == mode)
        {
            vesaMode = element;
            break;
        }
    }
    if (vesaMode == NULL)
    {
        log_crit("main", "Something is fucked");
    }
    
    log_debug("main", "vesa mode %d %dx%dx%d\n", vesaMode->mode, vesaMode->width, vesaMode->height, vesaMode->bitsPerPixel);
    
    uint32_t* fb = (uint32_t*)vesaMode->framebuffer;

    // map_page(page_table, page_table);
    mark_page_used(GetPageDirectoryIndex(fb));
    map_page_4kb_blocks((void*)fb, (void*)fb, 512);
    
    // [Paging] addr = 0x00400000
    void* addr = get_physaddr((void*)fb);
    log_debug("Paging", "addr = %p", addr);
    uint8_t* u8fb = (uint8_t*)fb;
    for (size_t i = 0; i < (vesaMode->width * 3) * 5; i += 3)
    {
        u8fb[0 + i] = 0xFF;
        u8fb[1 + i] = 0x00;
        u8fb[2 + i] = 0x00;
    }
    

    // loop
    for (;;)
        ;
}