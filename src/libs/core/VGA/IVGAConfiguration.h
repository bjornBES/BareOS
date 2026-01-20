#pragma once

#include <stdint.h>
#include "VGAEnumerables.h"
#include "Colors.h"

typedef struct t_IVGAConfiguration
{
    /// <summary>
    ///     Width of the screen in pixels.
    /// </summary>
    uint32_t Width;

    /// <summary>
    ///     Height of the screen in pixels.
    /// </summary>
    uint32_t Height;

    /// <summary>
    ///     Color depth of the palette in bits.
    /// </summary>
    uint32_t BitDepth;

    /// <summary>
    ///     Whether the configuration is a text or graphical mode configuration.
    /// </summary>
    ScreenMode Mode;

    /// <summary>
    ///     Method for setting a cell on the screen.
    /// </summary>
    /// <remarks>
    ///     This can still be valid for a graphical mode configuration if it supports
    ///     graphical text output.
    /// </remarks>
    void (*SetCellMethod)(struct t_IVGAConfiguration* config, int X, int Y, char c, Color4Bit *ForeColor, Color4Bit *BackColor);
    /// <summary>
    ///     Method for getting a cell from the screen.
    /// </summary>
    /// <remarks>
    ///     This can still be valid for a graphical mode configuration if it supports
    ///     graphical text output.
    /// </remarks>
    char (*GetCellMethod)(struct t_IVGAConfiguration* config, int X, int Y, Color4Bit *ForeColor, Color4Bit *BackColor);

    /// <summary>
    ///     Method for setting a pixel on the screen.
    /// </summary>
    /// <remarks>
    ///     Not valid for text mode configurations. Should return null if the configuration
    ///     is a text mode configuration.
    /// </remarks>
    void (*SetPixelMethod)(struct t_IVGAConfiguration* config, int X, int Y, Color24Bit *color);
    /// <summary>
    ///     Method for getting a pixel from the screen.
    /// </summary>
    /// <remarks>
    ///     Not valid for text mode configurations. Should return null if the configuration
    ///     is a text mode configuration.
    /// </remarks>
    Color24Bit *(*GetPixelMethod)(struct t_IVGAConfiguration* config, int X, int Y);

    /// <summary>
    ///     Method for setting all the pixels on the screen.
    /// </summary>
    /// <remarks>
    ///     Not valid for text mode configurations. Should return null if the configuration
    ///     is a text mode configuration.
    /// </remarks>
    void (*ClearMethod)(struct t_IVGAConfiguration* config);

    // Registers

    // Graphics registers

    /// <summary>
    ///    Value for the Set Reset register
    /// </summary>
    uint8_t SetReset;

    /// <summary>
    ///    Value for the Enable Set Reset register
    /// </summary>
    uint8_t EnableSetReset;

    /// <summary>
    ///    Value for the Color Compare register
    /// </summary>
    uint8_t ColorCompare;

    /// <summary>
    ///    Value for the Data Rotate register
    /// </summary>
    uint8_t DataRotate;

    /// <summary>
    ///    Value for the Read Map Select register
    /// </summary>
    uint8_t ReadMapSelect;

    /// <summary>
    ///    Value for the Graphics Mode register
    /// </summary>
    GraphicsModeFlags GraphicsMode;

    /// <summary>
    ///    Value for the Miscellaneous Graphics register
    /// </summary>
    uint8_t MiscellaneousGraphics;

    /// <summary>
    ///    Value for the Color Dont Care register
    /// </summary>
    uint8_t ColorDontCare;

    /// <summary>
    ///    Value for the Bit Mask register
    /// </summary>
    uint8_t BitMask;

    // Sequencer registers

    /// <summary>
    ///    Value for the Reset register
    /// </summary>
    uint8_t Reset;

    /// <summary>
    ///    Value for the Clocking Mode register
    /// </summary>
    uint8_t ClockingMode;

    /// <summary>
    ///    Value for the Map Mask register
    /// </summary>
    uint8_t MapMask;

    /// <summary>
    ///    Value for the Character Map Select register
    /// </summary>
    uint8_t CharacterMapSelect;

    /// <summary>
    ///    Value for the Sequencer Memory Mode register
    /// </summary>
    SequencerMemoryModeFlags SequencerMemoryMode;

    // Attribute registers

    /// <summary>
    ///    Value for the Palette 0 register
    /// </summary>
    uint8_t Palette0;

    /// <summary>
    ///    Value for the Palette 1 register
    /// </summary>
    uint8_t Palette1;

    /// <summary>
    ///    Value for the Palette 2 register
    /// </summary>
    uint8_t Palette2;

    /// <summary>
    ///    Value for the Palette 3 register
    /// </summary>
    uint8_t Palette3;

    /// <summary>
    ///    Value for the Palette 4 register
    /// </summary>
    uint8_t Palette4;

    /// <summary>
    ///    Value for the Palette 5 register
    /// </summary>
    uint8_t Palette5;

    /// <summary>
    ///    Value for the Palette 6 register
    /// </summary>
    uint8_t Palette6;

    /// <summary>
    ///    Value for the Palette 7 register
    /// </summary>
    uint8_t Palette7;

    /// <summary>
    ///    Value for the Palette 8 register
    /// </summary>
    uint8_t Palette8;

    /// <summary>
    ///    Value for the Palette 9 register
    /// </summary>
    uint8_t Palette9;

    /// <summary>
    ///    Value for the Palette 10 register
    /// </summary>
    uint8_t Palette10;

    /// <summary>
    ///    Value for the Palette 11 register
    /// </summary>
    uint8_t Palette11;

    /// <summary>
    ///    Value for the Palette 12 register
    /// </summary>
    uint8_t Palette12;

    /// <summary>
    ///    Value for the Palette 13 register
    /// </summary>
    uint8_t Palette13;

    /// <summary>
    ///    Value for the Palette 14 register
    /// </summary>
    uint8_t Palette14;

    /// <summary>
    ///    Value for the Palette 15 register
    /// </summary>
    uint8_t Palette15;

    /// <summary>
    ///    Value for the Attribute Mode Control register
    /// </summary>
    uint8_t AttributeModeControl;

    /// <summary>
    ///    Value for the Overscan Color register
    /// </summary>
    uint8_t OverscanColor;

    /// <summary>
    ///    Value for the Color Plane Enable register
    /// </summary>
    uint8_t ColorPlaneEnable;

    /// <summary>
    ///    Value for the Horizontal Pixel Panning register
    /// </summary>
    uint8_t HorizontalPixelPanning;

    /// <summary>
    ///    Value for the Color Select register
    /// </summary>
    uint8_t ColorSelect;

    // CRTC registers

    /// <summary>
    ///    Value for the Horizontal Total register
    /// </summary>
    uint8_t HorizontalTotal;

    /// <summary>
    ///    Value for the End Horizontal Display register
    /// </summary>
    uint8_t EndHorizontalDisplay;

    /// <summary>
    ///    Value for the Start Horizontal Blanking register
    /// </summary>
    uint8_t StartHorizontalBlanking;

    /// <summary>
    ///    Value for the End Horizontal Blanking register
    /// </summary>
    uint8_t EndHorizontalBlanking;

    /// <summary>
    ///    Value for the Start Horizontal Retrace register
    /// </summary>
    uint8_t StartHorizontalRetrace;

    /// <summary>
    ///    Value for the End Horizontal Retrace register
    /// </summary>
    uint8_t EndHorizontalRetrace;

    /// <summary>
    ///    Value for the Vertical Total register
    /// </summary>
    uint8_t VerticalTotal;

    /// <summary>
    ///    Value for the Overflow register
    /// </summary>
    uint8_t Overflow;

    /// <summary>
    ///    Value for the Preset Row Scan register
    /// </summary>
    uint8_t PresetRowScan;

    /// <summary>
    ///    Value for the Maximum Scan Line register
    /// </summary>
    uint8_t MaximumScanLine;

    /// <summary>
    ///    Value for the Cursor Start register
    /// </summary>
    uint8_t CursorStart;

    /// <summary>
    ///    Value for the Cursor End register
    /// </summary>
    uint8_t CursorEnd;

    /// <summary>
    ///    Value for the Start Address High register
    /// </summary>
    uint8_t StartAddressHigh;

    /// <summary>
    ///    Value for the Start Address Low register
    /// </summary>
    uint8_t StartAddressLow;

    /// <summary>
    ///    Value for the Cursor Location High register
    /// </summary>
    uint8_t CursorLocationHigh;

    /// <summary>
    ///    Value for the Cursor Location Low register
    /// </summary>
    uint8_t CursorLocationLow;

    /// <summary>
    ///    Value for the Vertical Retrace Start register
    /// </summary>
    uint8_t VerticalRetraceStart;

    /// <summary>
    ///    Value for the Vertical Retrace End register
    /// </summary>
    uint8_t VerticalRetraceEnd;

    /// <summary>
    ///    Value for the Vertical Display End register
    /// </summary>
    uint8_t VerticalDisplayEnd;

    /// <summary>
    ///    Value for the Offset register
    /// </summary>
    uint8_t Offset;

    /// <summary>
    ///    Value for the Underline Location register
    /// </summary>
    uint8_t UnderlineLocation;

    /// <summary>
    ///    Value for the Start Vertical Blanking register
    /// </summary>
    uint8_t StartVerticalBlanking;

    /// <summary>
    ///    Value for the End Vertical Blanking register
    /// </summary>
    uint8_t EndVerticalBlanking;

    /// <summary>
    ///    Value for the CRTC Mode Control register
    /// </summary>
    uint8_t CRTCModeControl;

    /// <summary>
    ///    Value for the Line Compare register
    /// </summary>
    uint8_t LineCompare;

    // Extended registers

    /// <summary>
    ///    Value for the Miscellaneous Output register
    /// </summary>
    uint8_t MiscellaneousOutput;
} IVGAConfiguration;

extern IVGAConfiguration* T_80x25;
