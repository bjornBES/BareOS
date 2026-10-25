/*
 * File: font_psf.h
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <types.h>

#define FONT_NO_GLYPH 0xFFFFFFFFu

typedef struct font_unicode_map
{
    struct unicode_map_entry
    {
        uint32_t cp;
        uint32_t glyph;
    } *e;

    size_t count;
    size_t cap;
} font_unicode_map_t;

typedef struct font
{
    uint32_t width, height;       // glyph size in pixels
    uint32_t glyph_count;
    uint32_t glyph_size;          // bytes per glyph = ((width + 7) / 8) * height
    const uint8_t *glyphs;        // glyph_count * glyph_size bytes, NOT copied
    font_unicode_map_t *map; // NULL if the font has no table (cp == glyph index)
    uint32_t fallback;            // glyph index drawn for missing codepoints
} font_t;

status_t font_psf_load(font_t *out, const void *data, size_t size);
status_t font_psf_free(font_t *font);

const font_t *psf_font_builtin();

status_t font_glyph_index(const font_t *font, uint32_t cp, uint32_t *out_cp);
status_t font_has_glyph(const font_t *font, uint32_t cp);
status_t font_get_glyph(const font_t *font, uint32_t cp, uint8_t **out_glyph);
