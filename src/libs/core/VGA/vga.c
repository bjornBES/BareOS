#include "vga.h"
#include "core/arch/i686/VGATextDevice.h"

#include "VGAEnumerables.h"
#include "VGARegisters.h"
#include "VGAConstants.h"

#define MODULE "VGA"

#ifndef DEBUGLEVELDEF
#define DEBUGLEVELDEF 1
typedef enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4
} DebugLevel;
#endif

void* malloc(size_t size);
void* calloc(size_t num, size_t size);

extern char VGAModesAddr;
vga_mode_t *vga_modes;
FrameBufferSegments FrameBufferSegment;
bool BlankedAndUnlocked;
uint32_t VGA_currentMode;

IVGAConfiguration *Configuration;

uint8_t *GetFrameBuffer()
{
    switch (FrameBufferSegment)
    {
    case VGA:
        return VGABlock;
    case Monochrome:
        return MonochromeTextBlock;
    case CGA:
        return CGATextBlock;
    default:
        break;
    }
    return NULL;
}

void BlankAndUnlock()
{
    Write_EndHorizontalBlanking(Read_EndHorizontalBlanking() | 0x80);
    Write_VerticalRetraceEnd(Read_VerticalRetraceEnd() & 0x7F);
    BlankedAndUnlocked = true;
}
void UnblankAndLock()
{
    Write_PaletteAddressSource(true);

    BlankedAndUnlocked = false;
}

void ConfigureForCurrentLockState(uint8_t *EndHorizontalBlankingValue, uint8_t *VerticalRetraceEndValue)
{
    if (BlankedAndUnlocked)
    {
        *EndHorizontalBlankingValue |= 0x80;
        *VerticalRetraceEndValue &= 0x7F;
    }
    else
    {
        *EndHorizontalBlankingValue &= 0x7F;
        *VerticalRetraceEndValue |= 0x80;
    }
}

void WriteSequencerRegisters(IVGAConfiguration *TheConfiguration)
{
    Write_Reset(TheConfiguration->Reset);
    Write_ClockingMode(TheConfiguration->ClockingMode);
    Write_MapMask(TheConfiguration->MapMask);
    Write_CharacterMapSelect(TheConfiguration->CharacterMapSelect);
    Write_SequencerMemoryMode(TheConfiguration->SequencerMemoryMode);
}

void WriteCRTCRegisters(IVGAConfiguration *TheConfiguration, uint8_t EndHorizontalBlankingValue, uint8_t VerticalRetraceEndValue)
{
    Write_HorizontalTotal(TheConfiguration->HorizontalTotal);
    Write_EndHorizontalDisplay(TheConfiguration->EndHorizontalDisplay);
    Write_StartHorizontalBlanking(TheConfiguration->StartHorizontalBlanking);
    Write_EndHorizontalBlanking(EndHorizontalBlankingValue); // Special value
    Write_StartHorizontalRetrace(TheConfiguration->StartHorizontalRetrace);
    Write_EndHorizontalRetrace(TheConfiguration->EndHorizontalRetrace);
    Write_VerticalTotal(TheConfiguration->VerticalTotal);
    Write_Overflow(TheConfiguration->Overflow);
    Write_PresetRowScan(TheConfiguration->PresetRowScan);
    Write_MaximumScanLine(TheConfiguration->MaximumScanLine);
    Write_CursorStart(TheConfiguration->CursorStart);
    Write_CursorEnd(TheConfiguration->CursorEnd);
    Write_StartAddressHigh(TheConfiguration->StartAddressHigh);
    Write_StartAddressLow(TheConfiguration->StartAddressLow);
    Write_CursorLocationHigh(TheConfiguration->CursorLocationHigh);
    Write_CursorLocationLow(TheConfiguration->CursorLocationLow);
    Write_VerticalRetraceStart(TheConfiguration->VerticalRetraceStart);
    Write_VerticalRetraceEnd(VerticalRetraceEndValue); // Special value
    Write_VerticalDisplayEnd(TheConfiguration->VerticalDisplayEnd);
    Write_Offset(TheConfiguration->Offset);
    Write_UnderlineLocation(TheConfiguration->UnderlineLocation);
    Write_StartVerticalBlanking(TheConfiguration->StartVerticalBlanking);
    Write_EndVerticalBlanking(TheConfiguration->EndVerticalBlanking);
    Write_CRTCModeControl(TheConfiguration->CRTCModeControl);
    Write_LineCompare(TheConfiguration->LineCompare);
}

