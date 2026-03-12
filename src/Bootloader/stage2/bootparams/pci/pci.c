#include "pci.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"
#include <core/Defs.h>
#include <core/arch/i686/bios.h>

void DetectPCI(boot_params* bp)
{
    memset(&bp->pciBios, 0, sizeof(PCI_bios_info));

    biosregs inRegs;
    initregs(&inRegs);
    inRegs.ax = 0xB101;
    inRegs.edi = 0;
    intcall(0x1A, &inRegs, &inRegs);

    if ((inRegs.flags & X86_EFLAGS_ZF) == 0 && inRegs.ah == 0)
    {
        printf("PCI is installed\n");
        bp->pciBios.majorVersion = inRegs.bh;
        bp->pciBios.minorVersion = inRegs.bl;
        bp->pciBios.lastBus = inRegs.cl;
        bp->pciBios.numDevices = 0;
        bp->pciBios.signature = inRegs.edx;
    }
    else
    {
        printf("PCI is not installed ah=%x\n", inRegs.ah);
    }
    return;
}
