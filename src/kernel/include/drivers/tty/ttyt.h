/*
 * File: ttyt.h
 * File Created: 28 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 28 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "termbits.h"

#include "dev/devicet.h"

#include "ringbuf/ringbuft.h"

#include <types.h>

typedef uint8_t cc_t;
typedef uint32_t speed_t;
typedef uint32_t tcflag_t;

// typedef struct tty_dev_ops
// {
//     void (*write_char)(device_t *dev, char c);
//     void (*write_buf)(device_t *dev, const uint8_t *buf, size_t n);
//     void (*flush_output)(device_t *dev);
//     ringbuf_index (*get_input_buf)(device_t *dev);
//     void (*flush_input)(device_t *dev);
// } tty_dev_ops_t;

typedef struct termios
{
    tcflag_t c_iflag; // input flags  (ICRNL, IXON etc.)
    tcflag_t c_oflag; // output flags (ONLCR etc.)
    tcflag_t c_cflag; // control flags (baud rate, stop bits - mostly serial)
    tcflag_t c_lflag; // local flags  (ICANON, ECHO, ISIG etc.)
    cc_t c_line;
    cc_t c_cc[NCCS];  // special characters (VINTR, VEOF, VERASE etc.)
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
} termios_t;

typedef struct winsize
{
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
} winsize_t;

typedef struct tty_ops tty_ops_t;
typedef struct tty_driver tty_driver_t;
typedef struct tty_dev tty_dev_t;
typedef struct tty_ldisc tty_ldisc_t;

typedef enum tty_struct_flags
{
    // TTY_THROTTLED,
    // TTY_IO_ERROR,
    // TTY_OTHER_CLOSED,
    // TTY_EXCLUSIVE,
    // TTY_DO_WRITE_WAKEUP,
    TTY_LDISC_OPEN,
    // TTY_PTY_LOCK,
    // TTY_NO_WRITE_SPLIT,
    // TTY_HUPPED,
    // TTY_HUPPING,
    TTY_LDISC_CHANGING,
    TTY_LDISC_HALTED,
} tty_struct_flags_t;

typedef struct tty_struct
{
    int id;
    int index;

    char name[64];
    uint32_t flags;

    tty_dev_t *tty_dev;
    tty_driver_t *driver;
    const tty_ops_t *ops;

    tty_ldisc_t *ldisc;
    void *disc_data;

    // input side
    // tty_line_buf_t line_buf; // canonical line discipline buffer

    // behaviour
    termios_t termios; // current terminal settings

    winsize_t winsize;

    struct
    {
        spinlock_t lock;
        bool stopped;
        bool tco_stopped;
    } flow;

    struct tty_struct *link;

    // ops
    // tty_dev_ops_t in_ops;
    // tty_dev_ops_t out_ops;


    // blocking read support
    // block_queue_t read_wait; // threads blocked in read() sleep here

    // process group (for signal delivery)
    pid_t fg_pgid; // foreground process group ID
} tty_struct_t;

#define DISABLED_CHAR     '\0'

#define INTR_CHAR(tty)    ((tty)->termios.c_cc[VINTR])
#define QUIT_CHAR(tty)    ((tty)->termios.c_cc[VQUIT])
#define ERASE_CHAR(tty)   ((tty)->termios.c_cc[VERASE])
#define KILL_CHAR(tty)    ((tty)->termios.c_cc[VKILL])
#define EOF_CHAR(tty)     ((tty)->termios.c_cc[VEOF])
#define TIME_CHAR(tty)    ((tty)->termios.c_cc[VTIME])
#define MIN_CHAR(tty)     ((tty)->termios.c_cc[VMIN])
#define SWTC_CHAR(tty)    ((tty)->termios.c_cc[VSWTC])
#define START_CHAR(tty)   ((tty)->termios.c_cc[VSTART])
#define STOP_CHAR(tty)    ((tty)->termios.c_cc[VSTOP])
#define SUSP_CHAR(tty)    ((tty)->termios.c_cc[VSUSP])
#define EOL_CHAR(tty)     ((tty)->termios.c_cc[VEOL])
#define REPRINT_CHAR(tty) ((tty)->termios.c_cc[VREPRINT])
#define DISCARD_CHAR(tty) ((tty)->termios.c_cc[VDISCARD])
#define WERASE_CHAR(tty)  ((tty)->termios.c_cc[VWERASE])
#define LNEXT_CHAR(tty)   ((tty)->termios.c_cc[VLNEXT])
#define EOL2_CHAR(tty)    ((tty)->termios.c_cc[VEOL2])

#define _I_FLAG(tty, f)   ((tty)->termios.c_iflag & (f))
#define _O_FLAG(tty, f)   ((tty)->termios.c_oflag & (f))
#define _C_FLAG(tty, f)   ((tty)->termios.c_cflag & (f))
#define _L_FLAG(tty, f)   ((tty)->termios.c_lflag & (f))

#define I_IGNBRK(tty)     _I_FLAG((tty), IGNBRK)
#define I_BRKINT(tty)     _I_FLAG((tty), BRKINT)
#define I_IGNPAR(tty)     _I_FLAG((tty), IGNPAR)
#define I_PARMRK(tty)     _I_FLAG((tty), PARMRK)
#define I_INPCK(tty)      _I_FLAG((tty), INPCK)
#define I_ISTRIP(tty)     _I_FLAG((tty), ISTRIP)
#define I_INLCR(tty)      _I_FLAG((tty), INLCR)
#define I_IGNCR(tty)      _I_FLAG((tty), IGNCR)
#define I_ICRNL(tty)      _I_FLAG((tty), ICRNL)
#define I_IUCLC(tty)      _I_FLAG((tty), IUCLC)
#define I_IXON(tty)       _I_FLAG((tty), IXON)
#define I_IXANY(tty)      _I_FLAG((tty), IXANY)
#define I_IXOFF(tty)      _I_FLAG((tty), IXOFF)
#define I_IMAXBEL(tty)    _I_FLAG((tty), IMAXBEL)
#define I_IUTF8(tty)      _I_FLAG((tty), IUTF8)

#define O_OPOST(tty)      _O_FLAG((tty), OPOST)
#define O_OLCUC(tty)      _O_FLAG((tty), OLCUC)
#define O_ONLCR(tty)      _O_FLAG((tty), ONLCR)
#define O_OCRNL(tty)      _O_FLAG((tty), OCRNL)
#define O_ONOCR(tty)      _O_FLAG((tty), ONOCR)
#define O_ONLRET(tty)     _O_FLAG((tty), ONLRET)
#define O_OFILL(tty)      _O_FLAG((tty), OFILL)
#define O_OFDEL(tty)      _O_FLAG((tty), OFDEL)
#define O_NLDLY(tty)      _O_FLAG((tty), NLDLY)
#define O_CRDLY(tty)      _O_FLAG((tty), CRDLY)
#define O_TABDLY(tty)     _O_FLAG((tty), TABDLY)
#define O_BSDLY(tty)      _O_FLAG((tty), BSDLY)
#define O_VTDLY(tty)      _O_FLAG((tty), VTDLY)
#define O_FFDLY(tty)      _O_FLAG((tty), FFDLY)

#define C_BAUD(tty)       _C_FLAG((tty), CBAUD)
#define C_CSIZE(tty)      _C_FLAG((tty), CSIZE)
#define C_CSTOPB(tty)     _C_FLAG((tty), CSTOPB)
#define C_CREAD(tty)      _C_FLAG((tty), CREAD)
#define C_PARENB(tty)     _C_FLAG((tty), PARENB)
#define C_PARODD(tty)     _C_FLAG((tty), PARODD)
#define C_HUPCL(tty)      _C_FLAG((tty), HUPCL)
#define C_CLOCAL(tty)     _C_FLAG((tty), CLOCAL)
#define C_CIBAUD(tty)     _C_FLAG((tty), CIBAUD)
#define C_CRTSCTS(tty)    _C_FLAG((tty), CRTSCTS)
#define C_CMSPAR(tty)     _C_FLAG((tty), CMSPAR)

#define L_ISIG(tty)       _L_FLAG((tty), ISIG)
#define L_ICANON(tty)     _L_FLAG((tty), ICANON)
#define L_XCASE(tty)      _L_FLAG((tty), XCASE)
#define L_ECHO(tty)       _L_FLAG((tty), ECHO)
#define L_ECHOE(tty)      _L_FLAG((tty), ECHOE)
#define L_ECHOK(tty)      _L_FLAG((tty), ECHOK)
#define L_ECHONL(tty)     _L_FLAG((tty), ECHONL)
#define L_NOFLSH(tty)     _L_FLAG((tty), NOFLSH)
#define L_TOSTOP(tty)     _L_FLAG((tty), TOSTOP)
#define L_ECHOCTL(tty)    _L_FLAG((tty), ECHOCTL)
#define L_ECHOPRT(tty)    _L_FLAG((tty), ECHOPRT)
#define L_ECHOKE(tty)     _L_FLAG((tty), ECHOKE)
#define L_FLUSHO(tty)     _L_FLAG((tty), FLUSHO)
#define L_PENDIN(tty)     _L_FLAG((tty), PENDIN)
#define L_IEXTEN(tty)     _L_FLAG((tty), IEXTEN)
#define L_EXTPROC(tty)    _L_FLAG((tty), EXTPROC)

typedef struct tty_dev
{
    void *driver_data;
    
    tty_struct_t *tty;
    tty_driver_t *driver;
    device_t dev;
} tty_dev_t;

#define to_tty_dev(n) container_of(n, tty_dev_t, dev)

typedef struct tty_action
{
    // TODO:
} tty_action_t;

typedef struct tty_ops
{
    status_t (*install)(tty_driver_t *driver, tty_struct_t *tty);
    status_t (*open)(tty_struct_t *tty, vfs_node_t *out_file);
    status_t (*close)(tty_struct_t *tty, vfs_node_t *file);
    status_t (*write)(tty_struct_t *tty, const uint8_t *buf, size_t count, ssize_t *size_out);
    status_t (*put_char)(tty_struct_t *tty, uint8_t ch);
    status_t (*flush_chars)(tty_struct_t *tty);
    status_t (*start)(tty_struct_t *tty);
    status_t (*write_room)(tty_struct_t *tty, size_t *size_out);
} tty_ops_t;

typedef enum tty_driver_flag
{
    TTY_DRIVER_INSTALLED = BIT(0),
    TTY_DRIVER_RESET_TERMIOS = BIT(1),
    TTY_DRIVER_REAL_RAW = BIT(2),
    TTY_DRIVER_DYNAMIC_DEV = BIT(3),
    TTY_DRIVER_DEVPTS_MEM = BIT(4),
    TTY_DRIVER_HARDWARE_BREAK = BIT(5),
    TTY_DRIVER_DYNAMIC_ALLOC = BIT(6),
    TTY_DRIVER_UNNUMBERED_NODE = BIT(7),
    TTY_DRIVER_NO_WORKQUEUE = BIT(8),
} tty_driver_flag_t;

typedef enum tty_driver_type
{
    TTY_DRIVER_TYPE_SYSTEM,
    TTY_DRIVER_TYPE_CONSOLE,
    TTY_DRIVER_TYPE_SERIAL,
    TTY_DRIVER_TYPE_PTY,
    TTY_DRIVER_TYPE_SCC,
    TTY_DRIVER_TYPE_SYSCONS,
} tty_driver_type_t;

typedef enum tty_driver_subtype
{
    SYSTEM_TYPE_TTY = 1,
    SYSTEM_TYPE_CONSOLE,
    SYSTEM_TYPE_SYSCONS,
    SYSTEM_TYPE_SYSPTMX,

    PTY_TYPE_MASTER = 1,
    PTY_TYPE_SLAVE,

    SERIAL_TYPE_NORMAL = 1,
} tty_driver_subtype_t;

typedef struct tty_driver
{
    LIST_ENTRY_THIS();
    driver_t driver;
    const char *driver_name;
    const char *name;
    int name_base;
    int device;
    int major;
    int minor_start;

    uint32_t num;
    tty_driver_type_t type;
    tty_driver_subtype_t subtype;
    termios_t init_termios;
    uint32_64_t flags;

    tty_struct_t **ttys;
    termios_t **termios;

    const tty_ops_t *ops;
} tty_driver_t;