void WriteGraphicsRegisters(IVGAConfiguration *TheConfiguration)
{
    Write_SetReset(TheConfiguration->SetReset);
    Write_EnableSetReset(TheConfiguration->EnableSetReset);
    Write_ColourCompare(TheConfiguration->ColorCompare);
    Write_DataRotate(TheConfiguration->DataRotate);
    Write_ReadMapSelect(TheConfiguration->ReadMapSelect);
    Write_GraphicsMode(TheConfiguration->GraphicsMode);
    Write_MiscellaneousGraphics(TheConfiguration->MiscellaneousGraphics);
    Write_ColourDontCare(TheConfiguration->ColorDontCare);
    Write_BitMask(TheConfiguration->BitMask);
}

void WriteAttributeRegisters(IVGAConfiguration *TheConfiguration)
{
    Write_Palette0(TheConfiguration->Palette0);
    Write_Palette1(TheConfiguration->Palette1);
    Write_Palette2(TheConfiguration->Palette2);
    Write_Palette3(TheConfiguration->Palette3);
    Write_Palette4(TheConfiguration->Palette4);
    Write_Palette5(TheConfiguration->Palette5);
    Write_Palette6(TheConfiguration->Palette6);
    Write_Palette7(TheConfiguration->Palette7);
    Write_Palette8(TheConfiguration->Palette8);
    Write_Palette9(TheConfiguration->Palette9);
    Write_Palette10(TheConfiguration->Palette10);
    Write_Palette11(TheConfiguration->Palette11);
    Write_Palette12(TheConfiguration->Palette12);
    Write_Palette13(TheConfiguration->Palette13);
    Write_Palette14(TheConfiguration->Palette14);
    Write_Palette15(TheConfiguration->Palette15);
    Write_AttributeModeControl(TheConfiguration->AttributeModeControl);
    Write_OverscanColour(TheConfiguration->OverscanColor);
    Write_ColourPlaneEnable(TheConfiguration->ColorPlaneEnable);
    Write_HorizontalPixelPanning(TheConfiguration->HorizontalPixelPanning);
    Write_ColourSelect(TheConfiguration->ColorSelect);
}

void VGA_LoadConfiguration(IVGAConfiguration *TheConfiguration)
{
    if (Configuration == TheConfiguration)
    {
        return;
    }

    Configuration = TheConfiguration;

    WriteSequencerRegisters(TheConfiguration);

    BlankAndUnlock();

    uint8_t EndHorizontalBlankingValue = TheConfiguration->EndHorizontalBlanking;
    uint8_t VerticalRetraceEndValue = TheConfiguration->VerticalRetraceEnd;
    ConfigureForCurrentLockState(&EndHorizontalBlankingValue, &VerticalRetraceEndValue);

    WriteCRTCRegisters(TheConfiguration, EndHorizontalBlankingValue, VerticalRetraceEndValue);

    WriteGraphicsRegisters(TheConfiguration);

    WriteAttributeRegisters(TheConfiguration);

    // Lock CRT registers and unblank the screen
    UnblankAndLock();

    if (TheConfiguration->Mode == Text)
    {
    }
}

/// <summary>
///     Sets the Color of a pixel on the screen.
/// </summary>
/// <remarks>
///     Assumes graphical mode is selected.
/// </remarks>
/// <param name="X">The X-coordinate (from left to right) as a 0-based index of the pixel to set.</param>
/// <param name="Y">The Y-coordinate (from top to bottom) as a 0-based index of the pixel to set.</param>
/// <param name="Color">The 24-bit Color to set the pixel to.</param>
void VGA_SetPixel(int X, int Y, Color24Bit *Color)
{
    Configuration->SetPixelMethod(Configuration, X, Y, Color);
}

