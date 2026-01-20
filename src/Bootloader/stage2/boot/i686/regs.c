
#include <core/arch/i686/bios.h>
#include "memory.h"
#include "string.h"

void initregs(biosregs *reg)
{
	memset(reg, 0, sizeof(*reg));
	reg->eflags |= 1;
	reg->ds = ds();
	reg->es = ds();
	reg->fs = fs();
	reg->gs = gs();
}