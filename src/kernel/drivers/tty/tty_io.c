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
#include "drivers/base/driver.h"

#include "drivers/base/core.h"
#include "drivers/base/class.h"

#include "dev/device.h"

#include "lists/list.h"

#include "vfs/inodet.h"

#include "kerrno.h"

#include "memory.h"

#include "stdio.h"

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

extern list_t tty_drivers;
extern device_t *console_dev;
extern const dev_class_t tty_class;

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

status_t tty_write(tty_struct_t *tty, wchar_t *ch, size_t count, ssize_t *out_size)
{
    ENTER_FUNC("%p, %p, %u, %p", tty, ch, count, out_size);
    ssize_t size;
    status_t ret = tty->ops->write(tty, ch, count, &size);
    if (out_size)
    {
        *out_size = size;
    }
    return ret;
}

status_t tty_do_resize(tty_struct_t *tty, winsize_t *ws)
{
    // FUNC_NOT_IMPLEMENTED();
    tty->winsize = *ws;
    return KERRNO_SUCCESSES;
}

INTERNAL ssize_t tty_line_name(tty_driver_t *driver, int index, char *p)
{
    if (driver->flags & TTY_DRIVER_UNNUMBERED_NODE)
    {
        return snprintf(p, DEVICE_NAME_MAX, "%s", driver->name);
    }
    else
    {
        return snprintf(p, DEVICE_NAME_MAX, "%s%d", driver->name, index + driver->name_base);
    }
}

kernel_dev_t tty_devnum(tty_struct_t *tty)
{
    return MKDEV_KERNEL(tty->driver->major, tty->driver->device, tty->driver->minor_start + tty->index);
}

INTERNAL device_t *tty_get_device(tty_struct_t *tty)
{
    dev_t devt = tty_devnum(tty);

    device_t *dev;
    class_find_device_by_devt(&tty_class, devt, &dev);
    return dev;
}

status_t alloc_tty_struct(tty_driver_t *driver, int idx, tty_struct_t **out_tty)
{
    ENTER_FUNC("%p, %u, %p", driver, idx, out_tty);
    tty_struct_t *tty;

    tty = kzmalloc(sizeof(tty_struct_t));
    if (!tty)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    if (tty_ldisc_init(tty))
    {
        return kfree(tty);
    }

    tty->driver = driver;
    tty->ops = driver->ops;
    tty->index = idx;
    tty_line_name(driver, idx, tty->name);
    device_t *dev = tty_get_device(tty);
    trace_info(MODULE, "device: %u (%p), id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p, parent: %p", 0, dev, dev->id, dev->devt.major, dev->devt.device, dev->devt.minor, dev->class->name, dev->class->type, dev->name, dev->driver, dev->parent);
    tty->tty_dev = to_tty_dev(dev);
    *out_tty = tty;

    return KERRNO_SUCCESSES;
}

status_t tty_register_device_attr(tty_driver_t *driver, uint32_t index, device_t *device, void *drvdata, device_t **dev_out)
{
    ENTER_FUNC("%p, %u, %p, %p, %p", driver, index, device, drvdata, dev_out);
    char name[DEVICE_NAME_MAX];
    status_t ret;

    if (index >= driver->num)
    {
        KERRNO_RETURN(EINVAL, "%s: Attempt to register invalid tty line number (%d)", driver->name, index);
    }

    size_t s = 0;
    if (driver->type == TTY_DRIVER_TYPE_PTY)
    {
        FUNC_NOT_IMPLEMENTED();
        // pty_line_name(driver, index, name);
    }
    else
    {
        s = tty_line_name(driver, index, name);
    }
    name[s] = '\0';

    device_t *dev = kzmalloc(sizeof(device_t));
    if (!dev)
    {
        KERRNO_RETURN(ENOMEM, "");
    }

    dev->class = &tty_class;
    dev->parent = device;
    // dev->ops.release = tty_device_create_release;
    device_set_name(dev, "%s", name);
    // dev->groups = attr_grp;
    device_set_drvdata(dev, drvdata);
    dev->driver = &driver->driver;

    ret = device_register(dev);
    if (ret)
    {
        goto err_put;
    }

    *dev_out = dev;
    return KERRNO_SUCCESSES;

// err_del:
// device_del(dev);
err_put:
    // put_device(dev);

    return ret;
}

status_t tty_register_device(tty_driver_t *driver, uint32_t index, device_t *device, device_t **out_dev)
{
    return tty_register_device_attr(driver, index, device, NULL, out_dev);
}

status_t tty_unregister_device(tty_driver_t *tty_driver, unsigned index)
{
    // device_destroy(&tty_class, MKDEV(tty_driver->major, tty_driver->minor_start) + index);
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_SUCCESSES;
}

status_t tty_init_dev(tty_driver_t *driver, int idx, tty_struct_t **tty_out, tty_dev_t **dev_out)
{
    ENTER_FUNC("%p, %u, %p, %p", driver, idx, tty_out, dev_out);
    tty_struct_t *tty;
    status_t ret;

    device_t *dev;
    ret = tty_register_device(driver, idx, NULL, &dev);
    if (ret)
    {
        return ret;
    }
    tty_dev_t *tty_dev = to_tty_dev(dev);
    *dev_out = tty_dev;

    ret = alloc_tty_struct(driver, idx, &tty);
    if (ret)
    {
        return ret;
    }
    *tty_out = tty;

    tty_dev->tty = tty;
    tty->tty_dev = tty_dev;
    return KERRNO_SUCCESSES;
}

