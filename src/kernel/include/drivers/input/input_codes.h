/*
 * File: input_codes.h
 * File Created: 27 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

typedef enum
{
    KEY_PRESSED,
    KEY_RELEASED,
} key_action_t;

typedef enum
{
    KEY_RESERVED = 0,
    KEY_ESCAPE,
    KEY_ESC = KEY_ESCAPE,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O = 24,
    KEY_P,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_ENTER,
    KEY_LEFTCTRL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE = 41,
    KEY_LEFTSHIFT,
    KEY_BACKSLASH = 0x2B,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_RIGHTSHIFT,
    KEY_KPASTERISK = 0x37,
    KEY_LEFTALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK = 69 /* nice */,
    KEY_SCROLL_LOCK,
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    KEY_KPMINUS,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,
    KEY_KPPLUS,
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP0,
    KEY_KPDOT = 83,

    KEY_ZENKAKUHANKAKU = 85,
    KEY_102ND = 0x56,
    KEY_F11,
    KEY_F12,
    KEY_RO,
    KEY_KATAKANA,
    KEY_HIRAGANA,
    KEY_HENKAN,
    KEY_KATAKANAHIRAGANA,
    KEY_MUHENKAN,
    KEY_KPJPCOMMA,
    KEY_KPENTER,
    KEY_RIGHTCTRL,
    KEY_KPSLASH,
    KEY_SYSRQ,
    KEY_RIGHTALT,
    KEY_LINEFEED,
    KEY_HOME,
    KEY_UP,
    KEY_PAGEUP,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_END,
    KEY_DOWN,
    KEY_PAGEDOWN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_MACRO,
    KEY_MUTE,
    KEY_VOLUMEDOWN,
    KEY_VOLUMEUP,
    KEY_POWER,
    KEY_KPEQUAL,
    KEY_KPPLUSMINUS,
    KEY_PAUSE,
    KEY_SCALE,
    KEY_KPCOMMA,
    KEY_HANGEUL,
    KEY_HANGUEL = KEY_HANGEUL,
    KEY_HANJA,
    KEY_YEN,
    KEY_LEFTMETA,
    KEY_RIGHTMETA,
    KEY_COMPOSE,

    KEY_CNT,
    KEY_NONE_END,
} key_code_t;

typedef enum
{
    MOD_NONE = 0,
    MOD_LSHIFT = (1 << 0),
    MOD_RSHIFT = (1 << 1),
    MOD_LCTRL = (1 << 2),
    MOD_RCTRL = (1 << 3),
    
    MOD_LALT = (1 << 4),
    MOD_RALT = (1 << 5),
    MOD_CAPS = (1 << 6),
} key_modifiers_t;

#define MOD_SHIFT (MOD_LSHIFT | MOD_RSHIFT)
#define MOD_CTRL (MOD_LCTRL | MOD_RCTRL)
#define MOD_ALT (MOD_LALT | MOD_RALT)

typedef struct
{
    key_code_t key;
    key_action_t action;
    key_modifiers_t mods;
} key_event_t;