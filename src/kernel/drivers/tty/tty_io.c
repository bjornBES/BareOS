/*
 * File: tty_io.c
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/tty.h"

#include "kerrno.h"

#include <array.h>

#define MODULE "tty-io"

// key_code -> uint32_t
static const wchar_t ascii[KEY_CNT] = {
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
};

static const wchar_t keymap_ctrl[KEY_CNT] = {
    [KEY_SLASH] = '\177',
    [KEY_A] = 1,
    [KEY_B] = 2,
    [KEY_C] = 3,
    [KEY_D] = 4,
    [KEY_E] = 5,
    [KEY_F] = 6,
    [KEY_G] = 7,
    [KEY_H] = 8,
    [KEY_I] = 9,
    [KEY_J] = 10,
    [KEY_K] = 11,
    [KEY_L] = 12,
    [KEY_M] = 13,
    [KEY_N] = 14,
    [KEY_O] = 15,
    [KEY_P] = 16,
    [KEY_Q] = 17,
    [KEY_R] = 18,
    [KEY_S] = 19,
    [KEY_T] = 20,
    [KEY_U] = 21,
    [KEY_V] = 22,
    [KEY_W] = 23,
    [KEY_X] = 24,
    [KEY_Y] = 25,
    [KEY_Z] = 26,
    [KEY_RIGHTBRACE] = 27,
    [KEY_BACKSLASH] = 28,
    [KEY_MINUS] = 31,
};

static const wchar_t keymap_shift[KEY_CNT] = {
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
};

#define ACTION_MAP_KEY(mod1, key_code) (uint64_t)((mod1) | ((uint64_t)key_code << 32))

typedef struct tty_action_raw
{
    uint64_t id;
    tty_action_t action;
} tty_action_raw_t;

static const tty_action_raw_t action_map[] = {
    {.id = ACTION_MAP_KEY(MOD_ALT, KEY_F1), .action = {}} // TODO
};

wchar_t tty_layout_resolve(key_code_t key, key_modifiers_t mods)
{
    ENTER_FUNC("0x%x, 0x%x", key, mods);
    if ((mods & (MOD_CTRL | MOD_SHIFT | MOD_ALT)) == (MOD_CTRL | MOD_SHIFT | MOD_ALT))
    {
        FUNC_NOT_IMPLEMENTED();
    }
    else if ((mods & (MOD_CTRL | MOD_SHIFT)) == (MOD_CTRL | MOD_SHIFT))
    {
        FUNC_NOT_IMPLEMENTED();
    }
    else if ((mods & (MOD_CTRL | MOD_ALT)) == (MOD_CTRL | MOD_ALT))
    {
        FUNC_NOT_IMPLEMENTED();
    }
    else if ((mods & (MOD_SHIFT | MOD_ALT)) == (MOD_SHIFT | MOD_ALT))
    {
        FUNC_NOT_IMPLEMENTED();
    }
    else if (mods & MOD_ALT)
    {
        FUNC_NOT_IMPLEMENTED();
    }
    else if (mods & MOD_CTRL)
    {
        return keymap_ctrl[key];
    }
    else if (mods & MOD_SHIFT || mods & MOD_CAPS)
    {
        return keymap_shift[key];
    }
    return ascii[key];
}

status_t tty_process_key_event(key_event_t *ev, wchar_t *char_out, tty_action_t **action_out)
{
    *action_out = NULL;
    // drop key-up events for non-modifiers
    if (ev->action == KEY_RELEASED)
    {
        return KERRNO_UNSUCCESS;
    }

    if (!char_out || !action_out)
    {
        KERRNO_RETURN(KERRNO_BAD_VALUE, "");
    }

    uint64_t action_id = ACTION_MAP_KEY(ev->mods, ev->key);

    for (size_t i = 0; i < ARRAY_SIZE(action_map); i++)
    {
        tty_action_raw_t raw = action_map[i];
        if (raw.id == action_id)
        {
            trace_debug(MODULE, "raw.id 0x%llx == id 0x%llx", raw.id, action_id);
            *action_out = &raw.action;
            return KERRNO_SUCCESSES;
        }
    }

    *char_out = tty_layout_resolve(ev->key, ev->mods);
    return KERRNO_SUCCESSES;
}

status_t tty_receive_key(tty_struct_t *tty, key_event_t *ev, device_id_t dev_id)
{
    ENTER_FUNC("%p, %p, %p", tty, ev, dev_id.device);
    if (ev->action == KEY_RELEASED)
    {
        return KERRNO_SUCCESSES;
    }
    wchar_t ch = 0;
    tty_action_t *action = NULL;
    bool is_action = false;
    tty_process_key_event(ev, &ch, &action);
    if (action != NULL)
    {
        FUNC_NOT_IMPLEMENTED();
        // TODO:
        return KERRNO_SUCCESSES;
    }
    return tty_receive(tty, ch);
}

status_t tty_echo(tty_struct_t *tty, char c)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t tty_echo_erase(tty_struct_t *tty)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

// --- line buffer ---

// push a byte into the line buffer
status_t tty_linebuf_push(tty_struct_t *tty, uint8_t byte)
{
    tty_line_buf_t *lb = &tty->line_buf;
    if (lb->head >= TTY_BUF_SIZE)
    {
        KERRNO_RETURN(ENOBUFS, "buffer full, drop");
    }

    lb->buf[lb->head++] = byte;
    return KERRNO_SUCCESSES;
}

// erase the last byte from the line buffer
status_t tty_linebuf_erase(tty_struct_t *tty)
{
    tty_line_buf_t *lb = &tty->line_buf;
    if (lb->head >= TTY_BUF_SIZE)
    {
        KERRNO_RETURN(ENOBUFS, "buffer full, drop");
    }
    lb->head--;
    if (FLAG_IS_SET(tty->termios.c_lflag, ECHOE))
    {
        return tty_echo_erase(tty);
    }
    return KERRNO_SUCCESSES;
}

status_t tty_process_byte(tty_struct_t *tty, wchar_t ch)
{
    // map \r → \n if ICRNL set
    if (ch == '\r' && FLAG_IS_SET(tty->termios.c_iflag, ICRNL))
    {
        ch = '\n';
    }

    // echo
    if (FLAG_IS_SET(tty->termios.c_lflag, ECHO))
    {
        // tty_echo(tty, ch);
    }

    // push into line buffer
    tty_linebuf_push(tty, ch);

    // if canonical mode and we got a newline, mark line complete
    if (FLAG_IS_SET(tty->termios.c_lflag, ICANON) && (ch == '\n' || ch == '\r'))
    {
        tty->line_buf.has_line = true;
    }
    return KERRNO_SUCCESSES;
}

status_t tty_process_event(tty_struct_t *tty, wchar_t ch)
{
    ENTER_FUNC("%p, 0x%x", tty, ch);

    {
        if (FLAG_IS_SET(tty->termios.c_lflag, ISIG))
        {
            if (ch == tty->termios.c_cc[VINTR])
            {
                // Ctrl+C
                FUNC_NOT_IMPLEMENTED();
                // tty_signal(tty, SIGINT, SI_USER);
                return KERRNO_SUCCESSES;
            }
            if (ch == tty->termios.c_cc[VSUSP])
            {
                // Ctrl+Z
                FUNC_NOT_IMPLEMENTED();
                // tty_signal(tty, SIGTSTP, SI_USER);
                return KERRNO_SUCCESSES;
            }
        }
        if (ch == tty->termios.c_cc[VEOF])
        {
            // Ctrl+D
            tty->line_buf.has_line = true; // wake reader with 0 bytes = EOF
            return KERRNO_SUCCESSES;
        }
        if (ch == tty->termios.c_cc[VERASE])
        {
            return tty_linebuf_erase(tty);
        }
    }

    // normal printable character
    return tty_process_byte(tty, ch);
}

status_t tty_receive(tty_struct_t *tty, wchar_t ch)
{
    tty_process_event(tty, ch);
    return KERRNO_SUCCESSES;
}
