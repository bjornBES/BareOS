/*
 * File: GKI.c
 * File Created: 20 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 31 May 2026
 * Modified By: BjornBEs
 * -----
 */

#include "GKI.h"

#include "kernel/memory.h"

#define KEY_MAX 255

// key_code -> uint32_t
static const wchar_t ascii[KEY_MAX] = {
    [KEY_ESC] = '\e',
    [KEY_1] = '1',
    [KEY_2] = '2',
    [KEY_3] = '3',
    [KEY_4] = '4',
    [KEY_5] = '5',
    [KEY_6] = '6',
    [KEY_7] = '7',
    [KEY_8] = '8',
    [KEY_9] = '9',
    [KEY_0] = '0',
    [KEY_MINUS] = '-',
    [KEY_EQUAL] = '=',
    [KEY_BACKSPACE] = '\b',
    [KEY_TAB] = '\t',
    [KEY_Q] = 'q',
    [KEY_W] = 'w',
    [KEY_E] = 'e',
    [KEY_R] = 'r',
    [KEY_T] = 't',
    [KEY_Y] = 'y',
    [KEY_U] = 'u',
    [KEY_I] = 'i',
    [KEY_O] = 'o',
    [KEY_P] = 'p',
    [KEY_LEFTBRACE] = '[',
    [KEY_RIGHTBRACE] = ']',
    [KEY_ENTER] = '\r',
    [KEY_LEFTCTRL] = 0,
    [KEY_A] = 'a',
    [KEY_S] = 's',
    [KEY_D] = 'd',
    [KEY_F] = 'f',
    [KEY_G] = 'g',
    [KEY_H] = 'h',
    [KEY_J] = 'j',
    [KEY_K] = 'k',
    [KEY_L] = 'l',
    [KEY_SEMICOLON] = ';',
    [KEY_APOSTROPHE] = '\'',
    [KEY_GRAVE] = '`',
    [KEY_LEFTSHIFT] = 0,
    [KEY_BACKSLASH] = '\\',
    [KEY_Z] = 'z',
    [KEY_X] = 'x',
    [KEY_C] = 'c',
    [KEY_V] = 'v',
    [KEY_B] = 'b',
    [KEY_N] = 'n',
    [KEY_M] = 'm',
    [KEY_COMMA] = ',',
    [KEY_DOT] = '.',
    [KEY_SLASH] = '/',
    [KEY_RIGHTSHIFT] = 0,
    [KEY_KPASTERISK] = '*',
    [KEY_LEFTALT] = 0,
    [KEY_SPACE] = ' ',
    [KEY_CAPSLOCK] = 0,
    [KEY_F1] = 0,
    [KEY_F2] = 0,
    [KEY_F3] = 0,
    [KEY_F4] = 0,
    [KEY_F5] = 0,
    [KEY_F6] = 0,
    [KEY_F7] = 0,
    [KEY_F8] = 0,
    [KEY_F9] = 0,
    [KEY_F10] = 0,
    [KEY_NUMLOCK] = 0,
    [KEY_SCROLL_LOCK] = 0,
    [KEY_KP7] = '7',
    [KEY_KP8] = '8',
    [KEY_KP9] = '9',
    [KEY_KPMINUS] = '-',
    [KEY_KP4] = '4',
    [KEY_KP5] = '5',
    [KEY_KP6] = '6',
    [KEY_KPPLUS] = '+',
    [KEY_KP1] = '1',
    [KEY_KP2] = '2',
    [KEY_KP3] = '3',
    [KEY_KP0] = '0',
    [KEY_KPDOT] = '.',
    [KEY_102ND] = '\\',
    [KEY_DELETE] = '\x7f',
    [KEY_NONE_END] = '\0',
};

