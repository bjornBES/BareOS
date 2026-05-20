#include "tty.h"

#include "libs/malloc.h"
#include "device/device.h"

#include "drivers/IO/Keyboard/Keyboard.h"

void tty_input(tty_t *tty)
{
    key_event key;
    keyboard_get_key(&key);
}

void tty_init()
{
    device_t *tty = malloc(sizeof(device_t));

    device_add(tty); 
}

