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

#define EPERM                                     1  /* Operation not permitted */
#define ENOENT                                    2  /* No such file or directory */
#define ESRCH                                     3  /* No such process */
#define EINTR                                     4  /* Interrupted system call */
#define EIO                                       5  /* I/O error */
#define ENXIO                                     6  /* No such device or address */
#define E2BIG                                     7  /* Argument list too long */
#define ENOEXEC                                   8  /* Exec format error */
#define EBADF                                     9  /* Bad file number */
#define ECHILD                                    10 /* No child processes */
#define EAGAIN                                    11 /* Try again */
#define ENOMEM                                    12 /* Out of memory */
#define EACCES                                    13 /* Permission denied */
#define EFAULT                                    14 /* Bad address */
#define ENOTBLK                                   15 /* Block device required */
#define EBUSY                                     16 /* Device or resource busy */
#define EEXIST                                    17 /* File exists */
#define EXDEV                                     18 /* Cross-device link */
#define ENODEV                                    19 /* No such device */
#define ENOTDIR                                   20 /* Not a directory */
#define EISDIR                                    21 /* Is a directory */
#define EINVAL                                    22 /* Invalid argument */
#define ENFILE                                    23 /* File table overflow */
#define EMFILE                                    24 /* Too many open files */
#define ENOTTY                                    25 /* Not a typewriter */
#define ETXTBSY                                   26 /* Text file busy */
#define EFBIG                                     27 /* File too large */
#define ENOSPC                                    28 /* No space left on device */
#define ESPIPE                                    29 /* Illegal seek */
#define EROFS                                     30 /* Read-only file system */
#define EMLINK                                    31 /* Too many links */
#define EPIPE                                     32 /* Broken pipe */
#define EDOM                                      33 /* Math argument out of domain of func */
#define ERANGE                                    34 /* Math result not representable */

#define EDEADLK      35 /* Resource deadlock would occur */
#define ENAMETOOLONG 36 /* File name too long */
#define ENOLCK       37 /* No record locks available */

/*
 * This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.
 */
#define ENOSYS          38      /* Invalid system call number */

#define ENOTEMPTY       39      /* Directory not empty */
#define ELOOP           40      /* Too many symbolic links encountered */
#define EWOULDBLOCK     EAGAIN  /* Operation would block */
#define ENOMSG          42      /* No message of desired type */
#define EIDRM           43      /* Identifier removed */
#define ECHRNG          44      /* Channel number out of range */
#define EL2NSYNC        45      /* Level 2 not synchronized */
#define EL3HLT          46      /* Level 3 halted */
#define EL3RST          47      /* Level 3 reset */
#define ELNRNG          48      /* Link number out of range */
#define EUNATCH         49      /* Protocol driver not attached */
#define ENOCSI          50      /* No CSI structure available */
#define EL2HLT          51      /* Level 2 halted */
#define EBADE           52      /* Invalid exchange */
#define EBADR           53      /* Invalid request descriptor */
#define EXFULL          54      /* Exchange full */
#define ENOANO          55      /* No anode */
#define EBADRQC         56      /* Invalid request code */
#define EBADSLT         57      /* Invalid slot */
#define EDEADLOCK       EDEADLK
#define EBFONT          59      /* Bad font file format */
#define ENOSTR          60      /* Device not a stream */
#define ENODATA         61      /* No data available */
#define ETIME           62      /* Timer expired */
#define ENOSR           63      /* Out of streams resources */
#define ENONET          64      /* Machine is not on the network */
#define ENOPKG          65      /* Package not installed */
#define EREMOTE         66      /* Object is remote */
#define ENOLINK         67      /* Link has been severed */
#define EADV            68      /* Advertise error */
#define ESRMNT          69      /* Srmount error */
#define ECOMM           70      /* Communication error on send */
#define EPROTO          71      /* Protocol error */
#define EMULTIHOP       72      /* Multihop attempted */
#define EDOTDOT         73      /* RFS specific error */
#define EBADMSG         74      /* Not a data message */
#define EFSBADCRC       EBADMSG /* Bad CRC detected */
#define EOVERFLOW       75      /* Value too large for defined data type */
#define ENOTUNIQ        76      /* Name not unique on network */
#define EBADFD          77      /* File descriptor in bad state */
#define EREMCHG         78      /* Remote address changed */
#define ELIBACC         79      /* Can not access a needed shared library */
#define ELIBBAD         80      /* Accessing a corrupted shared library */
#define ELIBSCN         81      /* .lib section in a.out corrupted */
#define ELIBMAX         82      /* Attempting to link in too many shared libraries */
#define ELIBEXEC        83      /* Cannot exec a shared library directly */
#define EILSEQ          84      /* Illegal byte sequence */
#define ERESTART        85      /* Interrupted system call should be restarted */
#define ESTRPIPE        86      /* Streams pipe error */
#define EUSERS          87      /* Too many users */
#define ENOTSOCK        88      /* Socket operation on non-socket */
#define EDESTADDRREQ    89      /* Destination address required */
#define EMSGSIZE        90      /* Message too long */
#define EPROTOTYPE      91      /* Protocol wrong type for socket */
#define ENOPROTOOPT     92      /* Protocol not available */
#define EPROTONOSUPPORT 93      /* Protocol not supported */
#define ESOCKTNOSUPPORT 94      /* Socket type not supported */
#define EOPNOTSUPP      95      /* Operation not supported on transport endpoint */
#define EPFNOSUPPORT    96      /* Protocol family not supported */
#define EAFNOSUPPORT    97      /* Address family not supported by protocol */
#define EADDRINUSE      98      /* Address already in use */
#define EADDRNOTAVAIL   99      /* Cannot assign requested address */
#define ENETDOWN        100     /* Network is down */
#define ENETUNREACH     101     /* Network is unreachable */
#define ENETRESET       102     /* Network dropped connection because of reset */
#define ECONNABORTED    103     /* Software caused connection abort */
#define ECONNRESET      104     /* Connection reset by peer */
#define ENOBUFS         105     /* No buffer space available */
#define EISCONN         106     /* Transport endpoint is already connected */
#define ENOTCONN        107     /* Transport endpoint is not connected */
#define ESHUTDOWN       108     /* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS    109     /* Too many references: cannot splice */
#define ETIMEDOUT       110     /* Connection timed out */
#define ECONNREFUSED    111     /* Connection refused */
#define EHOSTDOWN       112     /* Host is down */
#define EHOSTUNREACH    113     /* No route to host */
#define EALREADY        114     /* Operation already in progress */
#define EINPROGRESS     115     /* Operation now in progress */
#define ESTALE          116     /* Stale file handle */
#define EUCLEAN         117     /* Structure needs cleaning */
#define EFSCORRUPTED    EUCLEAN /* Filesystem is corrupted */
#define ENOTNAM         118     /* Not a XENIX named type file */
#define ENAVAIL         119     /* No XENIX semaphores available */
#define EISNAM          120     /* Is a named type file */
#define EREMOTEIO       121     /* Remote I/O error */
#define EDQUOT          122     /* Quota exceeded */

