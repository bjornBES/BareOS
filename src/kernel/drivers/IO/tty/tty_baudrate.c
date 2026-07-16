/*
 * File: tty_baudrate.c
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 10 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "tty_types.h"
#include "tty_flags.h"

#include <util/arrays.h>

static const speed_t baud_table[] = {
    0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
    4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
#ifdef __sparc__
    76800, 153600, 307200, 614400, 921600, 500000, 576000,
    1000000, 1152000, 1500000, 2000000
#else
    500000, 576000, 921600, 1000000, 1152000, 1500000, 2000000,
    2500000, 3000000, 3500000, 4000000
#endif
};
static const int baud_bits[] = {
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400,
    B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800,
#ifdef __sparc__
    B76800, B153600, B307200, B614400, B921600, B500000, B576000,
    B1000000, B1152000, B1500000, B2000000
#else
    B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000,
    B2500000, B3000000, B3500000, B4000000
#endif
};

#define n_baud_table ARRAY_SIZE(baud_table)

speed_t tty_baudrate_baud_rate(const termios_t *termios)
{
    unsigned int cbaud;

    cbaud = termios->c_cflag & CBAUD;

    /* Magic token for arbitrary speed via c_ispeed/c_ospeed */
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

speed_t tty_baudrate_input_baud_rate(const termios_t *termios)
{
    unsigned int cbaud = (termios->c_cflag >> IBSHIFT) & CBAUD;

    if (cbaud == B0)
    {
        return tty_baudrate_baud_rate(termios);
    }

    /* Magic token for arbitrary speed via c_ispeed */
    if (cbaud == BOTHER)
    {
        return termios->c_ispeed;
    }

    if (cbaud & CBAUDEX)
    {
        cbaud &= ~CBAUDEX;
        cbaud += 15;
    }
    return cbaud >= n_baud_table ? 0 : baud_table[cbaud];
}

void tty_baudrate_encode_baud_rate(termios_t *termios, speed_t ibaud, speed_t obaud)
{
    speed_t final_input_baud = -1;
    speed_t final_output_baud = -1;
    int input_close = ibaud / 50;
    int output_close = obaud / 50;
    // int exact_input = 0;
    // int exact_output = 0;

    termios->c_ispeed = ibaud;
    termios->c_ospeed = obaud;

    cc_t input_termios_input = (termios->c_cflag >> IBSHIFT) & CBAUD;
    cc_t output_termios_input = termios->c_cflag & CBAUD;

    if (input_termios_input != B0)
    {
        // exact_input = 1;
    }
 
    if (output_termios_input == BOTHER)
    {
        output_close = 0;
        if (!input_termios_input)
        {
            input_close = 0;
        }
    }
    if (input_termios_input == BOTHER)
    {
        input_close = 0;
    }

    termios->c_cflag &= ~CBAUD;
    termios->c_cflag &= ~(CBAUD << IBSHIFT);

    for (size_t i = 0; i < n_baud_table; i++)
    {
        if (final_input_baud == -1 && (baud_bits[i] == ibaud || (ibaud - input_close <= baud_table[i] && ibaud + input_close >= baud_table[i])))
        {
            final_input_baud = baud_bits[i];
        }
        if (final_output_baud == -1 && (baud_bits[i] == obaud || (obaud - output_close <= baud_table[i] && obaud + output_close >= baud_table[i])))
        {
            final_output_baud = baud_bits[i];
        }
    }

    termios->c_cflag |= final_output_baud;
    termios->c_cflag |= final_input_baud << IBSHIFT;
}
