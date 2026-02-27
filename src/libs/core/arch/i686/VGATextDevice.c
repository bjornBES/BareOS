#include "VGATextDevice.h"
#include "IO.h"
#include <stddef.h>
#include <core/VGA/vga.h>


uint16_t ScreenWidth = 80;
uint16_t ScreenHeight = 25;
uint8_t DefaultColor = 0x7;
uint8_t* g_ScreenBuffer = (uint8_t*)0xB8000;
int m_ScreenX, m_ScreenY;

void putchr(int x, int y, char c)
{
    g_ScreenBuffer[2 * (y * ScreenWidth + x)] = c;
}

void putcolor(int x, int y, uint8_t color)
{
    g_ScreenBuffer[2 * (y * ScreenWidth + x) + 1] = color;
}

char getchr(int x, int y)
{
    return g_ScreenBuffer[2 * (y * ScreenWidth + x)];
}

uint8_t getcolor(int x, int y)
{
    return g_ScreenBuffer[2 * (y * ScreenWidth + x) + 1];
}

char _VGA_GetCell(int x, int y, uint8_t *color)
{
    int index = 2 * (y * ScreenWidth + x);
    char c = g_ScreenBuffer[index];
    *color = g_ScreenBuffer[index + 1];
    return c;
}
void _VGA_SetCell(int x, int y, char c, uint8_t color)
{
    int index = 2 * (y * ScreenWidth + x);
    g_ScreenBuffer[index] = c;
    g_ScreenBuffer[index + 1] = color;
}


void SetCursor(int x, int y)
{
    int pos = y * ScreenWidth + x;
    m_ScreenX = x;
    m_ScreenY = y;

    Outb(0x3D4, 0x0F);
    Outb(0x3D5, (uint8_t)(pos & 0xFF));
    Outb(0x3D4, 0x0E);
    Outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void GetCursor(int* x, int* y)
{
    *x = m_ScreenX;
    *y = m_ScreenY;
}

void Clear()
{
    for (int y = 0; y < ScreenHeight; y++)
        for (int x = 0; x < ScreenWidth; x++)
        {
            _VGA_SetCell(x,y,' ', DefaultColor);
        }
    m_ScreenX = 0;
    m_ScreenY = 0;
    SetCursor(m_ScreenX, m_ScreenY);
}

void Scrollback(int lines)
{
    for (int y = lines; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            uint8_t color;
            char c = _VGA_GetCell(x, y, &color);
            _VGA_SetCell(x, y - lines, c, color);
        }
    }
    for (int y = ScreenHeight - lines; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            _VGA_SetCell(x, y - lines, ' ', DefaultColor);
        }
    }
        
    m_ScreenY -= lines;
}

void putChar(char c)
{
    switch (c)
    {
        case '\n':
            m_ScreenX = 0;
            m_ScreenY++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (m_ScreenX % 4); i++)
                putChar(' ');
            break;

        case '\r':
            m_ScreenX = 0;
            break;

        default:
            uint8_t color;
            _VGA_GetCell(m_ScreenX, m_ScreenY, &color);
            _VGA_SetCell(m_ScreenX, m_ScreenY, c, color);
            m_ScreenX++;
            break;
    }

    if (m_ScreenX >= ScreenWidth)
    {
        m_ScreenY++;
        m_ScreenX = 0;
    }
    if (m_ScreenY >= ScreenHeight)
        Scrollback(1);

    SetCursor(m_ScreenX, m_ScreenY);
}

