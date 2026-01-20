#pragma once

#include <stdint.h>

void Clear();

void _VGA_SetCell(int x, int y, char c, uint8_t color);

void SetCursor(int x, int y);
void GetCursor(int* x, int* y);
void Scrollback(int lines);

void putChar(char c);

extern uint16_t ScreenWidth;
extern uint16_t ScreenHeight;
extern int m_ScreenX, m_ScreenY;
extern uint8_t *g_ScreenBuffer;