static const wchar_t keymap_shift[KEY_MAX] = {
    [KEY_ESC] = '\e',
    [KEY_1] = '!',
    [KEY_2] = '@',
    [KEY_3] = '#',
    [KEY_4] = '$',
    [KEY_5] = '%',
    [KEY_6] = '^',
    [KEY_7] = '&',
    [KEY_8] = '*',
    [KEY_9] = '(',
    [KEY_0] = ')',
    [KEY_MINUS] = '_',
    [KEY_EQUAL] = '+',
    [KEY_BACKSPACE] = '\b',
    [KEY_TAB] = '\t',
    [KEY_Q] = 'Q',
    [KEY_W] = 'W',
    [KEY_E] = 'E',
    [KEY_R] = 'R',
    [KEY_T] = 'T',
    [KEY_Y] = 'Y',
    [KEY_U] = 'U',
    [KEY_I] = 'I',
    [KEY_O] = 'O',
    [KEY_P] = 'P',
    [KEY_LEFTBRACE] = '{',
    [KEY_RIGHTBRACE] = '}',
    [KEY_ENTER] = '\r',
    [KEY_LEFTCTRL] = 0,
    [KEY_A] = 'A',
    [KEY_S] = 'S',
    [KEY_D] = 'D',
    [KEY_F] = 'F',
    [KEY_G] = 'G',
    [KEY_H] = 'H',
    [KEY_J] = 'J',
    [KEY_K] = 'K',
    [KEY_L] = 'L',
    [KEY_SEMICOLON] = ':',
    [KEY_APOSTROPHE] = '\"',
    [KEY_GRAVE] = '~',
    [KEY_LEFTSHIFT] = 0,
    [KEY_BACKSLASH] = '|',
    [KEY_Z] = 'Z',
    [KEY_X] = 'X',
    [KEY_C] = 'C',
    [KEY_V] = 'V',
    [KEY_B] = 'B',
    [KEY_N] = 'N',
    [KEY_M] = 'M',
    [KEY_COMMA] = '<',
    [KEY_DOT] = '>',
    [KEY_SLASH] = '?',
    [KEY_RIGHTSHIFT] = 0,
    [KEY_KPASTERISK] = '*',
    [KEY_LEFTALT] = 0,
    [KEY_SPACE] = ' ',
    [KEY_CAPSLOCK] = 0,
    [KEY_F1] = 0,
    [KEY_F2] = 0,
    [KEY_F3] = 0,
    [KEY_F4] = 0,
    [KEY_F5] = 0,
    [KEY_F6] = 0,
    [KEY_F7] = 0,
    [KEY_F8] = 0,
    [KEY_F9] = 0,
    [KEY_F10] = 0,
    [KEY_NUMLOCK] = 0,
    [KEY_SCROLL_LOCK] = 0,
    [KEY_KP7] = 0,
    [KEY_KP8] = 0,
    [KEY_KP9] = 0,
    [KEY_KPMINUS] = '-',
    [KEY_KP4] = 0,
    [KEY_KP5] = '5',
    [KEY_KP6] = 0,
    [KEY_KPPLUS] = '+',
    [KEY_KP1] = 0,
    [KEY_KP2] = 0,
    [KEY_KP3] = 0,
    [KEY_KP0] = 0,
    [KEY_KPDOT] = '\x7f',
    [KEY_102ND] = '|',
    [KEY_DELETE] = '\x7f',
    [KEY_NONE_END] = '\0',
};

wchar_t gki_layout_resolve(key_code key, key_modifiers mods)
{
    if (mods & MOD_SHIFT)
    {
        return keymap_shift[key];
    }
    return ascii[key];
}

void gki_process_key_event(key_event *ev, gki_event_t *gki_out)
{
    // drop key-up events for non-modifiers
    if (ev->action == KEY_RELEASED)
    {
        return; // modifier state already captured in ev->mods
    }

    if (gki_out)
    {
        gki_out->key = ev->key;
        gki_out->modifier = ev->mods;
        gki_out->key_char = gki_layout_resolve(ev->key, ev->mods);
    }
}
