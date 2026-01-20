#pragma once

#include <core/Defs.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

void mmInit(uint32_t _end);
void mmPrintStatus();
void mmPrintBlocks();

void* pmalloc(size_t size);
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);

ASMCALL void* memcpy(void* dst, const void* src, size_t num);
ASMCALL void* memset(void* ptr, int value, size_t num);
ASMCALL uint16_t* memset16(uint16_t* ptr, uint16_t value, size_t num);
ASMCALL uint32_t* memset32(uint32_t* ptr, uint32_t value, size_t num);
ASMCALL void* memmove(void* dst, const void* src, size_t num);
ASMCALL int memcmp(const void* ptr1, const void* ptr2, size_t num);
ASMCALL void* memchr(const void* ptr, int value, size_t num);