#pragma once

/// summary: The identifier values for the different frame buffers.
/// remarks: Reading bits 2 and 3 of Miscellaneous Graphics will tell y which frame buffer is currently in use.
/// example: (FrameBufferSegments)((MyVGARegisters.MiscellaneousGraphics >> 2) &amp; 3)
typedef enum
{
    // summary: VGA buffer
    VGA = 1,
    
    // summary: Monchrome text buffer
    Monochrome = 2,
    
    // summary: CGA text buffer
    CGA = 3
} FrameBufferSegments;

/// summary: VGA screen modes : Either text mode or graphical (pixel-based) mode
typedef enum
{
    // summary: Text mode (Monochrome or CGA - see also <see cref="FrameBufferSegments"/>)
    Text,
    
    // summary: Graphical mode (VGA - see also <see cref="FrameBufferSegments"/>)
    Graphical
} ScreenMode;

/// summary: Flag bits within the Sequencer Memory Mode register
typedef enum SequencerMemoryModeFlags
{
    // summary: Odd/Even Host Memory Write Adressing Disable
    OddEvenDisable = 0x4,
    
    // summary: Low 2 bits of memory access determine which VGA map is selected. Odd/Even selector bit is ignored when this is 1.
    Chain4 = 0x8
} SequencerMemoryModeFlags;

/// summary: Flags bits within the Graphics Mode register
typedef enum
{
    // summary: Low bit of the Write Mode
    WriteModeL = 0x1,
    
    // summary: High bit of the Write Mode
    WriteModeH = 0x2,
    
    // summary: Read mode
    ReadMode = 0x8,
    
    // summary: Host Odd/Even Memory Read Addressing Enable - normally follows <see cref="SequencerMemoryModeFlags.OddEvenDisable"/>
    HostOddEvenMemoryReadAddressingEnable = 0x10,
    
    // summary: Interleaved shift mode: 2 bits popped for planes in order Plane 0, Plane 2, Plane 1, Plane 3
    InterleavedShift = 0x20,
    
    // summary: 256-Colour Shift : Used for linear 16-bit colour mode. 4 bits popped each time (usually merged to give 8 bit pixels). 2 pixels for Plane 0 then 2 for Plane 1 and so on.
    ColorShift256 = 0x40
} GraphicsModeFlags;