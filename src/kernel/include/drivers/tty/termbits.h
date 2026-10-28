/*
 * File: termbits.h
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// c_iflag
#define IGNBRK   BIT(0)  /* Ignore break condition */
#define BRKINT   BIT(1)  /* Signal interrupt on break */
#define IGNPAR   BIT(2)  /* Ignore characters with parity errors */
#define PARMRK   BIT(3)  /* Mark parity and framing errors */
#define INPCK    BIT(4)  /* Enable input parity check */
#define ISTRIP   BIT(5)  /* Strip 8th bit off characters */
#define INLCR    BIT(6)  /* Map NL to CR on input */
#define IGNCR    BIT(7)  /* Ignore CR */
#define ICRNL    BIT(8)  /* Map CR to NL on input */
#define IXANY    BIT(11) /* Any character will restart after stop */
#define IUCLC    BIT(17)
#define IXON     BIT(18)
#define IXOFF    BIT(20)
#define IMAXBEL  BIT(21)
#define IUTF8    BIT(22)

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

#define CBAUDEX  (cc_t)0x00001000
#define BOTHER   (cc_t)0x00001000
#define B57600   (cc_t)0x00001001
#define B115200  (cc_t)0x00001002
#define B230400  (cc_t)0x00001003
#define B460800  (cc_t)0x00001004
#define B500000  (cc_t)0x00001005
#define B576000  (cc_t)0x00001006
#define B921600  (cc_t)0x00001007
#define B1000000 (cc_t)0x00001008
#define B1152000 (cc_t)0x00001009
#define B1500000 (cc_t)0x0000100A
#define B2000000 (cc_t)0x0000100B
#define B2500000 (cc_t)0x0000100C
#define B3000000 (cc_t)0x0000100D
#define B3500000 (cc_t)0x0000100E
#define B4000000 (cc_t)0x0000100F

#define CIBAUD   0x100F0000 /* input baud rate */

#define IBSHIFT  16         /* Shift from CBAUD to CIBAUD */

// c_oflag
#define OPOST    BIT(0) // Perform output processing
#define OLCUC    BIT(1) // Map lower case to upper case on output.
#define ONLCR    BIT(2) // Map NL to CR-NL on output.

// c_lflag
#define ISIG     BIT(0) // enable signal generation (SIGINT etc.)
#define ICANON   BIT(1) // canonical mode (line buffered)
#define ECHO     BIT(3) // echo input back to output
#define ECHOE    BIT(4) // echo backspace as \b \b
#define ECHOK    BIT(5)
#define ECHONL	BIT(6)
#define NOFLSH	BIT(7)
#define TOSTOP	BIT(8)
#define ECHOCTL	BIT(9)
#define ECHOPRT	BIT(10)
#define ECHOKE	BIT(11)
#define FLUSHO	BIT(12)
#define PENDIN	BIT(14)
#define IEXTEN	BIT(15)
#define EXTPROC	BIT(16)

// c_cc indices
#define VINTR    0 // interrupt char   (default Ctrl+C = 0x03)
#define VQUIT    1
#define VERASE   2 // erase char       (default Backspace)
#define VKILL    3 // kill line char   (default Ctrl+U)
#define VEOF     4 // EOF char         (default Ctrl+D = 0x04)
#define VTIME    5
#define VMIN     6
#define VSWTC    7
#define VSTART   8
#define VSTOP    9
#define VSUSP    10 // suspend char     (default Ctrl+Z)
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16

#define NCCS     19 // for now
