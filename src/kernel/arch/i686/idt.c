#include "idt.h"
#include <util/binary.h>
#include <stdio.h>

void __attribute__((cdecl)) IDT_Load(IDTDescriptor* idtDescriptor);

IDTEntry _IDT[256];
IDTDescriptor _IDTDescriptor = { sizeof(_IDT) - 1, _IDT };

void i686_IDTSetGate(int interrupt, void* base, uint32_t segmentDescriptor, uint8_t flags)
{
    _IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    _IDT[interrupt].SegmentSelector = segmentDescriptor;
    _IDT[interrupt].Reserved = 0;
    _IDT[interrupt].Flags = flags;
    _IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

void i686_IDTDisableGate(int interrupt)
{
    FLAG_UNSET(_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDTEnableGate(int interrupt)
{
    FLAG_SET(_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}


void i686_IDTLoad()
{
    IDT_Load(&_IDTDescriptor);
}

void i686_IDTInitialize()
{

}

// debug function
#ifdef DEBUG

void i686_IDT_DumpSelector(uint16_t sel)
{
IDTDescriptor idtr;
    __asm__ ("sidt %0" : "=m"(idtr));
    IDTEntry* idt = (IDTEntry*)(idtr.Ptr);
    IDTEntry e = idt[n];
    uint32_t offset = ((uint32_t)e.BaseHigh << 16) | e.BaseLow;
    fprintf(stddebug, "IDT[%02x]: offset=0x%08X selector=0x%04X type_attr=0x%02X reserved=0x%02X",
           n, offset, e.SegmentSelector, e.Flags, e.Reserved);
}

#endif