/// <summary>
///     Gets the Color of a pixel on the screen.
/// </summary>
/// <remarks>
///     Assumes graphical mode is selected.
/// </remarks>
/// <param name="X">The X-coordinate (from left to right) as a 0-based index of the pixel to get.</param>
/// <param name="Y">The Y-coordinate (from top to bottom) as a 0-based index of the pixel to get.</param>
/// <returns>The 24-bit Color of the pixel.</returns>
Color24Bit *VGA_GetPixel(int X, int Y)
{
    return Configuration->GetPixelMethod(Configuration, X, Y);
}

/// <summary>
///     Sets a cell on the screen to the specified character and Color (/attribute).
/// </summary>
/// <remarks>
///     Assumes text mode is selected.
/// </remarks>
/// <param name="X">The X-coordinate (from left to right) as a 0-based index of the cell to set.</param>
/// <param name="Y">The Y-coordinate (from top to bottom) as a 0-based index of the cell to set.</param>
/// <param name="Character">The character to set the cell to.</param>
/// <param name="ForeColor">The foreground Color (/attribute) for the cell.</param>
/// <param name="BackColor">The background Color (/attribute) for the cell.</param>
void VGA_SetCell(int X, int Y, char Character, Color4Bit *ForeColor, Color4Bit *BackColor)
{
    Configuration->SetCellMethod(Configuration, X, Y, Character, ForeColor, BackColor);
}

/// <summary>
///     Gets the character and Color (/attribute) of a cell on the screen.
/// </summary>
/// <remarks>
///     Assumes text mode is selected.
/// </remarks>
/// <param name="X">The X-coordinate (from left to right) as a 0-based index of the cell to get.</param>
/// <param name="Y">The Y-coordinate (from top to bottom) as a 0-based index of the cell to get.</param>
/// <param name="ForeColor">The foreground Color (/attribute) of the cell.</param>
/// <param name="BackColor">The background Color (/attribute) of the cell.</param>
/// <returns>The character of the cell.</returns>
char VGA_GetCell(int X, int Y, Color4Bit *ForeColor, Color4Bit *BackColor)
{
    return Configuration->GetCellMethod(Configuration, X, Y, ForeColor, BackColor);
}

/// <summary>
///     The current colour palette.
/// </summary>
Color18Bit **ColorPalette;

/// <summary>
///     Sets the entire screen to the 0 index palette.
/// </summary>
void VGA_clrscr()
{
    Configuration->ClearMethod(Configuration);
}

/// <summary>
///     Looks up the specified index in the (cached) colour palette.
/// </summary>
/// <param name="Index">The index of the entry to look up.</param>
/// <returns>The entry value as an 18-bit colour.</returns>
Color18Bit* VGA_GetPaletteEntry(int Index)
{
    return ColorPalette[Index];
}

/// <summary>
///     Sets the colour palette to the specified colours.
/// </summary>
/// <param name="NewPallete">The colours to set the palette to.</param>
void VGA_SetPalette(Color18Bit** NewPallete, int size)
{
    VGA_SetPaletteOffset(0, NewPallete, size);
}

/// <summary>
///     Sets the colour palette starting from the specified offset to the specified colours.
/// </summary>
/// <param name="Offset">The offset to start setting at.</param>
/// <param name="NewPallete">The colours to set the palette to.</param>
void VGA_SetPaletteOffset(int Offset, Color18Bit** NewPallete, int size)
{
    Write_DACWriteAddress(0);
    // ReSharper disable once ForCanBeConvertedToForeach
    for (int i = 0; i < size; i++)
    {
        ColorPalette[i + Offset] = NewPallete[i];
        Write_DACData(NewPallete[i]);
    }
}

/*
void VGA_SetColor(int Index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (ColorPalette[Index] == NULL)
    {
        Color18Bit *color = malloc(sizeof(Color18Bit));
        color->Red = red;
        color->Green = green;
        color->Blue = blue;
        ColorPalette[Index] = color;
    }
    else
    {
        Color18Bit *color = ColorPalette[Index];
        color->Red = red;
        color->Green = green;
        color->Blue = blue;
    }
}
*/

