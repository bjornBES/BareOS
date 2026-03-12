#include "Equipment.h"
#include "x86.h"
#include "memory.h"
#include "stdio.h"
#include <core/Defs.h>
#include <core/arch/i686/bios.h>
#include <util/binary.h>

#define EQUIPMENT bp->equipment

void DetectEquipment(boot_params* bp)
{
    memset(&bp->equipment, 0, sizeof(equipment_flags));

    biosregs inRegs;
    initregs(&inRegs);
    inRegs.eax = 0;
    intcall(0x11, &inRegs, &inRegs);

    EQUIPMENT.hasFpu = BIT_GET(inRegs.eax, 1);
    EQUIPMENT.hasCoprocessor = BIT_GET(inRegs.eax, 2);
    EQUIPMENT.floppyFlag = BIT_GET(inRegs.eax, 0);
    if (EQUIPMENT.floppyFlag == 1)
    {
        EQUIPMENT.numFloppies = (BIT_GET(inRegs.eax, 7) << 1) | BIT_GET(inRegs.eax, 6);
    }
}