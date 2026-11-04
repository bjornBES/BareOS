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

#include "kernel_font.h"

#include <types.h>

status_t font_psf_load(font_t *out, const void *data, size_t size);
status_t font_psf_free(font_t *font);

const font_t *psf_font_builtin();
