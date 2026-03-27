/*
 * File: bios.c
 * File Created: 23 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 24 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#include "bios.h"

uint16_t get_ds(void)
{
	uint16_t seg;
	__asm__ ("mov %0,ds" : "=r" (seg));
	return seg;
}

void set_fs(uint16_t seg)
{
	__asm__ ("mov fs,%0" : : "r" (seg));
}
uint16_t get_fs(void)
{
	uint16_t seg;
	__asm__ ("mov %0,fs" : "=r" (seg));
	return seg;
}

void set_gs(uint16_t seg)
{
	__asm__ ("mov gs,%0" : : "r" (seg));
}
uint16_t get_gs(void)
{
	uint16_t seg;
	__asm__ ("mov %0,gs" : "=r" (seg));
	return seg;
}