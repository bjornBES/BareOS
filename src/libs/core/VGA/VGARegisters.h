#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "VGAIO.h"
#include "VGAConstants.h"
#include "VGAEnumerables.h"
#include "Colors.h"

#define CONCAT(a, b) a##b
#define EXPAND_CONCAT(a, b) CONCAT(a, b)

#define MakeIOFunction(name, portIndex, portData, reg, type, writeFunc, readFunc) \
    void Write_##name(type value)                                                 \
    {                                                                             \
        Write_Byte(portIndex, reg);                                               \
        writeFunc(portData, value);                                               \
    }                                                                             \
    type Read_##name()                                                            \
    {                                                                             \
        Write_Byte(portIndex, reg);                                               \
        return (type)readFunc(portData);                                          \
    }

#define MakeIOFunctionSameName(name, portIndex, portData, prefix, type, writeFunc, readFunc) \
    void Write_##name(type value)                                                            \
    {                                                                                        \
        Write_Byte(portIndex, EXPAND_CONCAT(name, prefix));                                  \
        writeFunc(portData, value);                                                          \
    }                                                                                        \
    type Read_##name()                                                                       \
    {                                                                                        \
        Write_Byte(portIndex, EXPAND_CONCAT(name, prefix));                                  \
        return (type)readFunc(portData);                                                     \
    }

#define MakePaletteIOFunction(paletteIndex)                                                  \
    void Write_Palette##paletteIndex(uint8_t value)                                          \
    {                                                                                        \
        Read_Byte(InputStatus1Colour);                                                       \
        Write_Byte(AttributeControllerWriteIndexData, Palette##paletteIndex##RegisterIndex); \
        Write_Byte(AttributeControllerWriteIndexData, value);                                \
    }                                                                                        \
    uint8_t Read_Palette##paletteIndex()                                                     \
    {                                                                                        \
        Read_Byte(InputStatus1Colour);                                                       \
        Write_Byte(AttributeControllerWriteIndexData, Palette##paletteIndex##RegisterIndex); \
        return Read_Byte(AttributeControllerWriteIndexData);                                 \
    }

#define MakeIO3FunctionRWX(name, reg, type)                   \
    void Write_##name(type value)                             \
    {                                                         \
        Read_Byte(InputStatus1Colour);                        \
        Write_Byte(AttributeControllerWriteIndexData, reg);   \
        Write_Byte(AttributeControllerWriteIndexData, value); \
    }                                                         \
    type Read_##name()                                        \
    {                                                         \
        Read_Byte(InputStatus1Colour);                        \
        Write_Byte(AttributeControllerWriteIndexData, reg);   \
        return Read_Byte(AttributeControllerWriteIndexData);  \
    }

#define MakeIO1Function(name, readIndex, writeIndex, type, funcSize) \
    void Write_##name(type value)                                    \
    {                                                                \
        Write_##funcSize(writeIndex, value);                         \
    }                                                                \
    type Read_##name()                                               \
    {                                                                \
        return Read_##funcSize(readIndex);                           \
    }

#define MakeDefIOFunction(name, type) \
    void Write_##name(type value);    \
    type Read_##name();

#define MakeDefPaletteIOFunction(paletteIndex)       \
    void Write_Palette##paletteIndex(uint8_t value); \
    uint8_t Read_Palette##paletteIndex();

MakeDefIOFunction(SetReset, uint8_t);
MakeDefIOFunction(EnableSetReset, uint8_t);
MakeDefIOFunction(ColourCompare, uint8_t);
MakeDefIOFunction(DataRotate, uint8_t);
MakeDefIOFunction(ReadMapSelect, uint8_t);
MakeDefIOFunction(GraphicsMode, GraphicsModeFlags);
MakeDefIOFunction(MiscellaneousGraphics, uint8_t);
MakeDefIOFunction(ColourDontCare, uint8_t);
MakeDefIOFunction(BitMask, uint8_t);

MakeDefIOFunction(Reset, uint8_t);
MakeDefIOFunction(ClockingMode, uint8_t);
MakeDefIOFunction(MapMask, uint8_t);
MakeDefIOFunction(CharacterMapSelect, uint8_t);
MakeDefIOFunction(SequencerMemoryMode, SequencerMemoryModeFlags);

MakeDefPaletteIOFunction(0);
MakeDefPaletteIOFunction(1);
MakeDefPaletteIOFunction(2);
MakeDefPaletteIOFunction(3);
MakeDefPaletteIOFunction(4);
MakeDefPaletteIOFunction(5);
MakeDefPaletteIOFunction(6);
MakeDefPaletteIOFunction(7);
MakeDefPaletteIOFunction(8);
MakeDefPaletteIOFunction(9);
MakeDefPaletteIOFunction(10);
MakeDefPaletteIOFunction(11);
MakeDefPaletteIOFunction(12);
MakeDefPaletteIOFunction(13);
MakeDefPaletteIOFunction(14);
MakeDefPaletteIOFunction(15);

MakeDefIOFunction(AttributeModeControl, uint8_t);
MakeDefIOFunction(OverscanColour, uint8_t);
MakeDefIOFunction(ColourPlaneEnable, uint8_t);
MakeDefIOFunction(HorizontalPixelPanning, uint8_t);
MakeDefIOFunction(ColourSelect, uint8_t);
MakeDefIOFunction(PaletteAddressSource, bool);

MakeDefIOFunction(HorizontalTotal, uint8_t);
MakeDefIOFunction(EndHorizontalDisplay, uint8_t);
MakeDefIOFunction(StartHorizontalBlanking, uint8_t);
MakeDefIOFunction(EndHorizontalBlanking, uint8_t);
MakeDefIOFunction(StartHorizontalRetrace, uint8_t);
MakeDefIOFunction(EndHorizontalRetrace, uint8_t);
MakeDefIOFunction(VerticalTotal, uint8_t);
MakeDefIOFunction(Overflow, uint8_t);
MakeDefIOFunction(PresetRowScan, uint8_t);
MakeDefIOFunction(MaximumScanLine, uint8_t);
MakeDefIOFunction(CursorStart, uint8_t);
MakeDefIOFunction(CursorEnd, uint8_t);
MakeDefIOFunction(StartAddressHigh, uint8_t);
MakeDefIOFunction(StartAddressLow, uint8_t);
MakeDefIOFunction(CursorLocationHigh, uint8_t);
MakeDefIOFunction(CursorLocationLow, uint8_t);
MakeDefIOFunction(VerticalRetraceStart, uint8_t);
MakeDefIOFunction(VerticalRetraceEnd, uint8_t);
MakeDefIOFunction(VerticalDisplayEnd, uint8_t);
MakeDefIOFunction(Offset, uint8_t);
MakeDefIOFunction(UnderlineLocation, uint8_t);
MakeDefIOFunction(StartVerticalBlanking, uint8_t);
MakeDefIOFunction(EndVerticalBlanking, uint8_t);
MakeDefIOFunction(CRTCModeControl, uint8_t);
MakeDefIOFunction(LineCompare, uint8_t);

MakeDefIOFunction(DACMask, uint8_t);
MakeDefIOFunction(DACWriteAddress, uint8_t);
void Write_DACData(Color18Bit* value);
void Read_DACData(Color18Bit* color);

MakeDefIOFunction(MiscellaneousOutput, uint8_t);