#define ENOMEDIUM       123     /* No medium found */
#define EMEDIUMTYPE     124     /* Wrong medium type */
#define ECANCELED       125     /* Operation Canceled */
#define ENOKEY          126     /* Required key not available */
#define EKEYEXPIRED     127     /* Key has expired */
#define EKEYREVOKED     128     /* Key has been revoked */
#define EKEYREJECTED    129     /* Key was rejected by service */

/* for robust mutexes */
#define EOWNERDEAD      130 /* Owner died */
#define ENOTRECOVERABLE 131 /* State not recoverable */

#define ERFKILL         132 /* Operation not possible due to RF-kill */

#define EHWPOISON       133 /* Memory page has hardware error */

/* E_TO_STRING(1, "EPERM")
E_TO_STRING(2, "ENOENT")
E_TO_STRING(3, "ESRCH")
E_TO_STRING(4, "EINTR")
E_TO_STRING(5, "EIO")
E_TO_STRING(6, "ENXIO")
E_TO_STRING(7, "E2BIG")
E_TO_STRING(8, "ENOEXEC")
E_TO_STRING(9, "EBADF")
E_TO_STRING(10, "ECHILD")
E_TO_STRING(11, "EAGAIN")
E_TO_STRING(12, "ENOMEM")
E_TO_STRING(13, "EACCES")
E_TO_STRING(14, "EFAULT")
E_TO_STRING(15, "ENOTBLK")
E_TO_STRING(16, "EBUSY")
E_TO_STRING(17, "EEXIST")
E_TO_STRING(18, "EXDEV")
E_TO_STRING(19, "ENODEV")
E_TO_STRING(20, "ENOTDIR")
E_TO_STRING(21, "EISDIR")
E_TO_STRING(22, "EINVAL")
E_TO_STRING(23, "ENFILE")
E_TO_STRING(24, "EMFILE")
E_TO_STRING(25, "ENOTTY")
E_TO_STRING(26, "ETXTBSY")
E_TO_STRING(27, "EFBIG")
E_TO_STRING(28, "ENOSPC")
E_TO_STRING(29, "ESPIPE")
E_TO_STRING(30, "EROFS")
E_TO_STRING(31, "EMLINK")
E_TO_STRING(32, "EPIPE")
E_TO_STRING(33, "EDOM")
E_TO_STRING(34, "ERANGE")
E_TO_STRING(35, "EDEADLK")
E_TO_STRING(36, "ENAMETOOLONG")
E_TO_STRING(37, "ENOLCK")
E_TO_STRING(38, "ENOSYS")
E_TO_STRING(39, "ENOTEMPTY")
E_TO_STRING(40, "ELOOP")
E_TO_STRING(11, "EWOULDBLOCK")
E_TO_STRING(42, "ENOMSG")
E_TO_STRING(43, "EIDRM")
E_TO_STRING(44, "ECHRNG")
E_TO_STRING(45, "EL2NSYNC")
E_TO_STRING(46, "EL3HLT")
E_TO_STRING(47, "EL3RST")
E_TO_STRING(48, "ELNRNG")
E_TO_STRING(49, "EUNATCH")
E_TO_STRING(50, "ENOCSI")
E_TO_STRING(51, "EL2HLT")
E_TO_STRING(52, "EBADE")
E_TO_STRING(53, "EBADR")
E_TO_STRING(54, "EXFULL")
E_TO_STRING(55, "ENOANO")
E_TO_STRING(56, "EBADRQC")
E_TO_STRING(57, "EBADSLT")
E_TO_STRING(35, "EDEADLOCK")
E_TO_STRING(59, "EBFONT")
E_TO_STRING(60, "ENOSTR")
E_TO_STRING(61, "ENODATA")
E_TO_STRING(62, "ETIME")
E_TO_STRING(63, "ENOSR")
E_TO_STRING(64, "ENONET")
E_TO_STRING(65, "ENOPKG")
E_TO_STRING(66, "EREMOTE")
E_TO_STRING(67, "ENOLINK")
E_TO_STRING(68, "EADV")
E_TO_STRING(69, "ESRMNT")
E_TO_STRING(70, "ECOMM")
E_TO_STRING(71, "EPROTO")
E_TO_STRING(72, "EMULTIHOP")
E_TO_STRING(73, "EDOTDOT")
E_TO_STRING(74, "EBADMSG")
E_TO_STRING(74, "EFSBADCRC")
E_TO_STRING(75, "EOVERFLOW")
E_TO_STRING(76, "ENOTUNIQ")
E_TO_STRING(77, "EBADFD")
E_TO_STRING(78, "EREMCHG")
E_TO_STRING(79, "ELIBACC")
E_TO_STRING(80, "ELIBBAD")
E_TO_STRING(81, "ELIBSCN")
E_TO_STRING(82, "ELIBMAX")
E_TO_STRING(83, "ELIBEXEC")
E_TO_STRING(84, "EILSEQ")
E_TO_STRING(85, "ERESTART")
E_TO_STRING(86, "ESTRPIPE")
E_TO_STRING(87, "EUSERS")
E_TO_STRING(88, "ENOTSOCK")
E_TO_STRING(89, "EDESTADDRREQ")
E_TO_STRING(90, "EMSGSIZE")
E_TO_STRING(91, "EPROTOTYPE")
E_TO_STRING(92, "ENOPROTOOPT")
E_TO_STRING(93, "EPROTONOSUPPORT")
E_TO_STRING(94, "ESOCKTNOSUPPORT")
E_TO_STRING(95, "EOPNOTSUPP")
E_TO_STRING(96, "EPFNOSUPPORT")
E_TO_STRING(97, "EAFNOSUPPORT")
E_TO_STRING(98, "EADDRINUSE")
E_TO_STRING(99, "EADDRNOTAVAIL")
E_TO_STRING(100, "ENETDOWN")
E_TO_STRING(101, "ENETUNREACH")
E_TO_STRING(102, "ENETRESET")
E_TO_STRING(103, "ECONNABORTED")
E_TO_STRING(104, "ECONNRESET")
E_TO_STRING(105, "ENOBUFS")
E_TO_STRING(106, "EISCONN")
E_TO_STRING(107, "ENOTCONN")
E_TO_STRING(108, "ESHUTDOWN")
E_TO_STRING(109, "ETOOMANYREFS")
E_TO_STRING(110, "ETIMEDOUT")
E_TO_STRING(111, "ECONNREFUSED")
E_TO_STRING(112, "EHOSTDOWN")
E_TO_STRING(113, "EHOSTUNREACH")
E_TO_STRING(114, "EALREADY")
E_TO_STRING(115, "EINPROGRESS")
E_TO_STRING(116, "ESTALE")
E_TO_STRING(117, "EUCLEAN")
E_TO_STRING(117, "EFSCORRUPTED")
E_TO_STRING(118, "ENOTNAM")
E_TO_STRING(119, "ENAVAIL")
E_TO_STRING(120, "EISNAM")
E_TO_STRING(121, "EREMOTEIO")
E_TO_STRING(122, "EDQUOT")
E_TO_STRING(123, "ENOMEDIUM")
E_TO_STRING(124, "EMEDIUMTYPE")
E_TO_STRING(125, "ECANCELED")
E_TO_STRING(126, "ENOKEY")
E_TO_STRING(127, "EKEYEXPIRED")
E_TO_STRING(128, "EKEYREVOKED")
E_TO_STRING(129, "EKEYREJECTED")
E_TO_STRING(130, "EOWNERDEAD")
E_TO_STRING(131, "ENOTRECOVERABLE")
E_TO_STRING(132, "ERFKILL")
E_TO_STRING(133, "EHWPOISON") */