/*
 * File: base_errno.h
 * File Created: 09 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 09 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define EPERM           1      /* POSIX required: Operation not permitted */
#define ENOENT          2      /* POSIX required: No such file or directory */
#define ESRCH           3      /* POSIX required: Process does not exist */
#define EINTR           4      /* POSIX required: Interrupted system call */
#define EIO             5      /* POSIX required: Input/output error */
#define ENXIO           6      /* POSIX required: Device or address does not exist */
#define E2BIG           7      /* POSIX required: Argument list is too long */
#define ENOEXEC         8      /* POSIX required: Exec format error */
#define EBADF           9      /* POSIX required: Bad file descriptor */
#define ECHILD          10     /* POSIX required: Required child process is unavailable */
#define EAGAIN          11     /* POSIX required: Try again */
#define ENOMEM          12     /* POSIX required: POSIX name for out-of-memory */
#define EACCES          13     /* POSIX required: Permission denied */
#define EFAULT          14     /* POSIX required: Bad address */

#define EBUSY           16     /* POSIX required: Device or resource is busy */
#define EEXIST          17     /* POSIX required: File exists */
#define EXDEV           18     /* POSIX required: Cross-device link */
#define ENODEV          19     /* POSIX required: Device does not exist */
#define ENOTDIR         20     /* POSIX required: Not a directory */
#define EISDIR          21     /* POSIX required: Is a directory */
#define EINVAL          22     /* POSIX required: Invalid argument */
#define ENFILE          23     /* POSIX required: System file table is full */
#define EMFILE          24     /* POSIX required: Too many open files */
#define ENOTTY          25     /* POSIX required: Inappropriate device operation & Not a typewriter */
#define ETXTBSY         26     /* POSIX required: Text file is busy */
#define EFBIG           27     /* POSIX required: File is too large */
#define ENOSPC          28     /* POSIX required: No space left on device */
#define ESPIPE          29     /* POSIX required: Invalid seek on a pipe or stream */
#define EROFS           30     /* POSIX required: Read-only file system */
#define EMLINK          31     /* POSIX required: Too many links */
#define EPIPE           32     /* POSIX required: Broken pipe */
#define EDOM            33     /* POSIX required: Argument is outside the function domain */
#define ERANGE          34     /* POSIX required: Result is outside the representable range */
#define ENOMSG          35     /* POSIX required: Required message is unavailable */
#define EIDRM           36     /* POSIX required: Identifier was removed */
#define EFPOS           37     /* Invalid file position */

#define EDEADLK         45     /* POSIX required: Deadlock would occur */
#define ENOLCK          46     /* POSIX required: No locks are available */

#define ENOSTR          60     /* POSIX required: Object is not a stream */
#define ENODATA         61     /* POSIX required: No data is available */
#define ETIME           62     /* POSIX required: Timer expired */
#define ENOSR           63     /* POSIX required: No stream resources */

#define ENOLINK         67     /* POSIX required: Link has been severed */

#define EPROTO          71     /* POSIX required: Protocol error */

#define EMULTIHOP       77     /* POSIX required: Too many protocol hops */

#define EBADMSG         79     /* POSIX required: Message is invalid */

#define EFTYPE          79     /* Inappropriate file type or format */

#define ENOSYS          88     /* POSIX required: Function is not implemented */

#define ENOTEMPTY       90     /* POSIX required: Directory not empty */
#define ENAMETOOLONG    91     /* POSIX required: File name is too long */
#define ELOOP           92     /* POSIX required: Too many symbolic links */

#define EOPNOTSUPP      95     /* POSIX required: Operation is not supported */
#define EPFNOSUPPORT    96     /* Protocol family is unsupported */

