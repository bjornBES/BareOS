/*
 * File: tty_baudrate.c
 * File Created: 20 Nov 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 20 Nov 2026
 * Modified By: BjornBEs
 * -----
 */

#include "drivers/tty/termios.h"
#include "debug/debug.h"
#include <array.h>

#define MODULE "tty-baudrate"

static const speed_t baud_table[] = {
    0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
    4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
    500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000,
    2500000, 3000000, 3500000, 4000000};

static const tcflag_t baud_bits[] = {
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800,
    B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000,
    B2500000, B3000000, B3500000, B4000000};

static int n_baud_table = ARRAY_SIZE(baud_table);

void tty_termios_encode_baud_rate(termios_t *termios, speed_t ibaud, speed_t obaud)
{
    // ENTER_FUNC("%p, %u, %u", termios, ibaud, obaud);

    if (obaud == 0)
    {
        ibaud = 0;
    }

    termios->c_ispeed = ibaud;
    termios->c_ospeed = obaud;
    int ibinput = 0;
    if (((termios->c_cflag >> IBSHIFT) & CBAUD) != B0)
    {
        ibinput = 1;
    }

    int iclose = ibaud / 50;
    int oclose = obaud / 50;
    if ((termios->c_cflag & CBAUD) == BOTHER)
    {
        oclose = 0;
        if (!ibinput)
        {
            iclose = 0;
        }
    }
    if (((termios->c_cflag >> IBSHIFT) & CBAUD) == BOTHER)
    {
        iclose = 0;
    }

    termios->c_cflag &= ~CBAUD;
    termios->c_cflag &= ~(CBAUD << IBSHIFT);

    int i = 0;
    int ifound = -1;
    int ofound = -1;
    do
    {
        if (obaud - oclose <= baud_table[i] && obaud + oclose >= baud_table[i])
        {
            termios->c_cflag |= baud_bits[i];
            ofound = i;
        }
        if (ibaud - iclose <= baud_table[i] && ibaud + iclose >= baud_table[i])
        {
            if (ofound == i && !ibinput)
            {
                ifound = i;
            }
            else
            {
                ifound = i;
                termios->c_cflag |= (baud_bits[i] << IBSHIFT);
            }
        }
    } while (++i < n_baud_table);

    if (ofound == -1)
    {
        termios->c_cflag |= BOTHER;
    }

    if (ifound == -1 && (ibaud != obaud || ibinput))
    {
        termios->c_cflag |= (BOTHER << IBSHIFT);
    }
}

speed_t tty_termios_baud_rate(const termios_t *termios)
{
    // ENTER_FUNC("%p", termios);
    unsigned int cbaud;

    cbaud = termios->c_cflag & CBAUD;

    if (cbaud == BOTHER)
    {
        return termios->c_ospeed;
    }

    if (cbaud & CBAUDEX)
    {
        cbaud &= ~CBAUDEX;
        cbaud += 15;
    }
    return cbaud >= n_baud_table ? 0 : baud_table[cbaud];
}
