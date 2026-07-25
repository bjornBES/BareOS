/*
 * File: regs.c
 * File Created: 20 Jan 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 23 Mar 2026
 * Modified By: BjornBEs
 * -----
 */


#include "bios.h"
#include "memory.h"
#include "string.h"

void initregs(biosregs *reg)
{
	memset(reg, 0, sizeof(*reg));
	reg->eflags |= 1;
	reg->ds = get_ds();
	reg->es = get_ds();
	reg->fs = get_fs();
	reg->gs = get_gs();
}