#define ECONNRESET      104    /* POSIX required: Connection was reset */
#define ENOBUFS         105    /* POSIX required: No buffer space is available */
#define EAFNOSUPPORT    106    /* POSIX required: Address family is unsupported */
#define EPROTOTYPE      107    /* POSIX required: Protocol type is incorrect */
#define ENOTSOCK        108    /* POSIX required: File descriptor is not a socket */
#define ENOPROTOOPT     109    /* POSIX required: Protocol option is unavailable */
#define ESHUTDOWN       110    /* Endpoint has been shut down */
#define ECONNREFUSED    111    /* POSIX required: Connection was refused */
#define EADDRINUSE      112    /* POSIX required: Address is already in use */
#define ECONNABORTED    113    /* POSIX required: Connection was aborted */
#define ENETUNREACH     114    /* POSIX required: Network is unreachable */
#define ENETDOWN        115    /* POSIX required: Network is down */
#define ETIMEDOUT       116    /* POSIX required: Operation timed out */
#define EHOSTDOWN       117    /* Host is down */
#define EHOSTUNREACH    118    /* POSIX required: Host is unreachable */
#define EINPROGRESS     119    /* POSIX required: Operation is in progress */
#define EALREADY        120    /* POSIX required: Operation is already in progress */
#define EDESTADDRREQ    121    /* POSIX required: Destination address is required */
#define EMSGSIZE        122    /* POSIX required: Message is too large */
#define EPROTONOSUPPORT 123    /* POSIX required: Protocol is unsupported */
#define ESOCKTNOSUPPORT 124    /* Socket type is unsupported */
#define EADDRNOTAVAIL   125    /* POSIX required: Address is not available */
#define ENETRESET       126    /* POSIX required: Network connection was reset */
#define EISCONN         127    /* POSIX required: Socket is already connected */
#define ENOTCONN        128    /* POSIX required: Socket is not connected */
#define ETOOMANYREFS    129    /* Too many references: cannot splice */

#define EDQUOT          132    /* POSIX required: Disk quota is exceeded */
#define ESTALE          133    /* POSIX required: Stale file handle */
#define ENOTSUP         134    /* POSIX required: Feature is not supported */

#define EILSEQ          138    /* POSIX required: Invalid byte sequence */
#define EOVERFLOW       139    /* POSIX required: Value would overflow */
#define ECANCELED       140    /* POSIX required: Operation was canceled */
#define ENOTRECOVERABLE 141    /* POSIX required: State cannot be recovered */
#define EOWNERDEAD      142    /* POSIX required: Previous owner died */

#define EWOULDBLOCK     EAGAIN /* POSIX required: Operation would block + BSD compatibility */

// #define ENOTBLK         15      /* Block device required */
// #define ECHRNG          44      /* Channel number out of range */
// #define EL2NSYNC        45      /* Level 2 not synchronized */
// #define EL3HLT          46      /* Level 3 halted */
// #define EL3RST          47      /* Level 3 reset */
// #define ELNRNG          48      /* Link number out of range */
// #define EUNATCH         49      /* Protocol driver not attached */
// #define ENOCSI          50      /* No CSI structure available */
// #define EL2HLT          51      /* Level 2 halted */
// #define EBADE           52      /* Invalid exchange */
// #define EBADR           53      /* Invalid request descriptor */
// #define EXFULL          54      /* Exchange full */
// #define ENOANO          55      /* No anode */
// #define EBADRQC         56      /* Invalid request code */
// #define EBADSLT         57      /* Invalid slot */
// #define EBFONT          59      /* Bad font file format */
// #define ENONET          64      /* Machine is not on the network */
// #define ENOPKG          65      /* Package not installed */
// #define EREMOTE         66      /* Object is remote */
// #define EADV            68      /* Advertise error */
// #define ESRMNT          69      /* Srmount error */
// #define ECOMM           70      /* Communication error on send */
// #define EDOTDOT         73      /* RFS specific error */
// #define ENOTUNIQ        76      /* Name not unique on network */
// #define EBADFD          77      /* File descriptor in bad state */
// #define EREMCHG         78      /* Remote address changed */
// #define ELIBACC         79      /* Can not access a needed shared library */
// #define ELIBBAD         80      /* Accessing a corrupted shared library */
// #define ELIBSCN         81      /* .lib section in a.out corrupted */
// #define ELIBMAX         82      /* Attempting to link in too many shared libraries */
// #define ELIBEXEC        83      /* Cannot exec a shared library directly */
// #define ERESTART        85      /* Interrupted system call should be restarted */
// #define ESTRPIPE        86      /* Streams pipe error */
// #define EUSERS          87      /* Too many users */
// #define EUCLEAN         117     /* Structure needs cleaning */
// #define EFSCORRUPTED    EUCLEAN /* Filesystem is corrupted */
// #define ENOTNAM         118     /* Not a XENIX named type file */
// #define ENAVAIL         119     /* No XENIX semaphores available */
// #define EISNAM          120     /* Is a named type file */
// #define EREMOTEIO       121     /* Remote I/O error */
// #define ENOMEDIUM       123     /* No medium found */
// #define EMEDIUMTYPE     124     /* Wrong medium type */
// #define ENOKEY          126     /* Required key not available */
// #define EKEYEXPIRED     127     /* Key has expired */
// #define EKEYREVOKED     128     /* Key has been revoked */
// #define EKEYREJECTED    129     /* Key was rejected by service */
// #define ERFKILL         132 /* Operation not possible due to RF-kill */
// #define EHWPOISON       133 /* Memory page has hardware error */

