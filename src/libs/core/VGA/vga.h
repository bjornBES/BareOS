#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "IVGAConfiguration.h"
#include "Colors.h"

#define FrameBuffer GetFrameBuffer()

enum RegisterSelect_t
{
    Graphics_Registers,
    Sequencer_Registers,
    Attribute_Controller,
    CRT_Controller_Registers,
    Color_Registers,
    External_Registers,
} typedef RegisterSelect;

enum ExternalRegisterSelect_t
{
    ER_OutputRegister,
    ER_FeatureControlRegister,
    ER_InputStatus0Regiser,
    ER_InputStatus1Regiser
} typedef ExternalRegisterSelect;

enum GraphicsRegisters
{

    GR_Set_Reset_Register,
    GR_Enable_Set_Reset_Register,
    GR_Color_Compare_Register,
    GR_Data_Rotate_Register,
    GR_Read_Map_Select_Register,
    GR_Graphics_Mode_Register,
    GR_Miscellaneous_Graphics_Register,
    GR_Color_Don_Care_Register,
    GR_Bit_Mask_Register
};

enum CRTController
{
    CRTC_Horizontal_Total_Register              = 0x0,
    CRTC_End_Horizontal_Display_Register        = 0x1,
    CRTC_Start_Horizontal_Blanking_Register     = 0x2,
    CRTC_End_Horizontal_Blanking_Register       = 0x3,
    CRTC_Start_Horizontal_Retrace_Register      = 0x4,
    CRTC_End_Horizontal_Retrace_Register        = 0x5,
    CRTC_Vertical_Total_Register                = 0x6,
    CRTC_Overflow_Register                      = 0x7,
    CRTC_Preset_Row_Scan_Register               = 0x8,
    CRTC_Maximum_Scan_Line_Register             = 0x9,
    CRTC_Cursor_Start_Register                  = 0xA,
    CRTC_Cursor_End_Register                    = 0xB,
    CRTC_Start_Address_High_Register            = 0xC,
    CRTC_Start_Address_Low_Register             = 0xD,
    CRTC_Cursor_Location_High_Register          = 0xE,
    CRTC_Cursor_Location_Low_Register           = 0xF,
    CRTC_Vertical_Retrace_Start_Register        = 0x10,
    CRTC_Vertical_Retrace_End_Register          = 0x11,
    CRTC_Vertical_Display_End_Register          = 0x12,
    CRTC_Offset_Register                        = 0x13,
    CRTC_Underline_Location_Register            = 0x14,
    CRTC_Start_Vertical_Blanking_Register       = 0x15,
    CRTC_End_Vertical_Blanking                  = 0x16,
    CRTC_Mode_Control_Register                  = 0x17,
    CRTC_Line_Compare_Register                  = 0x18
};

enum ModeBPP_t {
    Unknown,
    TEXT_1BPP,
    GRAP_1BPP = 1,
    GRAP_4BPP = 4,
    GRAP_8BPP = 8,
    GRAP_15BPP = 15,
    GRAP_16BPP = 16,
    GRAP_24BPP = 24,
    GRAP_32BPP = 32,
} typedef ModeBPP;

/* VGA Mode Information Structure */
typedef struct {
    uint8_t mode;                   // VGA Mode Type (Text or Graphics)
    uint16_t width;                 // Resolution width in pixels
    uint16_t height;                // Resolution height in pixels
    uint8_t bpp;                    // Bits per pixel
    uint16_t pitch;                 // Bytes per scanline (pitch)
    uint32_t framebuffer;           // Physical address of the linear framebuffer
    uint8_t memory_model;           // Memory model (e.g., Direct Color)
    uint8_t planes;                 // Number of planes
    uint8_t red_mask;               // Red color mask (for direct color modes)
    uint8_t red_position;           // Red color position
    uint8_t green_mask;             // Green color mask
    uint8_t green_position;         // Green color position
    uint8_t blue_mask;              // Blue color mask
    uint8_t blue_position;          // Blue color position
    bool Colored;                   // Indicates if the mode is colored
} vga_mode_t;

#define UINT8TOFORECOLOR4BIT(value) \
    (Color4Bit)((uint8_t)value & 0x0F)

#define UINT8TOBACKCOLOR4BIT(value) \
    (Color4Bit)(((uint8_t)value >> 4) & 0x0F)

extern IVGAConfiguration *Configuration;
extern vga_mode_t *vga_modes;
extern uint32_t VGA_currentMode;

void VGA_CursorScanLine(int start, int end);
void VGA_SetPalette(Color18Bit** NewPallete, int size);
void VGA_SetPaletteOffset(int Offset, Color18Bit** NewPallete, int size);
void VGA_SetPaletteEntry(int Index, Color18Bit *Color);
void VGA_LoadConfiguration(IVGAConfiguration *TheConfiguration);
void VGA_initialize(/*BootParams* bootParams, VESAInfo* VESAinfo*/);
void VGA_SetMode(uint16_t mode);
void VGA_SetColor(int Index, uint8_t red, uint8_t green, uint8_t blue);
bool VGA_GetColor(int Index, uint8_t* red, uint8_t* green, uint8_t* blue);
Color18Bit* VGA_GetPaletteEntry(int Index);
uint8_t *GetFrameBuffer();

void VGA_clrscr();
char VGA_GetCell(int X, int Y, Color4Bit *ForeColor, Color4Bit *BackColor);
void VGA_SetCell(int X, int Y, char Character, Color4Bit *ForeColor, Color4Bit *BackColor);
Color24Bit *VGA_GetPixel(int X, int Y);
void VGA_SetPixel(int X, int Y, Color24Bit *Color);