/*
 * File: kernel_font.c
 * File Created: 30 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 30 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/console/kernel_font.h"

#include "kerrno.h"

#include "memory.h"

#define MODULE "kernel-font"

status_t font_alloc(font_t **out)
{
    *out = kzmalloc(sizeof(font_t));
    return KERRNO_SUCCESSES;
}
status_t font_free(font_t *font)
{
    return kfree(font);    
}

status_t font_internal_map_lookup(const font_unicode_map_t *m, wchar_t ch, uint32_t *cp_out)
{
    // ENTER_FUNC("%p, 0x%x, %p", m, ch, cp_out);
    if (cp_out == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "cp_out is NULL");
    }
    size_t lo = 0;
    size_t hi = m->count;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (m->e[mid].ch < ch)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    // trace_debug(MODULE, "lo = %u, m = %p", lo, m);
    // trace_debug(MODULE, "m->e = %p", m->e);
    // trace_debug(MODULE, "cp_out = %p", cp_out);
    *cp_out = (lo < m->count && m->e[lo].ch == ch) ? m->e[lo].glyph : FONT_NO_GLYPH;
    if (*cp_out == FONT_NO_GLYPH)
    {
        return KERRNO_ENTRY_NOT_FOUND;
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
    }
    return KERRNO_SUCCESSES;
}

status_t font_map_lookup(const font_t *font, wchar_t ch, uint32_t *cp_out)
{
    // ENTER_FUNC("%p, 0x%x, %p", m, ch, cp_out);
    if (cp_out == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "cp_out is NULL");
    }
    return font_internal_map_lookup(font->map, ch, cp_out);
}

status_t font_glyph_index(const font_t *font, wchar_t ch, uint32_t *out_cp)
{
    // ENTER_FUNC("%p, %x, %p", font, ch, out_cp);
    if (out_cp == NULL)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "out_cp is NULL");
    }
    if (!font->map)
    {
        *out_cp = ch < font->glyph_count ? ch : FONT_NO_GLYPH;
        if (*out_cp == FONT_NO_GLYPH)
        {
            KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
        }
        return KERRNO_SUCCESSES;
    }
    status_t ret = font_internal_map_lookup(font->map, ch, out_cp);
    // KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        *out_cp = font->fallback;
        return KERRNO_BAD_INDEX;
        // KERRNO_RETURN(KERRNO_BAD_INDEX, "");
    }
    return ret;
}

status_t font_has_glyph(const font_t *font, wchar_t ch)
{
    uint32_t out_cp;
    if (!font->map)
    {
        out_cp = ch < font->glyph_count ? ch : FONT_NO_GLYPH;
        if (out_cp == FONT_NO_GLYPH)
        {
            KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
        }
        return KERRNO_SUCCESSES;
    }
    status_t ret = font_internal_map_lookup(font->map, ch, &out_cp);
    // KERRNO_CHECK_RETURN(ret);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return ret;
}

status_t font_get_glyph(const font_t *font, wchar_t ch, uint8_t **out_glyph)
{
    *out_glyph = (uint8_t *)(font->glyphs + ch * font->glyph_size);
    return KERRNO_SUCCESSES;
}