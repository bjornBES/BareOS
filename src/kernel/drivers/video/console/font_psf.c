/*
 * File: font_psf.c
 * File Created: 25 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 25 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/video/console/font_psf.h"

#include "debug/debug.h"

#include "memory.h"
#include "stdlib.h"

#include <binary.h>
#include <defs.h>

#define PSF1_MODE512    BIT(0)
#define PSF1_MODEHASTAB BIT(1)
#define PSF1_MODESEQ    BIT(2)

#define PSF1_FONT_MAGIC 0x0436

#define MODULE          "font_psf"

/*
    For PSF1 glyph width is always = 8 bits
    and glyph height = characterSize
*/
typedef struct
{
    uint16_t magic;         // Magic bytes for identification.
    uint8_t font_mode;      // PSF font mode.
    uint8_t character_size; // PSF character size.
} psf1_header_t;

#define PSF_FONT_MAGIC 0x864AB572

/*
    PSF2
*/
typedef struct
{
    uint32_t magic;           /* magic bytes to identify PSF */
    uint32_t version;         /* zero */
    uint32_t header_size;     /* offset of bitmaps in file, 32 */
    uint32_t flags;           /* 0 if there's no unicode table */
    uint32_t num_glyph;       /* number of glyphs */
    uint32_t bytes_per_glyph; /* size of each glyph */
    uint32_t height;          /* height in pixels */
    uint32_t width;           /* width in pixels */
} psf_font_t;

status_t map_insert(font_unicode_map_t *m, uint32_t cp, uint32_t glyph)
{
    ENTER_FUNC("%p, 0x%x, 0x%x", m, cp, glyph);
    if (m->count >= m->cap)
    {
        m->cap <<= 1;
        m->e = krealloc(m->e, m->cap * sizeof(struct unicode_map_entry));
    }
    m->e[m->count].cp = cp;
    m->e[m->count].glyph = glyph;
    m->count++;
    return KERRNO_SUCCESSES;
}

status_t map_lookup(const font_unicode_map_t *m, uint32_t cp, uint32_t *cp_out)
{
    ENTER_FUNC("%p, 0x%x, %p", m, cp, cp_out);
    size_t lo = 0;
    size_t hi = m->count;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (m->e[mid].cp < cp)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    *cp_out = (lo < m->count && m->e[lo].cp == cp) ? m->e[lo].glyph : FONT_NO_GLYPH;
    if (*cp_out == FONT_NO_GLYPH)
    {
        KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
    }
    return KERRNO_SUCCESSES;
}

int map_compare_cp(const void *blockA, const void *blockB)
{
    struct unicode_map_entry *a = (struct unicode_map_entry *)blockA;
    struct unicode_map_entry *b = (struct unicode_map_entry *)blockB;
    return ((int)a->cp) - ((int)b->cp);
}

status_t map_finalize(const font_unicode_map_t *m)
{
    qsort(m->e, m->count, sizeof(struct unicode_map_entry), map_compare_cp);
    return KERRNO_SUCCESSES;
}

status_t parse_unicode_table_psf1(font_t *out, const uint8_t *tab, size_t len)
{
    ENTER_FUNC("%p, %p, 0x%x", out, tab, len);
    font_unicode_map_t *m = kzmalloc(sizeof(font_unicode_map_t));
    if (!m)
    {
        KERRNO_RETURN(ENOMEM, "");
    }
    m->cap = out->glyph_count;
    m->count = 0;
    m->e = kcalloc(m->cap, sizeof(struct unicode_map_entry));

    uint32_t glyph = 0;
    int in_seq = 0;

    for (size_t i = 0; i + 1 < len && glyph < out->glyph_count; i += 2)
    {
        uint16_t v = tab[i] | (tab[i + 1] << 8); // little-endian UCS-2
        if (v == 0xFFFF)
        {
            glyph++;
            in_seq = 0;
            continue;
        }
        if (v == 0xFFFE)
        {
            in_seq = 1;
            continue;
        }
        if (in_seq)
        {
            continue;
        }

        {

            if (map_insert(m, v, glyph) != KERRNO_SUCCESSES)
            {
                return kfree(m);
            }
        }
    }

    map_finalize(m); // sort, drop duplicates
    out->map = m;
    return KERRNO_SUCCESSES;
}

