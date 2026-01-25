#pragma once

#include <core/Defs.h>
#include <stdint.h>

#define ASM_INT2() __asm__ __volatile__("int $0x02" : : : "memory")

void ASMCALL i686_Panic();