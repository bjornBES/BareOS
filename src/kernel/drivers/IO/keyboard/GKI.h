/*
 * File: GKI.h
 * File Created: 20 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

// GKI (generalized key input)
#pragma once

#include "kernel.h"
#include "Keyboard.h"

typedef struct
{
    key_code key;
    key_modifiers modifier;
    wchar_t key_char;
} gki_event_t;

void gki_process_key_event(key_event *ev, gki_event_t *gki_out);
