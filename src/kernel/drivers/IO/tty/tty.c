/*
 * File: tty.c
 * File Created: 15 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 11 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty.h"
#include "tty_types.h"
#include "tty_config.h"
#include "tty_flags.h"

#include "kernel/memory.h"
#include "kernel/irq.h"
#include "kernel/string.h"
#include "device/device.h"

#include "drivers/IO/keyboard/Keyboard.h"
#include "drivers/IO/keyboard/GKI.h"

#include "task/threading/scheduling/scheduler.h"

#include "errno/errno.h"
#include <signals.h>

#define MODULE "TTY"

int tty_count = 0;

ssize_t tty_read_dev(void *buf, off_t offset, size_t count, device_t *dev);
ssize_t tty_write_dev(void *buf, off_t offset, size_t count, device_t *dev);
int tty_ioctl_dev(uint32_t cmd, void *arg, device_t *dev);
void tty_destroy_dev(device_t *dev);

// --- lifecycle ---

// allocate and initialise a tty, hook it to a gki ring buffer
tty_struct_t *tty_create(device_t *input_dev, device_t *output_dev)
{
    tty_struct_t *new = malloc(sizeof(tty_struct_t));
    memset(new, 0, sizeof(tty_struct_t));
    new->input_dev = input_dev;
    new->output_dev = output_dev;
    tty_termios_init(new);

    if (input_dev)
    {
        if (input_dev->tty_ops)
        {
            input_dev->tty_ops(input_dev, &new->in_ops);
        }
    }
    if (output_dev)
    {
        if (output_dev->tty_ops)
        {
            output_dev->tty_ops(input_dev, &new->out_ops);
        }
    }

    device_t *tty_device = malloc(sizeof(device_t));
    memset(tty_device, 0, sizeof(device_t));
    tty_device->class_name = "tty";
    tty_device->type = DEVICE_TTY;
    tty_device->flags = DEVICE_FLAG_RW | DEVICE_FLAG_BLOCKDEV | DEVICE_FLAG_VIRTUAL;
    tty_device->read = tty_read_dev;
    tty_device->write = tty_write_dev;
    tty_device->ioctl = tty_ioctl_dev;
    tty_device->destroy = tty_destroy_dev;
    tty_device->priv = new;
    device_register(tty_device);
    new->id = (int)tty_device->device_id;

    return new;
}

// tear down
void tty_destroy(tty_struct_t *tty)
{
}

// --- signal handling ---

// send a signal to the foreground process group
void tty_signal(tty_struct_t *tty, int signo)
{
}


// writes buf to the output side, handling \n -> \r\n if ONLCR set
ssize_t tty_write(tty_struct_t *tty, const uint8_t *buf, size_t n)
{
    // ENTER_FUNC(MODULE, "%p, %p, %u", tty, buf, n);
    for (size_t i = 0; i < n; i++)
    {
        uint8_t c = buf[i];
        if (c == '\n' && FLAG_IS_SET(tty->termios.c_oflag, ONLCR))
        {
            tty->out_ops.write_char(tty->output_dev, '\r');
        }
        tty->out_ops.write_char(tty->output_dev, c);
    }
    return n;
}

ssize_t tty_read_dev(void *buf, off_t offset, size_t count, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_read(tty, buf, count);
}

ssize_t tty_write_dev(void *buf, off_t offset, size_t count, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_write(tty, buf, count);
}

int tty_ioctl_dev(uint32_t cmd, void *arg, device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    return tty_ioctl(tty, cmd, arg);
}

void tty_destroy_dev(device_t *dev)
{
    tty_struct_t *tty = dev->priv;
    tty_destroy(tty);
}

