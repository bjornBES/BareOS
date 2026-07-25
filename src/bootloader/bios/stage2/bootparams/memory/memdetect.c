/*
 * File: memdetect.c
 * File Created: 06 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 18 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#include "memdetect.h"
#include "x86.h"
#include "stdio.h"

void DetectMemory(boot_params_t* bt)
{
    E820_memory_block block;
    uint32_t continuation = 0;
    int ret;
    
    int g_MemRegionCount = 0;
    ret = x86_E820GetNextBlock(&block, &continuation);

    while (ret > 0 && continuation != 0)
    {
        bt->memory.entries[g_MemRegionCount].addr = block.addr;
        bt->memory.entries[g_MemRegionCount].size = block.size;
        bt->memory.entries[g_MemRegionCount].type = block.type;
        g_MemRegionCount++;

        printf("E820: base=0x%llx length=0x%llx type=0x%x\n", block.addr, block.size, block.type);
        ret = x86_E820GetNextBlock(&block, &continuation);
    }
    bt->memory.count = g_MemRegionCount;
    printf("E820: count=%u\n", bt->memory.count);
    bt->memory.max_count = MAX_MEMORY_ENTRIES;
}