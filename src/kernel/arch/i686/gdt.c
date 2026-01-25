#include "gdt.h"
#include <stdio.h>

// helper functions
#define GDT_LIMIT_LOW(limit) (limit & 0xFFFF)
#define GDT_BASE_LOW(base) (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base) ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags) (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base) ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_FLAGS_LIMIT_HI(limit, flags),           \
    GDT_BASE_HIGH(base)}

void ASMCALL GDT_Load(GDTDescriptor *descriptor, uint16_t codeSegment, uint16_t dataSegment);

GDTEntry _GDT[NUM_DESCRIOTOR];
GDTDescriptor _GDTDescriptor = {sizeof(_GDT) - 1, _GDT};

void i686_GDTSetEntry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    _GDT[index] = (GDTEntry)GDT_ENTRY(base, limit, access, flags);
}

void i686_GDTLoad()
{
    GDT_Load(&_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
}

void i686_GDTInitialize()
{
    i686_GDTSetEntry(0, 0, 0, 0, 0);
    i686_GDTSetEntry(1, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    i686_GDTSetEntry(2, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
}

// debug function
#ifdef DEBUG

void i686_GDT_DumpSelector(uint16_t sel)
{
    GDTDescriptor gdtr;
    __asm__ ("sgdt %0" : "=m"(gdtr));
    uint8_t *gdt = (uint8_t*)gdtr.Ptr;
    uint16_t index = sel >> 3;
    fprintf(stddebug,"selector 0x%04X index %u RPL %u", sel, index, sel & 3);
    // Print raw descriptor bytes
    uint8_t *desc = gdt + index*8;
    fprintf(stddebug, "gdt[%u] = %02x %02x %02x %02x %02x %02x %02x %02x",
           index, desc[0],desc[1],desc[2],desc[3],desc[4],desc[5],desc[6],desc[7]);
}

#endif