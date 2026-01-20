#include "VGAIO.h"
#include "VGAConstants.h"
#include <IO.h>

int DACState;
int InputStatus0;
int InputStatus1Colour;
int InputStatus1Mono;

uint8_t Read_Byte(int port)
{
    return Inb(port);
}

void Write_Byte(int port, uint8_t aVal)
{
    Outb(port, aVal);
}

void IOInit()
{
    DACState = DACIndexForReadData;
    InputStatus0 = MiscellaneousOutputWrite;
    InputStatus1Colour = FeatureControlWriteColour;
    InputStatus1Mono = FeatureControlWriteMono;
}
