#include "memdetect.h"
#include "x86.h"
#include "stdio.h"

void DetectMemory(boot_params* bt)
{
    E820MemoryBlock block;
    uint32_t continuation = 0;
    int ret;
    
    int g_MemRegionCount = 0;
    ret = x86_E820GetNextBlock(&block, &continuation);

    while (ret > 0 && continuation != 0)
    {
        bt->e820Entries[g_MemRegionCount].addr = block.addr;
        bt->e820Entries[g_MemRegionCount].size = block.size;
        bt->e820Entries[g_MemRegionCount].type = block.type;
        ++g_MemRegionCount;

        printf("E820: base=0x%llx length=0x%llx type=0x%x\n", block.addr, block.size, block.type);
        ret = x86_E820GetNextBlock(&block, &continuation);
    }
    bt->e820Count = g_MemRegionCount;
}