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

#include "kernel_fontt.h"

#include <types.h>

#define FONT_NO_GLYPH 0xFFFFFFFFu

status_t font_alloc(font_t **out);
status_t font_free(font_t *font);

status_t font_map_lookup(const font_t *font, wchar_t ch, uint32_t *cp_out);
status_t font_glyph_index(const font_t *font, wchar_t ch, uint32_t *out_cp);
status_t font_has_glyph(const font_t *font, wchar_t ch);
status_t font_get_glyph(const font_t *font, wchar_t ch, uint8_t **out_glyph);
