/*
 * File: kernel_font.h
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

typedef struct font_unicode_map
{
    struct unicode_map_entry
    {
        uint32_t ch;
        uint32_t glyph;
    } *e;

    size_t count;
    size_t cap;
} font_unicode_map_t;

typedef struct font
{
    uint32_t width, height;  // glyph size in pixels
    uint32_t glyph_count;
    uint32_t glyph_size;     // bytes per glyph = ((width + 7) / 8) * height

    const uint8_t *glyphs;   // glyph_count * glyph_size bytes, NOT copied
    font_unicode_map_t *map; // NULL if the font has no table (cp == glyph index)

    uint32_t fallback;       // glyph index drawn for missing codepoints
} font_t;