bool VGA_GetColor(int Index, uint8_t* red, uint8_t* green, uint8_t* blue)
{
    if (ColorPalette[Index] == NULL)
    {
        return false;
    }
    else
    {
        Color18Bit *color = ColorPalette[Index];
        red = &color->Red;
        green = &color->Green;
        blue = &color->Blue;
    }
    return true;
}

/// <summary>
///     Sets a specific entry in the colour palette.
/// </summary>
/// <seealso cref="SetPalette(int, Kernel.VGA.Colour18Bit[])"/>
/// <param name="Index">The idnex of the entry to set.</param>
/// <param name="Colour">The colour to set the entry to.</param>
void VGA_SetPaletteEntry(int Index, Color18Bit *Color)
{
    ColorPalette[Index] = Color;

    Write_DACWriteAddress((uint8_t)Index);
    Write_DACData(Color);
}

/*
uint8_t GetVgaMode(vesa_mode_info_t *vesaMode)
{

    if ((vesaMode->attributes & 0x10) == 0x10)
    {
        return Graphical;
    }
    else
    {
        return Text;
    }
    return 0xFF;
}
void SwitchToVGAModes(VESAInfo *VESAinfo)
{
    vga_modes = (vga_mode_t *)&VGAModesAddr;
    int size = 0;
    for (size_t i = 0; i < VESAinfo->VESACount; i++)
    {
        vesa_mode_info_t *entry = &VESAinfo->entries[i];
        
        vga_modes[i].mode = GetVgaMode(entry);
        vga_modes[i].width = entry->width;
        vga_modes[i].height = entry->height;
        vga_modes[i].bpp = entry->bpp;
        vga_modes[i].pitch = entry->pitch;
        vga_modes[i].framebuffer = entry->framebuffer;
        vga_modes[i].memory_model = entry->memory_model;
        vga_modes[i].planes = entry->planes;
        vga_modes[i].red_mask = entry->red_mask;
        vga_modes[i].red_position = entry->red_position;
        vga_modes[i].green_mask = entry->green_mask;
        vga_modes[i].green_position = entry->green_position;
        vga_modes[i].blue_mask = entry->blue_mask;
        vga_modes[i].blue_position = entry->blue_position;
        vga_modes[i].Colored = (entry->attributes & 0x08) ? true : false;
        logf(MODULE, LVL_DEBUG, "%2u: 0x%08X %s, %s, mm: %u %ux%ux%u",
        i,
        entry->framebuffer,
        (vga_modes[i].mode == Text) ? "GRAP" : "TEXT",
        (vga_modes[i].Colored == 1) ? "COLOR" : "MONO ",
        vga_modes[i].memory_model,
        entry->width,
        entry->height,
        entry->bpp);
        
        size += sizeof(vga_mode_t);
    }
}
*/

void VGA_CursorScanLine(int _start, int _end)
{
    uint8_t end = Read_CursorEnd();
    uint8_t start = Read_CursorStart();
    end = (end & 0xE0) | (uint8_t)_end;
    start = (start & 0xE0) | (uint8_t)_start;
    Write_CursorEnd(end);
    Write_CursorStart(start);
}

void VGA_SetMode(uint16_t mode)
{
}

void VGAConfigurationStart();
void VGA_initialize(/*BootParams *bootParams, VESAInfo *VESAinfo*/)
{
    VGAConfigurationStart();
    // logf(MODULE, LVL_DEBUG, "VGA_initialize");
    uint8_t data = Read_MiscellaneousGraphics();
    // logf(MODULE, LVL_DEBUG, "Read_MiscellaneousGraphics %u", data);
    FrameBufferSegment = (FrameBufferSegments)((data >> 2) & 3);
    // logf(MODULE, LVL_DEBUG, "FrameBufferSegment %u", FrameBufferSegment);
    ColorPalette = (Color18Bit **)calloc(256, sizeof(Color18Bit));


    // SwitchToVGAModes(VESAinfo);
}
