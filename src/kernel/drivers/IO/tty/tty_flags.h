/*
 * File: tty_flags.h
 * File Created: 08 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 08 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include <util/binary.h>

// c_iflag
#define ICRNL BIT(0) // map \r to \n on input

// c_cflag
#define CBAUD    0x0000100F

#define B0       0x00000000 /* hang up */
#define B50      0x00000001
#define B75      0x00000002
#define B110     0x00000003
#define B134     0x00000004
#define B150     0x00000005
#define B200     0x00000006
#define B300     0x00000007
#define B600     0x00000008
#define B1200    0x00000009
#define B1800    0x0000000A
#define B2400    0x0000000B
#define B4800    0x0000000C
#define B9600    0x0000000D
#define B19200   0x0000000E
#define B38400   0x0000000F
#define EXTA     B19200
#define EXTB     B38400

#define CSIZE    0x00000030 /* Number of bits per byte (mask) */
#define CS5      0x00000000 /* 5 bits per byte */
#define CS6      0x00000010 /* 6 bits per byte */
#define CS7      0x00000020 /* 7 bits per byte */
#define CS8      0x00000030 /* 8 bits per byte */
#define CSTOPB   0x00000040 /* Two stop bits instead of one */
#define CREAD    0x00000080 /* Enable receiver */
#define PARENB   0x00000100 /* Parity enable */
#define PARODD   0x00000200 /* Odd parity instead of even */
#define HUPCL    0x00000400 /* Hang up on last close */
#define CLOCAL   0x00000800 /* Ignore modem status lines */

#define CBAUDEX  0x00001000
#define BOTHER   0x00001000
#define B57600   0x00001001
#define B115200  0x00001002
#define B230400  0x00001003
#define B460800  0x00001004
#define B500000  0x00001005
#define B576000  0x00001006
#define B921600  0x00001007
#define B1000000 0x00001008
#define B1152000 0x00001009
#define B1500000 0x0000100A
#define B2000000 0x0000100B
#define B2500000 0x0000100C
#define B3000000 0x0000100D
#define B3500000 0x0000100E
#define B4000000 0x0000100F

#define CIBAUD   0x100F0000 /* input baud rate */

#define IBSHIFT  16         /* Shift from CBAUD to CIBAUD */

// c_oflag
#define OLCUC BIT(1) // Map lower case to upper case on output.
#define ONLCR BIT(2) // Map NL to CR-NL on output.

// c_lflag
#define ICANON BIT(0) // canonical mode (line buffered)
#define ECHO   BIT(1) // echo input back to output
#define ISIG   BIT(2) // enable signal generation (SIGINT etc.)
#define ECHOE  BIT(3) // echo backspace as \b \b

// c_cc indices
#define VINTR  0 // interrupt char   (default Ctrl+C = 0x03)
#define VEOF   1 // EOF char         (default Ctrl+D = 0x04)
#define VERASE 2 // erase char       (default Backspace)
#define VSUSP  3 // suspend char     (default Ctrl+Z)
#define VKILL  4 // kill line char   (default Ctrl+U)
