#include <core/arch/i686/bios.h>

uint16_t ds(void)
{
	uint16_t seg;
	__asm__ ("movw %%ds,%0" : "=rm" (seg));
	return seg;
}

void set_fs(uint16_t seg)
{
	__asm__ ("movw %0,%%fs" : : "rm" (seg));
}
uint16_t fs(void)
{
	uint16_t seg;
	__asm__ ("movw %%fs,%0" : "=rm" (seg));
	return seg;
}

void set_gs(uint16_t seg)
{
	__asm__ ("movw %0,%%gs" : : "rm" (seg));
}
uint16_t gs(void)
{
	uint16_t seg;
	__asm__ ("movw %%gs,%0" : "=rm" (seg));
	return seg;
}