#include "e9.h"

#include <IO.h>
#include <core/Defs.h>

#include <stdint.h>

void e9_putc(char c)
{
    Outb(0xE9, c);
}