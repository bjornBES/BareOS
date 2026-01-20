#pragma once
#include <stdint.h>

// Registers

// Graphics registers

// Index of the Set/Reset register
#define SetResetRegisterIndex 0x00

// Index of the Enable Set/Reset register
#define EnableSetResetRegisterIndex 0x01

// Index of the Colour Compare register
#define ColourCompareRegisterIndex 0x02

// Index of the Data Rotate register
#define DataRotateRegisterIndex 0x03

// Index of the Read Map Select register
#define ReadMapSelectRegisterIndex 0x04

// Index of the Graphics Mode register
#define GraphicsModeRegisterIndex 0x05

// Index of the Miscellaneous Graphics register
#define MiscellaneousGraphicsRegisterIndex 0x06

// Index of the Colour Don't Care register
#define ColourDontCareRegisterIndex 0x07

// Index of the Bit Mask register
#define BitMaskRegisterIndex 0x08

// Sequencer registers

// Index of the Reset register
#define ResetRegisterIndex 0x00

// Index of the Clocking Mode register
#define ClockingModeRegisterIndex 0x01

// Index of the Map Mask register
#define MapMaskRegisterIndex 0x02

// Index of the Character Map Select register
#define CharacterMapSelectRegisterIndex 0x03

// Index of the Sequencer Memory Mode register
#define SequencerMemoryModeRegisterIndex 0x04

// Attribute registers

// Index of palette register 0
#define Palette0RegisterIndex 0x0

// Index of palette register 1
#define Palette1RegisterIndex 0x1

// Index of palette register 2
#define Palette2RegisterIndex 0x2

// Index of palette register 3
#define Palette3RegisterIndex 0x3

// Index of palette register 4
#define Palette4RegisterIndex 0x4

// Index of palette register 5
#define Palette5RegisterIndex 0x5

// Index of palette register 6
#define Palette6RegisterIndex 0x6

// Index of palette register 7
#define Palette7RegisterIndex 0x7

// Index of palette register 8
#define Palette8RegisterIndex 0x8

// Index of palette register 9
#define Palette9RegisterIndex 0x9

// Index of palette register 10
#define Palette10RegisterIndex 0xA

// Index of palette register 11
#define Palette11RegisterIndex 0xB

// Index of palette register 12
#define Palette12RegisterIndex 0xC

// Index of palette register 13
#define Palette13RegisterIndex 0xD

// Index of palette register 14
#define Palette14RegisterIndex 0xE

// Index of palette register 15
#define Palette15RegisterIndex 0xF

// Index of the Attribute Mode Control register
#define AttributeModeControlRegisterIndex 0x10

// Index of the Overscan Colour register
#define OverscanColourRegisterIndex 0x11

// Index of the Colour Plane Enable register
#define ColourPlaneEnableRegisterIndex 0x12

// Index of the Horizontal Pixel Panning register
#define HorizontalPixelPanningRegisterIndex 0x13

// Index of the Colour Select register
#define ColourSelectRegisterIndex 0x14

// CRTC registers

// Index of the Horizontal Total register
#define HorizontalTotalRegisterIndex 0x00

// Index of the End Horizontal Display register
#define EndHorizontalDisplayRegisterIndex 0x01

// Index of the Start Horizontal Blanking register
#define StartHorizontalBlankingRegisterIndex 0x02

// Index of the End Horizontal Blanking register
#define EndHorizontalBlankingRegisterIndex 0x03

// Index of the Start Horizontal Retrace register
#define StartHorizontalRetraceRegisterIndex 0x04

// Index of the End Horizontal Retrace register
#define EndHorizontalRetraceRegisterIndex 0x05

// Index of the Vertical Total register
#define VerticalTotalRegisterIndex 0x06

// Index of the Overflow register
#define OverflowRegisterIndex 0x07

// Index of the Preset Row Scan register
#define PresetRowScanRegisterIndex 0x08

// Index of the Maximum Scan Line register
#define MaximumScanLineRegisterIndex 0x09

// Index of the Cursor Start register
#define CursorStartRegisterIndex 0x0A

// Index of the Cursor End register
#define CursorEndRegisterIndex 0x0B

// Index of the Start Address High register
#define StartAddressHighRegisterIndex 0x0C

// Index of the Start Address Low register
#define StartAddressLowRegisterIndex 0x0D

// Index of the Cursor Location High register
#define CursorLocationHighRegisterIndex 0x0E

// Index of the Cursor Location Low register
#define CursorLocationLowRegisterIndex 0x0F

// Index of the Vertical Retrace Start register
#define VerticalRetraceStartRegisterIndex 0x10

// Index of the Vertical Retrace End register
#define VerticalRetraceEndRegisterIndex 0x11

// Index of the Vertical Display End register
#define VerticalDisplayEndRegisterIndex 0x12

// Index of the Offset register
#define OffsetRegisterIndex 0x13

// Index of the Underline Location register
#define UnderlineLocationRegisterIndex 0x14

// Index of the Start Vertical Blanking register
#define StartVerticalBlankingRegisterIndex 0x15

// Index of the End Vertical Blanking
#define EndVerticalBlankingRegisterIndex 0x16

// Index of the CRTC Mode Control register
#define CRTCModeControlRegisterIndex 0x17

// Index of the Line Compare register
#define LineCompareRegisterIndex 0x18

// Masks

// Mask for the Enable Set/Reset bits in the Enable Set/Reset register
#define EnableSetResetMask 0xF

// Mask for the Set/Reset Value bits in the Set/Reset register
#define SetResetValueMask 0xF

// Mask for the Rotate Count bits in the Data Rotate register
#define RotateCountMask 0x7

// Mask for the Logical Operation bits in the Data Rotate register
#define LogicalOperationMask 0x18

// Memory

// 0xA0000
#define VGABlock (uint8_t*)0xA0000

// 64KiB
#define VGABlockSize 1024*64

// 0xB0000
#define MonochromeTextBlock (uint8_t*)0xB0000;

// 32KiB
#define MonochromeTextBlockSize 1024*32
        
// 0xB8000
#define CGATextBlock (uint8_t*)0xB8000;

// 32KiB
#define CGATextBlockSize 1024 * 32
