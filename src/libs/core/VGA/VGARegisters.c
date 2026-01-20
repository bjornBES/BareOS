#include "VGARegisters.h"

// Graphics registers
MakeIOFunctionSameName(SetReset, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(EnableSetReset, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(ColourCompare, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(DataRotate, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(ReadMapSelect, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(GraphicsMode, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, GraphicsModeFlags, Write_Byte, Read_Byte);
MakeIOFunctionSameName(MiscellaneousGraphics, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(ColourDontCare, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunctionSameName(BitMask, GraphicsControllerIndex, GraphicsControllerData, RegisterIndex, uint8_t, Write_Byte, Read_Byte);

// Sequencer registers
MakeIOFunction(Reset, SequencerIndex, SequencerData, ResetRegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunction(ClockingMode, SequencerIndex, SequencerData, ClockingModeRegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunction(MapMask, SequencerIndex, SequencerData, MapMaskRegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunction(CharacterMapSelect, SequencerIndex, SequencerData, CharacterMapSelectRegisterIndex, uint8_t, Write_Byte, Read_Byte);
MakeIOFunction(SequencerMemoryMode, SequencerIndex, SequencerData, SequencerMemoryModeRegisterIndex, SequencerMemoryModeFlags, Write_Byte, Read_Byte);

// Attribute registers
MakePaletteIOFunction(0);
MakePaletteIOFunction(1);
MakePaletteIOFunction(2);
MakePaletteIOFunction(3);
MakePaletteIOFunction(4);
MakePaletteIOFunction(5);
MakePaletteIOFunction(6);
MakePaletteIOFunction(7);
MakePaletteIOFunction(8);
MakePaletteIOFunction(9);
MakePaletteIOFunction(10);
MakePaletteIOFunction(11);
MakePaletteIOFunction(12);
MakePaletteIOFunction(13);
MakePaletteIOFunction(14);
MakePaletteIOFunction(15);
MakeIO3FunctionRWX(AttributeModeControl, AttributeModeControlRegisterIndex, uint8_t);
MakeIO3FunctionRWX(OverscanColour, OverscanColourRegisterIndex, uint8_t);
MakeIO3FunctionRWX(ColourPlaneEnable, ColourPlaneEnableRegisterIndex, uint8_t);
MakeIO3FunctionRWX(HorizontalPixelPanning, HorizontalPixelPanningRegisterIndex, uint8_t);
MakeIO3FunctionRWX(ColourSelect, ColourSelectRegisterIndex, uint8_t);
void Write_PaletteAddressSource(bool value)
{
    Read_Byte(InputStatus1Colour);
    Write_Byte(AttributeControllerWriteIndexData, value ? 0x20 : 0x00);
}
bool Read_PaletteAddressSource()
{
    Read_Byte(InputStatus1Colour);
    return ((Read_Byte(AttributeControllerReadData) >> 5) & 0x1) == 0;
}

// CRTC registers
MakeIOFunctionSameName(HorizontalTotal, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(EndHorizontalDisplay, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(StartHorizontalBlanking, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(EndHorizontalBlanking, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(StartHorizontalRetrace, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(EndHorizontalRetrace, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(VerticalTotal, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(Overflow, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(PresetRowScan, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(MaximumScanLine, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(CursorStart, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(CursorEnd, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(StartAddressHigh, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(StartAddressLow, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(CursorLocationHigh, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(CursorLocationLow, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(VerticalRetraceStart, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(VerticalRetraceEnd, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(VerticalDisplayEnd, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(Offset, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(UnderlineLocation, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(StartVerticalBlanking, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(EndVerticalBlanking, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(CRTCModeControl, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)
MakeIOFunctionSameName(LineCompare, CRTControllerIndexColour, CRTControllerDataColour, RegisterIndex, uint8_t, Write_Byte, Read_Byte)

// Color registers
MakeIO1Function(DACMask, DACMask, DACMask, uint8_t, Byte);
MakeIO1Function(DACWriteAddress, DACIndexForWriteData, DACIndexForWriteData, uint8_t, Byte);
void Write_DACData(Color18Bit* value)
{
    Write_Byte(DACData, value->Red);
    Write_Byte(DACData, value->Green);
    Write_Byte(DACData, value->Blue);
}
void Read_DACData(Color18Bit* color)
{
    color->Red = Read_Byte(DACData);
    color->Green = Read_Byte(DACData);
    color->Blue = Read_Byte(DACData);
}

// Extended registers
MakeIO1Function(MiscellaneousOutput, MiscellaneousOutputRead, MiscellaneousOutputWrite, uint8_t, Byte);