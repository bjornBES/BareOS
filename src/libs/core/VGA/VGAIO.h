#pragma once

#include <stdint.h>

#define AttributeControllerWriteIndexData (int)0x3C0
#define AttributeControllerReadData (int)0x3C1
#define MiscellaneousOutputWrite (int)0x3C2
#define MiscellaneousOutputRead (int)0x3CC
#define SequencerIndex (int)0x3C4
#define SequencerData (int)0x3C5
#define GraphicsControllerIndex (int)0x3CE
#define GraphicsControllerData (int)0x3CF
#define DACMask (int)0x3C6 // Should normally be set to 0xFF
#define DACIndexForReadData (int)0x3C7
#define DACIndexForWriteData (int)0x3C8
#define DACData (int)0x3C9
#define FeatureControlRead (int)0x3CA
#define FeatureControlWriteColour (int)0x3DA
#define FeatureControlWriteMono (int)0x3BA
#define CRTControllerIndexColour (int)0x3D4 // If Bit 0 of MiscellaneousOutput set, this is 0x3B4, [Registers 0 to 7] protected by [Bit 7 of Reg 17 (0x11)]
#define CRTControllerIndexMono (int)0x3B4
#define CRTControllerDataColour (int)0x3D5
#define CRTControllerDataMono (int)0x3B5

extern int DACState;
extern int InputStatus0;
extern int InputStatus1Colour; // If Bit 0 (Mono mode) of MiscellaneousOutput set, this is 0x3BA
extern int InputStatus1Mono;

void Write_Byte(int port, uint8_t aVal);
uint8_t Read_Byte(int port);
void IOInit();