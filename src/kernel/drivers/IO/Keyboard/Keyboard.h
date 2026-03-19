/*
 * File: Keyboard.h
 * File Created: 13 Mar 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 Mar 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    KBD_STATE_NORMAL,
    KBD_STATE_BREAK,        // saw 0xF0
    KBD_STATE_EXTENDED,     // saw 0xE0
    KBD_STATE_EXTENDED_BREAK, // saw 0xE0 0xF0
} KeyboardState;

typedef enum {
    KEY_PRESSED,
    KEY_RELEASED,
} KeyAction;

typedef enum {
    KEY_UNKNOWN = 0,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,
    KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,
    KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, KEY_SPACE,
    KEY_LSHIFT, KEY_RSHIFT, KEY_LCTRL, KEY_RCTRL,
    KEY_LALT, KEY_RALT, KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN,
    KEY_INSERT, KEY_DELETE,
    KEY_COUNT,
    KEY_COMMA, KEY_PERIOD, KEY_MINUS,
} KeyCode;

typedef struct {
    KeyCode key;
    KeyAction action;
} key_event;

bool keyboard_process_byte(uint8_t raw, key_event *out);
bool keyboard_put_key(key_event *key);
bool keyboard_get_key(key_event *key);
uint32_t keyboard_get_ascii(key_event *key);

void keyboard_init();