status_t parse_psf1(font_t *out, const uint8_t *data, size_t size)
{
    psf1_header_t *header = (psf1_header_t *)data;
    if (header->magic != PSF1_FONT_MAGIC)
    {
        return KERRNO_UNSUCCESS;
    }

    out->width = 8;
    out->height = header->character_size;
    out->glyph_size = ((out->width + 7) / 8) * out->height;
    out->glyph_count = 256;
    if (FLAG_IS_SET(header->font_mode, PSF1_MODE512))
    {
        out->glyph_count = 512;
    }
    // 16*256=4096
    out->glyphs = data + 4;

    size_t glyph_bytes = (size_t)out->glyph_count * out->glyph_size;
    if (size < 4 + glyph_bytes)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }

    uint8_t *end = (uint8_t *)data + size;

    uint8_t *s = (uint8_t *)(data + 4);
    for (size_t count = 0; count < out->glyph_count; count++)
    {
        for (size_t i = 0; i < out->glyph_size; i++)
        {
            uint16_t index = (count * out->glyph_size) + i;
            uint8_t v = s[index];
            trace_debug(MODULE, "0x%04x, %08b %p", index, v, &s[index]);
        }
        trace_debug(MODULE, "");
    }

    if (FLAG_IS_SET(header->font_mode, PSF1_MODEHASTAB) || FLAG_IS_SET(header->font_mode, PSF1_MODESEQ))
    {
        status_t ret = parse_unicode_table_psf1(out, data + 4 + glyph_bytes, size - 4 - glyph_bytes);
    }

    map_lookup(out->map, '?', &out->fallback);
    // 63

    return KERRNO_SUCCESSES;
}

INTERNAL status_t parse_unicode_table_psf2(font_t *out, const uint8_t *tab, size_t len);

status_t parse_psf2(font_t *out, const uint8_t *data, size_t size)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t font_psf_load(font_t *out, const void *data, size_t size)
{
    ENTER_FUNC("%p, %p, 0x%x", out, data, size);
    status_t ret = parse_psf1(out, data, size);
    if (ret == KERRNO_UNSUCCESS)
    {
        ret = parse_psf2(out, data, size);
    }
    return ret;
}

const font_t *psf_font_builtin()
{
    font_t *out = kmalloc(sizeof(font_t));
    extern char _binary_font_psf_start;
    extern char _binary_font_psf_end;
    uint8_t *start = (uint8_t *)&_binary_font_psf_start;
    uint8_t *end = (uint8_t *)&_binary_font_psf_end;
    size_t size = end - start;
    font_psf_load(out, start, size);
    return out;
}

status_t font_glyph_index(const font_t *font, uint32_t cp, uint32_t *out_cp)
{
    if (!font->map)
    {
        *out_cp = cp < font->glyph_count ? cp : FONT_NO_GLYPH;
        if (*out_cp == FONT_NO_GLYPH)
        {
            KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
        }
        return KERRNO_SUCCESSES;
    }
    status_t ret = map_lookup(font->map, cp, out_cp);
    if (ret != KERRNO_SUCCESSES)
    {
        *out_cp = font->fallback;
        return ret;
    }
    return ret;
}

status_t font_has_glyph(const font_t *font, uint32_t cp)
{
    uint32_t out_cp;
    if (!font->map)
    {
        out_cp = cp < font->glyph_count ? cp : FONT_NO_GLYPH;
        if (out_cp == FONT_NO_GLYPH)
        {
            KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
        }
        return KERRNO_SUCCESSES;
    }
    status_t ret = map_lookup(font->map, cp, &out_cp);
    if (ret != KERRNO_SUCCESSES)
    {
        return ret;
    }
    return ret;
}

status_t font_get_glyph(const font_t *font, uint32_t cp, uint8_t **out_glyph)
{
    // uint32_t out_cp;
    // if (!font->map)
    // {
    //     out_cp = cp < font->glyph_count ? cp : FONT_NO_GLYPH;
    //     if (out_cp == FONT_NO_GLYPH)
    //     {
    //         KERRNO_RETURN(KERRNO_ENTRY_NOT_FOUND, "");
    //     }
    // }
    // else
    // {
    //     map_lookup(font->map, cp, &out_cp);
    // }
    *out_glyph = (uint8_t *)(font->glyphs + cp * font->glyph_size);
    return KERRNO_SUCCESSES;
}
