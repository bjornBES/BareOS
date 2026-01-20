#include "IVGAConfiguration.h"
#include "vga.h"

extern char VGAConfigurations;
IVGAConfiguration* T_80x25;

void SetCell4bpp(IVGAConfiguration* config, int X, int Y, char c, Color4Bit *ForeColor, Color4Bit *BackColor)
{
    int offset = 2 * (Y * config->Width + X);
    uint8_t* frameBuffer = FrameBuffer;
    frameBuffer[offset] = c;
    frameBuffer[offset + 1] = (BackColor->Value << 4 | ForeColor->Value);
}

char GetCell4bpp(IVGAConfiguration* config, int X, int Y, Color4Bit *ForeColor, Color4Bit *BackColor)
{
    int offset = 2 * (Y * config->Width + X);
    uint8_t* frameBuffer = FrameBuffer;
    char c = (char)frameBuffer[offset];
    uint8_t color = frameBuffer[offset + 1];
    // (BackColor->Value << 4 | ForeColor->Value)
    BackColor->Value = (color >> 4) & 0x0F;
    ForeColor->Value = color & 0x0F;
    return c;
}

void ClearScreen(IVGAConfiguration* config)
{
    Color4Bit color;
    color.Value = 0;
    for (int y = 0; y < config->Height; y++)
    {
        for (int x = 0; x < config->Width; x++)
        {
            SetCell4bpp(config, x, y, '\0', &color, &color);
        }
    }
}

void NotSupportedFunction(IVGAConfiguration* config, ...)
{

}

void Init80x25(void* address)
{
    T_80x25 = (IVGAConfiguration*)address;
    T_80x25->SetCellMethod = (void (*)(struct t_IVGAConfiguration* config, int X, int Y, char c, Color4Bit *ForeColor, Color4Bit *BackColor))SetCell4bpp;
    T_80x25->GetCellMethod = (char (*)(struct t_IVGAConfiguration* config, int X, int Y, Color4Bit *ForeColor, Color4Bit *BackColor))GetCell4bpp;
    T_80x25->SetPixelMethod = (void (*)(struct t_IVGAConfiguration* config, int X, int Y, Color24Bit *color))NotSupportedFunction;
    T_80x25->GetPixelMethod = (Color24Bit *(*)(struct t_IVGAConfiguration* config, int X, int Y))NotSupportedFunction;
    T_80x25->ClearMethod = (void (*)(struct t_IVGAConfiguration* config))ClearScreen;
    T_80x25->Width = 80;
    T_80x25->Height = 25;
    T_80x25->BitDepth = 4;
    T_80x25->Mode = Text;
    T_80x25->SetReset = 0x00;
    T_80x25->EnableSetReset = 0x00;
    T_80x25->ColorCompare = 0x00;
    T_80x25->DataRotate = 0x00;
    T_80x25->ReadMapSelect = 0x00;
    T_80x25->GraphicsMode = HostOddEvenMemoryReadAddressingEnable;
    T_80x25->MiscellaneousGraphics = 0x0E;
    T_80x25->ColorDontCare = 0x00;
    T_80x25->BitMask = 0xFF;
    T_80x25->Reset = 0x03;
    T_80x25->ClockingMode = 0x00;
    T_80x25->MapMask = 0x03;
    T_80x25->CharacterMapSelect = 0x00;
    T_80x25->SequencerMemoryMode = (SequencerMemoryModeFlags)0x02; // Gray coding applied so that a smooth progression up the colours can be made
    T_80x25->Palette0 = 0x00;
    T_80x25->Palette1 = 0x01;
    T_80x25->Palette2 = 0x02;
    T_80x25->Palette3 = 0x03;
    T_80x25->Palette4 = 0x04;
    T_80x25->Palette5 = 0x05;
    T_80x25->Palette6 = 0x06;
    T_80x25->Palette7 = 0x07;
    T_80x25->Palette8 = 0x08;
    T_80x25->Palette9 = 0x09;
    T_80x25->Palette10 = 0x0A;
    T_80x25->Palette11 = 0x0B;
    T_80x25->Palette12 = 0x0C;
    T_80x25->Palette13 = 0x0D;
    T_80x25->Palette14 = 0x0E;
    T_80x25->Palette15 = 0x0F;
    T_80x25->AttributeModeControl = 0x0C;
    T_80x25->OverscanColor = 0x00;
    T_80x25->ColorPlaneEnable = 0x0F;
    T_80x25->HorizontalPixelPanning = 0x08;
    T_80x25->ColorSelect = 0x00;
    T_80x25->HorizontalTotal = 0x5F;
    T_80x25->EndHorizontalDisplay = 0x4F;
    T_80x25->StartHorizontalBlanking = 0x50;
    T_80x25->EndHorizontalBlanking = 0x82;
    T_80x25->StartHorizontalRetrace = 0x55;
    T_80x25->EndHorizontalRetrace = 0x81;
    T_80x25->VerticalTotal = 0xBF;
    T_80x25->Overflow = 0x1F;
    T_80x25->PresetRowScan = 0x00;
    T_80x25->MaximumScanLine = 0x4F;
    T_80x25->CursorStart = 0x0D;
    T_80x25->CursorEnd = 0x0E;
    T_80x25->StartAddressHigh = 0x00;
    T_80x25->StartAddressLow = 0x00;
    T_80x25->CursorLocationHigh = 0x00;
    T_80x25->CursorLocationLow = 0x50;
    T_80x25->VerticalRetraceStart = 0x9C;
    T_80x25->VerticalRetraceEnd = 0x8E;
    T_80x25->VerticalDisplayEnd = 0x8F;
    T_80x25->Offset = 0x28;
    T_80x25->UnderlineLocation = 0x1F;
    T_80x25->StartVerticalBlanking = 0x96;
    T_80x25->EndVerticalBlanking = 0xB9;
    T_80x25->CRTCModeControl = 0xA3;
    T_80x25->LineCompare = 0xFF;
    T_80x25->MiscellaneousOutput = 0x67;
}

void VGAConfigurationStart()
{
    void* address = (void*)&VGAConfigurations;
    Init80x25(address);
    address += sizeof(IVGAConfiguration);
}