status_t tty_register_driver(tty_driver_t *tty_driver)
{
    ENTER_FUNC("%p", tty_driver);
    tty_dev_t *tty_dev = core_device_create_type(&tty_class, NULL, tty_dev_t, NULL, tty_driver->name);
    tty_driver->device = tty_dev->dev.devt.device;
    tty_driver->minor_start = tty_dev->dev.devt.minor + 1;
    // return drivers_register(&tty_driver->driver);

    tty_dev->dev.driver = &tty_driver->driver;
    tty_dev->driver = tty_driver;

    status_t ret = 0;

    ADD_THIS_TO_LIST(tty_drivers, tty_driver);

    int i = 0;

    if (!(tty_driver->flags & TTY_DRIVER_DYNAMIC_DEV))
    {
        for (i = 0; i < tty_driver->num; i++)
        {
            tty_dev_t *t_dev;
            tty_struct_t *tty;
            ret = tty_init_dev(tty_driver, i, &tty, &t_dev);
            KERRNO_CHECK_RETURN(ret);
            if (ret)
            {
                goto err_unreg_devs;
            }
            tty_driver->ttys[i] = tty;
            tty_driver->ops->install(tty_driver, tty);
            if (!active_tty && i == 1)
            {
                active_tty = tty;
            }
        }
    }
    tty_driver->flags |= TTY_DRIVER_INSTALLED;

    if (!console_dev->drv_data)
    {
        device_set_drvdata(console_dev, (void *)tty_dev->dev.devt.id);
    }

    return KERRNO_SUCCESSES;

err_unreg_devs:
    for (i--; i >= 0; i--)
    {
        tty_unregister_device(tty_driver, i);
    }

    list_remove(&tty_drivers, &THIS_NODE(tty_driver));

    // err_unreg_char:
    device_unregister(&tty_dev->dev);
    // err:
    return ret;
}

status_t tty_alloc_driver(unsigned int lines, unsigned long flags, tty_driver_t **out)
{
    tty_driver_t *driver;
    status_t ret;

    if (!lines || (flags & TTY_DRIVER_UNNUMBERED_NODE && lines > 1))
    {
        KERRNO_RETURN(EINVAL, "");
    }

    driver = kzmalloc(sizeof(tty_driver_t));
    if (!driver)
    {
        KERRNO_RETURN(ENOMEM, "no more memory");
    }

    driver->num = lines;
    driver->flags = flags;

    if (!(flags & TTY_DRIVER_DEVPTS_MEM))
    {
        driver->ttys = kcalloc(lines, sizeof(tty_struct_t *));
        driver->termios = kcalloc(lines, sizeof(termios_t));
        if (!driver->ttys || !driver->termios)
        {
            KERRNO_NO_RETURN(ENOMEM, "");
            ret = ENOMEM;
            goto err_free_all;
        }
    }

    if (!(flags & TTY_DRIVER_DYNAMIC_ALLOC))
    {
        // driver->ports = kzalloc_objs(*driver->ports, lines);
        // if (!driver->ports)
        // {
        //     err = -ENOMEM;
        //     goto err_free_all;
        // }
    }

    // driver->cdevs = kzalloc_objs(*driver->cdevs, cdevs);
    // if (!driver->cdevs)
    // {
    //     err = -ENOMEM;
    //     goto err_free_all;
    // }

    *out = driver;
    return KERRNO_SUCCESSES;
err_free_all:
    // kfree(driver->ports);
    kfree(driver->ttys);
    kfree(driver->termios);
    // kfree(driver->cdevs);
    kfree(driver);
    return ret;
}

INTERNAL status_t tty_open_by_driver(device_t *dev, vfs_node_t *filp, tty_struct_t **out_tty)
{
    tty_struct_t *tty;
    tty_dev_t *tty_dev = to_tty_dev(dev);
    tty_driver_t *tty_driver = tty_dev->driver;
    trace_info(MODULE, "tty_driver->major = %u, tty_driver->device = %u, tty_driver->minor_start = %u, tty_driver->name_base = %u, tty_driver->num = %u", tty_driver->major, tty_driver->device, tty_driver->minor_start, tty_driver->name_base, tty_driver->num);

    status_t ret = 0;
    int index = 0;
    if (tty_driver->ops->get_index)
    {
        ret = tty_driver->ops->get_index(tty_driver, &index);
    }
    else
    {
        FUNC_NOT_IMPLEMENTED();
    }

    if (ret)
    {
        return ret;
    }

    if (index >= tty_driver->num)
    {
        KERRNO_RETURN(EINVAL, "");
    }
    tty = tty_driver->ttys[index];
    if (tty)
    {
        return KERRNO_UNSUCCESS;
    }
    else
    {
        // ret = tty_init_dev(tty_driver, index, &tty);
    }

    if (ret)
    {
        return ret;
    }
    *out_tty = tty;
    return KERRNO_SUCCESSES;
}

status_t tty_open(inode_t *inode, vfs_node_t *filp)
{
    /* ENTER_FUNC("%p, %p", inode, filp);
    dev_inode_t *dev_inode = to_dev_inode(inode);
    device_t *dev;
    status_t ret = device_get_device(&dev_inode->dev_id, &dev);

    trace_info(MODULE, "device: %u (%p), id:0x%x, dev_id=%u:%u:%u, %s(%u), name:%s, driver:%p, parent: %p", 0, dev, dev->id, dev->devt.major, dev->devt.device, dev->devt.minor, dev->class->name, dev->class->type, dev->name, dev->driver, dev->parent);

    tty_struct_t *tty;
    tty_open_by_driver(dev, filp, &tty);

    if (tty->ops->open)
    {
        ret = tty->ops->open(tty, filp);
    }
    else
    {
        KERRNO_RETURN(ENODEV, "");
    }

    while (true)
    {
        ;
    } */
    return KERRNO_SUCCESSES;
}
