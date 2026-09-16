/*
 * File: kerrno.h
 * File Created: 15 Sep 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 15 Sep 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "debug/debug.h"


// Design from https://github.com/haiku/haiku/blob/master/headers/os/support/Errors.h

#define KERRNO_RETURN(number, ...)                                        \
    {                                                                    \
        log_err(MODULE, "func %s outputting %s", __FUNCTION__, #number); \
        logfl(MODULE, LVL_INFO, __VA_ARGS__);                            \
        return number;                                                   \
    }

#define KERRNO_NO_RETURN(number, ...)                                     \
    {                                                                    \
        log_err(MODULE, "func %s outputting %s", __FUNCTION__, #number); \
        logfl(MODULE, LVL_INFO, __VA_ARGS__);                            \
    }

// most (if not all) of the comments in this file are made by an AI, this will be changed in later versions of the kernel.

/* Error baselines */
#define KERRNO_GENERAL_ERROR_BASE                0                                    /* General errors */
#define KERRNO_OS_ERROR_BASE                     (KERRNO_GENERAL_ERROR_BASE + 0x1000) /* Kernel Kit errors */
#define KERRNO_APP_ERROR_BASE                    (KERRNO_GENERAL_ERROR_BASE + 0x2000) /* Application Kit errors */
#define KERRNO_INTERFACE_ERROR_BASE              (KERRNO_GENERAL_ERROR_BASE + 0x3000) /* Interface Kit errors */
#define KERRNO_MEDIA_ERROR_BASE                  (KERRNO_GENERAL_ERROR_BASE + 0x4000) /* Media Kit errors: 0x4000-0x41ff */
#define KERRNO_TRANSLATION_ERROR_BASE            (KERRNO_GENERAL_ERROR_BASE + 0x4800) /* Translation Kit errors: 0x4800-0x48ff */
#define KERRNO_MIDI_ERROR_BASE                   (KERRNO_GENERAL_ERROR_BASE + 0x5000) /* MIDI Kit errors */
#define KERRNO_STORAGE_ERROR_BASE                (KERRNO_GENERAL_ERROR_BASE + 0x6000) /* Storage Kit and file-system errors */
#define KERRNO_POSIX_ERROR_BASE                  (KERRNO_GENERAL_ERROR_BASE + 0x7000) /* POSIX-compatible errors */
#define KERRNO_MAIL_ERROR_BASE                   (KERRNO_GENERAL_ERROR_BASE + 0x8000) /* Mail Kit errors */
#define KERRNO_PRINT_ERROR_BASE                  (KERRNO_GENERAL_ERROR_BASE + 0x9000) /* Printing errors */
#define KERRNO_DEVICE_ERROR_BASE                 (KERRNO_GENERAL_ERROR_BASE + 0xA000) /* Device Kit errors */

/* Developer-defined errors start at (KERRNO_ERRORS_END+1) */
#define KERRNO_ERRORS_END                        (KERRNO_GENERAL_ERROR_BASE + 0xFFFF) /* Last reserved system error value */

/* General Errors */
#define KERRNO_OOM                               (KERRNO_GENERAL_ERROR_BASE + 1)  /* Out of memory */
#define KERRNO_IO_ERROR                          (KERRNO_GENERAL_ERROR_BASE + 2)  /* Input/output failure */
#define KERRNO_PERMISSION_DENIED                 (KERRNO_GENERAL_ERROR_BASE + 3)  /* Operation is not permitted */
#define KERRNO_BAD_INDEX                         (KERRNO_GENERAL_ERROR_BASE + 4)  /* Index is outside the valid range */
#define KERRNO_BAD_TYPE                          (KERRNO_GENERAL_ERROR_BASE + 5)  /* Value has the wrong type */
#define KERRNO_BAD_VALUE                         (KERRNO_GENERAL_ERROR_BASE + 6)  /* Value is invalid */
#define KERRNO_MISMATCHED_VALUES                 (KERRNO_GENERAL_ERROR_BASE + 7)  /* Related values do not match */
#define KERRNO_NAME_NOT_FOUND                    (KERRNO_GENERAL_ERROR_BASE + 8)  /* Requested name was not found */
#define KERRNO_NAME_IN_USE                       (KERRNO_GENERAL_ERROR_BASE + 9)  /* Name is already in use */
#define KERRNO_TIMED_OUT                         (KERRNO_GENERAL_ERROR_BASE + 10) /* Operation exceeded its time limit */
#define KERRNO_INTERRUPTED                       (KERRNO_GENERAL_ERROR_BASE + 11) /* Operation was interrupted */
#define KERRNO_WOULD_BLOCK                       (KERRNO_GENERAL_ERROR_BASE + 12) /* Operation would block */
#define KERRNO_CANCELED                          (KERRNO_GENERAL_ERROR_BASE + 13) /* Operation was canceled */
#define KERRNO_NO_INIT                           (KERRNO_GENERAL_ERROR_BASE + 14) /* Required initialization is unavailable */
#define KERRNO_NOT_INITIALIZED                   (KERRNO_GENERAL_ERROR_BASE + 15) /* Component has not been initialized */
#define KERRNO_BUSY                              (KERRNO_GENERAL_ERROR_BASE + 16) /* Resource is busy */
#define KERRNO_NOT_ALLOWED                       (KERRNO_GENERAL_ERROR_BASE + 17) /* Operation is not allowed */
#define KERRNO_BAD_DATA                          (KERRNO_GENERAL_ERROR_BASE + 18) /* Data is malformed or invalid */
#define KERRNO_DONT_DO_THAT                      (KERRNO_GENERAL_ERROR_BASE + 19) /* Operation is explicitly unsupported */

#define KERRNO_SUCCESSES                         ((int)0)

/* Kernel Kit Errors */
#define KERRNO_BAD_SEM_ID                        (KERRNO_OS_ERROR_BASE + 1)     /* Semaphore ID is invalid */
#define KERRNO_NO_MORE_SEMS                      (KERRNO_OS_ERROR_BASE + 2)     /* No semaphore IDs remain */

#define KERRNO_BAD_THREAD_ID                     (KERRNO_OS_ERROR_BASE + 0x100) /* Thread ID is invalid */
#define KERRNO_NO_MORE_THREADS                   (KERRNO_OS_ERROR_BASE + 0x101) /* No thread IDs remain */
#define KERRNO_BAD_THREAD_STATE                  (KERRNO_OS_ERROR_BASE + 0x102) /* Thread state is invalid */
#define KERRNO_BAD_TEAM_ID                       (KERRNO_OS_ERROR_BASE + 0x103) /* Team ID is invalid */
#define KERRNO_NO_MORE_TEAMS                     (KERRNO_OS_ERROR_BASE + 0x104) /* No team IDs remain */

#define KERRNO_BAD_PORT_ID                       (KERRNO_OS_ERROR_BASE + 0x200) /* Port ID is invalid */
#define KERRNO_NO_MORE_PORTS                     (KERRNO_OS_ERROR_BASE + 0x201) /* No port IDs remain */

#define KERRNO_BAD_IMAGE_ID                      (KERRNO_OS_ERROR_BASE + 0x300) /* Image ID is invalid */
#define KERRNO_BAD_ADDRESS                       (KERRNO_OS_ERROR_BASE + 0x301) /* Address is invalid */
#define KERRNO_NOT_AN_EXECUTABLE                 (KERRNO_OS_ERROR_BASE + 0x302) /* File is not executable */
#define KERRNO_MISSING_LIBRARY                   (KERRNO_OS_ERROR_BASE + 0x303) /* Required library is missing */
#define KERRNO_MISSING_SYMBOL                    (KERRNO_OS_ERROR_BASE + 0x304) /* Required symbol is missing */
#define KERRNO_UNKNOWN_EXECUTABLE                (KERRNO_OS_ERROR_BASE + 0x305) /* Executable format is unknown */
#define KERRNO_LEGACY_EXECUTABLE                 (KERRNO_OS_ERROR_BASE + 0x306) /* Executable uses a legacy format */

#define KERRNO_DEBUGGER_ALREADY_INSTALLED        (KERRNO_OS_ERROR_BASE + 0x400) /* A debugger is already installed */

/* Application Kit Errors */
#define KERRNO_BAD_REPLY                         (KERRNO_APP_ERROR_BASE + 1)  /* Reply is invalid */
#define KERRNO_DUPLICATE_REPLY                   (KERRNO_APP_ERROR_BASE + 2)  /* Reply has already been handled */
#define KERRNO_MESSAGE_TO_SELF                   (KERRNO_APP_ERROR_BASE + 3)  /* Message was sent to its own sender */
#define KERRNO_BAD_HANDLER                       (KERRNO_APP_ERROR_BASE + 4)  /* Handler is invalid */
#define KERRNO_ALREADY_RUNNING                   (KERRNO_APP_ERROR_BASE + 5)  /* Application is already running */
#define KERRNO_LAUNCH_FAILED                     (KERRNO_APP_ERROR_BASE + 6)  /* Application launch failed */
#define KERRNO_AMBIGUOUS_APP_LAUNCH              (KERRNO_APP_ERROR_BASE + 7)  /* More than one application can handle the launch */
#define KERRNO_UNKNOWN_MIME_TYPE                 (KERRNO_APP_ERROR_BASE + 8)  /* MIME type is unknown */
#define KERRNO_BAD_SCRIPT_SYNTAX                 (KERRNO_APP_ERROR_BASE + 9)  /* Application script syntax is invalid */
#define KERRNO_LAUNCH_FAILED_NO_RESOLVE_LINK     (KERRNO_APP_ERROR_BASE + 10) /* Launch failed while resolving a link */
#define KERRNO_LAUNCH_FAILED_EXECUTABLE          (KERRNO_APP_ERROR_BASE + 11) /* Executable could not be launched */
#define KERRNO_LAUNCH_FAILED_APP_NOT_FOUND       (KERRNO_APP_ERROR_BASE + 12) /* Application was not found */
#define KERRNO_LAUNCH_FAILED_APP_IN_TRASH        (KERRNO_APP_ERROR_BASE + 13) /* Application is in the trash */
#define KERRNO_LAUNCH_FAILED_NO_PREFERRED_APP    (KERRNO_APP_ERROR_BASE + 14) /* No preferred application is configured */
#define KERRNO_LAUNCH_FAILED_FILES_APP_NOT_FOUND (KERRNO_APP_ERROR_BASE + 15) /* No application handles the selected files */
#define KERRNO_BAD_MIME_SNIFFER_RULE             (KERRNO_APP_ERROR_BASE + 16) /* MIME sniffer rule is invalid */
#define KERRNO_NOT_A_MESSAGE                     (KERRNO_APP_ERROR_BASE + 17) /* Object is not a message */
#define KERRNO_SHUTDOWN_CANCELLED                (KERRNO_APP_ERROR_BASE + 18) /* Shutdown was canceled */
#define KERRNO_SHUTTING_DOWN                     (KERRNO_APP_ERROR_BASE + 19) /* System is shutting down */

/* Storage Kit/File System Errors */
#define KERRNO_FILE_ERROR                        (KERRNO_STORAGE_ERROR_BASE + 1)  /* Generic file error */
#define KERRNO_FILE_EXISTS                       (KERRNO_STORAGE_ERROR_BASE + 2)  /* File or directory already exists */
#define KERRNO_ENTRY_NOT_FOUND                   (KERRNO_STORAGE_ERROR_BASE + 3)  /* Directory entry was not found */
#define KERRNO_NAME_TOO_LONG                     (KERRNO_STORAGE_ERROR_BASE + 4)  /* File or directory name is too long */
#define KERRNO_NOT_A_DIRECTORY                   (KERRNO_STORAGE_ERROR_BASE + 5)  /* Path component is not a directory */
#define KERRNO_DIRECTORY_NOT_EMPTY               (KERRNO_STORAGE_ERROR_BASE + 6)  /* Directory contains entries */
#define KERRNO_DEVICE_FULL                       (KERRNO_STORAGE_ERROR_BASE + 7)  /* Device has no free space */
#define KERRNO_READ_ONLY_DEVICE                  (KERRNO_STORAGE_ERROR_BASE + 8)  /* Device cannot be written */
#define KERRNO_IS_A_DIRECTORY                    (KERRNO_STORAGE_ERROR_BASE + 9)  /* Path refers to a directory */
#define KERRNO_NO_MORE_FDS                       (KERRNO_STORAGE_ERROR_BASE + 10) /* No file descriptors remain */
#define KERRNO_CROSS_DEVICE_LINK                 (KERRNO_STORAGE_ERROR_BASE + 11) /* Link crosses devices */
#define KERRNO_LINK_LIMIT                        (KERRNO_STORAGE_ERROR_BASE + 12) /* Link traversal limit was reached */
#define KERRNO_BUSTED_PIPE                       (KERRNO_STORAGE_ERROR_BASE + 13) /* Pipe has no readers */
#define KERRNO_UNSUPPORTED                       (KERRNO_STORAGE_ERROR_BASE + 14) /* Operation is unsupported */
#define KERRNO_PARTITION_TOO_SMALL               (KERRNO_STORAGE_ERROR_BASE + 15) /* Partition is too small */
#define KERRNO_PARTIAL_READ                      (KERRNO_STORAGE_ERROR_BASE + 16) /* Read completed only partially */
#define KERRNO_PARTIAL_WRITE                     (KERRNO_STORAGE_ERROR_BASE + 17) /* Write completed only partially */

/* POSIX Errors */
#define KERRNO_TO_POSIX_ERROR(error)             ((error))                                          /* Convert a positive kernel error to a negative POSIX-style result */
#define KERRNO_FROM_POSIX_ERROR(error)           ((error))                                          /* Convert a negative POSIX-style result to a positive kernel error */

#define KERRNO_POSIX_ENOMEM                      KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 0)  /* Memory allocation failed */
#define E2BIG                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 1)  /* Argument list is too long */
#define ECHILD                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 2)  /* Required child process is unavailable */
#define EDEADLK                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 3)  /* Deadlock would occur */
#define EFBIG                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 4)  /* File is too large */
#define EMLINK                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 5)  /* Too many links */
#define ENFILE                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 6)  /* System file table is full */
#define ENODEV                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 7)  /* Device does not exist */
#define ENOLCK                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 8)  /* No locks are available */
#define ENOSYS                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 9)  /* Function is not implemented */
#define ENOTTY                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 10) /* Inappropriate device operation */
#define ENXIO                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 11) /* Device or address does not exist */
#define ESPIPE                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 12) /* Invalid seek on a pipe or stream */
#define ESRCH                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 13) /* Process does not exist */
#define EFPOS                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 14) /* Invalid file position */
#define ESIGPARM                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 15) /* Invalid signal parameter */
#define EDOM                                     KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 16) /* Argument is outside the function domain */
#define ERANGE                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 17) /* Result is outside the representable range */
#define EPROTOTYPE                               KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 18) /* Protocol type is incorrect */
#define EPROTONOSUPPORT                          KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 19) /* Protocol is unsupported */
#define EPFNOSUPPORT                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 20) /* Protocol family is unsupported */
#define EAFNOSUPPORT                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 21) /* Address family is unsupported */
#define EADDRINUSE                               KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 22) /* Address is already in use */
#define EADDRNOTAVAIL                            KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 23) /* Address is not available */
#define ENETDOWN                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 24) /* Network is down */
#define ENETUNREACH                              KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 25) /* Network is unreachable */
#define ENETRESET                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 26) /* Network connection was reset */
#define ECONNABORTED                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 27) /* Connection was aborted */
#define ECONNRESET                               KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 28) /* Connection was reset */
#define EISCONN                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 29) /* Socket is already connected */
#define ENOTCONN                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 30) /* Socket is not connected */
#define ESHUTDOWN                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 31) /* Endpoint has been shut down */
#define ECONNREFUSED                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 32) /* Connection was refused */
#define EHOSTUNREACH                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 33) /* Host is unreachable */
#define ENOPROTOOPT                              KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 34) /* Protocol option is unavailable */
#define ENOBUFS                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 35) /* No buffer space is available */
#define EINPROGRESS                              KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 36) /* Operation is in progress */
#define EALREADY                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 37) /* Operation is already in progress */
#define EILSEQ                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 38) /* Invalid byte sequence */
#define ENOMSG                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 39) /* Required message is unavailable */
#define ESTALE                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 40) /* Stale file handle */
#define EOVERFLOW                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 41) /* Value would overflow */
#define EMSGSIZE                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 42) /* Message is too large */
#define EOPNOTSUPP                               KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 43) /* Operation is not supported */
#define ENOTSOCK                                 KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 44) /* File descriptor is not a socket */
#define EHOSTDOWN                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 45) /* Host is down */
#define EBADMSG                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 46) /* Message is invalid */
#define ECANCELED                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 47) /* Operation was canceled */
#define EDESTADDRREQ                             KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 48) /* Destination address is required */
#define EDQUOT                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 49) /* Disk quota is exceeded */
#define EIDRM                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 50) /* Identifier was removed */
#define EMULTIHOP                                KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 51) /* Too many protocol hops */
#define ENODATA                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 52) /* No data is available */
#define ENOLINK                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 53) /* Link has been severed */
#define ENOSR                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 54) /* No stream resources */
#define ENOSTR                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 55) /* Object is not a stream */
#define ENOTSUP                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 56) /* Feature is not supported */
#define EPROTO                                   KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 57) /* Protocol error */
#define ETIME                                    KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 58) /* Timer expired */
#define ETXTBSY                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 59) /* Text file is busy */
#define ENOATTR                                  KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 60) /* Attribute is missing */
#define ENOTRECOVERABLE                          KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 61) /* State cannot be recovered */
#define EOWNERDEAD                               KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 62) /* Previous owner died */
#define ESOCKTNOSUPPORT                          KERRNO_TO_POSIX_ERROR(KERRNO_POSIX_ERROR_BASE + 63) /* Socket type is unsupported */

#define ENOMEM                                   KERRNO_POSIX_ENOMEM                                 /* POSIX name for out-of-memory */

/* POSIX errors that can be mapped to BeOS error codes */
#define EACCES                                   KERRNO_TO_POSIX_ERROR(KERRNO_PERMISSION_DENIED)   /* Permission denied */
#define EINTR                                    KERRNO_TO_POSIX_ERROR(KERRNO_INTERRUPTED)         /* Interrupted system call */
#define EIO                                      KERRNO_TO_POSIX_ERROR(KERRNO_IO_ERROR)            /* Input/output error */
#define EBUSY                                    KERRNO_TO_POSIX_ERROR(KERRNO_BUSY)                /* Device or resource is busy */
#define EFAULT                                   KERRNO_TO_POSIX_ERROR(KERRNO_BAD_ADDRESS)         /* Bad address */
#define ETIMEDOUT                                KERRNO_TO_POSIX_ERROR(KERRNO_TIMED_OUT)           /* Operation timed out */
#define EAGAIN                                   KERRNO_TO_POSIX_ERROR(KERRNO_WOULD_BLOCK)         /* SysV compatibility */
#define EWOULDBLOCK                              KERRNO_TO_POSIX_ERROR(KERRNO_WOULD_BLOCK)         /* BSD compatibility */
#define EBADF                                    KERRNO_TO_POSIX_ERROR(KERRNO_FILE_ERROR)          /* Bad file descriptor */
#define EEXIST                                   KERRNO_TO_POSIX_ERROR(KERRNO_FILE_EXISTS)         /* File exists */
#define EINVAL                                   KERRNO_TO_POSIX_ERROR(KERRNO_BAD_VALUE)           /* Invalid argument */
#define ENAMETOOLONG                             KERRNO_TO_POSIX_ERROR(KERRNO_NAME_TOO_LONG)       /* File name is too long */
#define ENOENT                                   KERRNO_TO_POSIX_ERROR(KERRNO_ENTRY_NOT_FOUND)     /* No such file or directory */
#define EPERM                                    KERRNO_TO_POSIX_ERROR(KERRNO_NOT_ALLOWED)         /* Operation not permitted */
#define ENOTDIR                                  KERRNO_TO_POSIX_ERROR(KERRNO_NOT_A_DIRECTORY)     /* Not a directory */
#define EISDIR                                   KERRNO_TO_POSIX_ERROR(KERRNO_IS_A_DIRECTORY)      /* Is a directory */
#define ENOTEMPTY                                KERRNO_TO_POSIX_ERROR(KERRNO_DIRECTORY_NOT_EMPTY) /* Directory not empty */
#define ENOSPC                                   KERRNO_TO_POSIX_ERROR(KERRNO_DEVICE_FULL)         /* No space left on device */
#define EROFS                                    KERRNO_TO_POSIX_ERROR(KERRNO_READ_ONLY_DEVICE)    /* Read-only file system */
#define EMFILE                                   KERRNO_TO_POSIX_ERROR(KERRNO_NO_MORE_FDS)         /* Too many open files */
#define EXDEV                                    KERRNO_TO_POSIX_ERROR(KERRNO_CROSS_DEVICE_LINK)   /* Cross-device link */
#define ELOOP                                    KERRNO_TO_POSIX_ERROR(KERRNO_LINK_LIMIT)          /* Too many symbolic links */
#define ENOEXEC                                  KERRNO_TO_POSIX_ERROR(KERRNO_NOT_AN_EXECUTABLE)   /* Exec format error */
#define EPIPE                                    KERRNO_TO_POSIX_ERROR(KERRNO_BUSTED_PIPE)         /* Broken pipe */

/* new error codes that can be mapped to POSIX errors */
#define KERRNO_BUFFER_OVERFLOW                   KERRNO_FROM_POSIX_ERROR(EOVERFLOW)  /* Buffer or value overflowed */
#define KERRNO_TOO_MANY_ARGS                     KERRNO_FROM_POSIX_ERROR(E2BIG)      /* Too many arguments */
#define KERRNO_FILE_TOO_LARGE                    KERRNO_FROM_POSIX_ERROR(EFBIG)      /* File is too large */
#define KERRNO_RESULT_NOT_REPRESENTABLE          KERRNO_FROM_POSIX_ERROR(ERANGE)     /* Result cannot be represented */
#define KERRNO_DEVICE_NOT_FOUND                  KERRNO_FROM_POSIX_ERROR(ENODEV)     /* Device was not found */
#define KERRNO_NOT_SUPPORTED                     KERRNO_FROM_POSIX_ERROR(EOPNOTSUPP) /* Operation is not supported */

/* Media Kit Errors */
#define KERRNO_STREAM_NOT_FOUND                  (KERRNO_MEDIA_ERROR_BASE + 1)   /* Requested stream was not found */
#define KERRNO_SERVER_NOT_FOUND                  (KERRNO_MEDIA_ERROR_BASE + 2)   /* Media server was not found */
#define KERRNO_RESOURCE_NOT_FOUND                (KERRNO_MEDIA_ERROR_BASE + 3)   /* Media resource was not found */
#define KERRNO_RESOURCE_UNAVAILABLE              (KERRNO_MEDIA_ERROR_BASE + 4)   /* Media resource is unavailable */
#define KERRNO_BAD_SUBSCRIBER                    (KERRNO_MEDIA_ERROR_BASE + 5)   /* Subscriber is invalid */
#define KERRNO_SUBSCRIBER_NOT_ENTERED            (KERRNO_MEDIA_ERROR_BASE + 6)   /* Subscriber has not entered the media system */
#define KERRNO_BUFFER_NOT_AVAILABLE              (KERRNO_MEDIA_ERROR_BASE + 7)   /* Media buffer is unavailable */
#define KERRNO_LAST_BUFFER_ERROR                 (KERRNO_MEDIA_ERROR_BASE + 8)   /* Last media buffer failed */

#define KERRNO_MEDIA_SYSTEM_FAILURE              (KERRNO_MEDIA_ERROR_BASE + 100) /* Media system failure */
#define KERRNO_MEDIA_BAD_NODE                    (KERRNO_MEDIA_ERROR_BASE + 101) /* Media node is invalid */
#define KERRNO_MEDIA_NODE_BUSY                   (KERRNO_MEDIA_ERROR_BASE + 102) /* Media node is busy */
#define KERRNO_MEDIA_BAD_FORMAT                  (KERRNO_MEDIA_ERROR_BASE + 103) /* Media format is invalid */
#define KERRNO_MEDIA_BAD_BUFFER                  (KERRNO_MEDIA_ERROR_BASE + 104) /* Media buffer is invalid */
#define KERRNO_MEDIA_TOO_MANY_NODES              (KERRNO_MEDIA_ERROR_BASE + 105) /* Too many media nodes */
#define KERRNO_MEDIA_TOO_MANY_BUFFERS            (KERRNO_MEDIA_ERROR_BASE + 106) /* Too many media buffers */
#define KERRNO_MEDIA_NODE_ALREADY_EXISTS         (KERRNO_MEDIA_ERROR_BASE + 107) /* Media node already exists */
#define KERRNO_MEDIA_BUFFER_ALREADY_EXISTS       (KERRNO_MEDIA_ERROR_BASE + 108) /* Media buffer already exists */
#define KERRNO_MEDIA_CANNOT_SEEK                 (KERRNO_MEDIA_ERROR_BASE + 109) /* Media stream cannot seek */
#define KERRNO_MEDIA_CANNOT_CHANGE_RUN_MODE      (KERRNO_MEDIA_ERROR_BASE + 110) /* Media run mode cannot change */
#define KERRNO_MEDIA_APP_ALREADY_REGISTERED      (KERRNO_MEDIA_ERROR_BASE + 111) /* Media application is already registered */
#define KERRNO_MEDIA_APP_NOT_REGISTERED          (KERRNO_MEDIA_ERROR_BASE + 112) /* Media application is not registered */
#define KERRNO_MEDIA_CANNOT_RECLAIM_BUFFERS      (KERRNO_MEDIA_ERROR_BASE + 113) /* Media buffers cannot be reclaimed */
#define KERRNO_MEDIA_BUFFERS_NOT_RECLAIMED       (KERRNO_MEDIA_ERROR_BASE + 114) /* Media buffers were not reclaimed */
#define KERRNO_MEDIA_TIME_SOURCE_STOPPED         (KERRNO_MEDIA_ERROR_BASE + 115) /* Media time source is stopped */
#define KERRNO_MEDIA_TIME_SOURCE_BUSY            (KERRNO_MEDIA_ERROR_BASE + 116) /* Media time source is busy */
#define KERRNO_MEDIA_BAD_SOURCE                  (KERRNO_MEDIA_ERROR_BASE + 117) /* Media source is invalid */
#define KERRNO_MEDIA_BAD_DESTINATION             (KERRNO_MEDIA_ERROR_BASE + 118) /* Media destination is invalid */
#define KERRNO_MEDIA_ALREADY_CONNECTED           (KERRNO_MEDIA_ERROR_BASE + 119) /* Media endpoints are already connected */
#define KERRNO_MEDIA_NOT_CONNECTED               (KERRNO_MEDIA_ERROR_BASE + 120) /* Media endpoints are not connected */
#define KERRNO_MEDIA_BAD_CLIP_FORMAT             (KERRNO_MEDIA_ERROR_BASE + 121) /* Media clip format is invalid */
#define KERRNO_MEDIA_ADDON_FAILED                (KERRNO_MEDIA_ERROR_BASE + 122) /* Media add-on failed */
#define KERRNO_MEDIA_ADDON_DISABLED              (KERRNO_MEDIA_ERROR_BASE + 123) /* Media add-on is disabled */
#define KERRNO_MEDIA_CHANGE_IN_PROGRESS          (KERRNO_MEDIA_ERROR_BASE + 124) /* Media change is in progress */
#define KERRNO_MEDIA_STALE_CHANGE_COUNT          (KERRNO_MEDIA_ERROR_BASE + 125) /* Media change count is stale */
#define KERRNO_MEDIA_ADDON_RESTRICTED            (KERRNO_MEDIA_ERROR_BASE + 126) /* Media add-on is restricted */
#define KERRNO_MEDIA_NO_HANDLER                  (KERRNO_MEDIA_ERROR_BASE + 127) /* No media handler is available */
#define KERRNO_MEDIA_DUPLICATE_FORMAT            (KERRNO_MEDIA_ERROR_BASE + 128) /* Media format is duplicated */
#define KERRNO_MEDIA_REALTIME_DISABLED           (KERRNO_MEDIA_ERROR_BASE + 129) /* Real-time media processing is disabled */
#define KERRNO_MEDIA_REALTIME_UNAVAILABLE        (KERRNO_MEDIA_ERROR_BASE + 130) /* Real-time media processing is unavailable */

/* Mail Kit Errors */
#define KERRNO_MAIL_NO_DAEMON                    (KERRNO_MAIL_ERROR_BASE + 0) /* Mail daemon is unavailable */
#define KERRNO_MAIL_UNKNOWN_USER                 (KERRNO_MAIL_ERROR_BASE + 1) /* Mail user is unknown */
#define KERRNO_MAIL_WRONG_PASSWORD               (KERRNO_MAIL_ERROR_BASE + 2) /* Mail password is incorrect */
#define KERRNO_MAIL_UNKNOWN_HOST                 (KERRNO_MAIL_ERROR_BASE + 3) /* Mail host is unknown */
#define KERRNO_MAIL_ACCESS_ERROR                 (KERRNO_MAIL_ERROR_BASE + 4) /* Mail access failed */
#define KERRNO_MAIL_UNKNOWN_FIELD                (KERRNO_MAIL_ERROR_BASE + 5) /* Mail field is unknown */
#define KERRNO_MAIL_NO_RECIPIENT                 (KERRNO_MAIL_ERROR_BASE + 6) /* Mail has no recipient */
#define KERRNO_MAIL_INVALID_MAIL                 (KERRNO_MAIL_ERROR_BASE + 7) /* Mail message is invalid */

/* Printing Errors */
#define KERRNO_NO_PRINT_SERVER                   (KERRNO_PRINT_ERROR_BASE + 0) /* Print server is unavailable */

/* Device Kit Errors */
#define KERRNO_DEV_INVALID_IOCTL                 (KERRNO_DEVICE_ERROR_BASE + 1)  /* IOCTL request is invalid */
#define KERRNO_DEV_NO_MEMORY                     (KERRNO_DEVICE_ERROR_BASE + 2)  /* Device memory allocation failed */
#define KERRNO_DEV_BAD_DRIVE_NUM                 (KERRNO_DEVICE_ERROR_BASE + 3)  /* Drive number is invalid */
#define KERRNO_DEV_NO_MEDIA                      (KERRNO_DEVICE_ERROR_BASE + 4)  /* Device has no media */
#define KERRNO_DEV_UNREADABLE                    (KERRNO_DEVICE_ERROR_BASE + 5)  /* Device cannot be read */
#define KERRNO_DEV_FORMAT_ERROR                  (KERRNO_DEVICE_ERROR_BASE + 6)  /* Device format is invalid */
#define KERRNO_DEV_TIMEOUT                       (KERRNO_DEVICE_ERROR_BASE + 7)  /* Device operation timed out */
#define KERRNO_DEV_RECALIBRATE_ERROR             (KERRNO_DEVICE_ERROR_BASE + 8)  /* Device recalibration failed */
#define KERRNO_DEV_SEEK_ERROR                    (KERRNO_DEVICE_ERROR_BASE + 9)  /* Device seek failed */
#define KERRNO_DEV_ID_ERROR                      (KERRNO_DEVICE_ERROR_BASE + 10) /* Device identification failed */
#define KERRNO_DEV_READ_ERROR                    (KERRNO_DEVICE_ERROR_BASE + 11) /* Device read failed */
#define KERRNO_DEV_WRITE_ERROR                   (KERRNO_DEVICE_ERROR_BASE + 12) /* Device write failed */
#define KERRNO_DEV_NOT_READY                     (KERRNO_DEVICE_ERROR_BASE + 13) /* Device is not ready */
#define KERRNO_DEV_MEDIA_CHANGED                 (KERRNO_DEVICE_ERROR_BASE + 14) /* Device media changed */
#define KERRNO_DEV_MEDIA_CHANGE_REQUESTED        (KERRNO_DEVICE_ERROR_BASE + 15) /* Device requested a media change */
#define KERRNO_DEV_RESOURCE_CONFLICT             (KERRNO_DEVICE_ERROR_BASE + 16) /* Device resource conflict */
#define KERRNO_DEV_CONFIGURATION_ERROR           (KERRNO_DEVICE_ERROR_BASE + 17) /* Device configuration error */
#define KERRNO_DEV_DISABLED_BY_USER              (KERRNO_DEVICE_ERROR_BASE + 18) /* Device was disabled by the user */
#define KERRNO_DEV_DOOR_OPEN                     (KERRNO_DEVICE_ERROR_BASE + 19) /* Device door is open */

#define KERRNO_DEV_INVALID_PIPE                  (KERRNO_DEVICE_ERROR_BASE + 20) /* Device pipe is invalid */
#define KERRNO_DEV_CRC_ERROR                     (KERRNO_DEVICE_ERROR_BASE + 21) /* Device reported a CRC error */
#define KERRNO_DEV_STALLED                       (KERRNO_DEVICE_ERROR_BASE + 22) /* Device transfer stalled */
#define KERRNO_DEV_BAD_PID                       (KERRNO_DEVICE_ERROR_BASE + 23) /* Device process ID is invalid */
#define KERRNO_DEV_UNEXPECTED_PID                (KERRNO_DEVICE_ERROR_BASE + 24) /* Device returned an unexpected process ID */
#define KERRNO_DEV_DATA_OVERRUN                  (KERRNO_DEVICE_ERROR_BASE + 25) /* Device data overrun */
#define KERRNO_DEV_DATA_UNDERRUN                 (KERRNO_DEVICE_ERROR_BASE + 26) /* Device data underrun */
#define KERRNO_DEV_FIFO_OVERRUN                  (KERRNO_DEVICE_ERROR_BASE + 27) /* Device FIFO overrun */
#define KERRNO_DEV_FIFO_UNDERRUN                 (KERRNO_DEVICE_ERROR_BASE + 28) /* Device FIFO underrun */
#define KERRNO_DEV_PENDING                       (KERRNO_DEVICE_ERROR_BASE + 29) /* Device operation is pending */
#define KERRNO_DEV_MULTIPLE_ERRORS               (KERRNO_DEVICE_ERROR_BASE + 30) /* Device reported multiple errors */
#define KERRNO_DEV_TOO_LATE                      (KERRNO_DEVICE_ERROR_BASE + 31) /* Device operation was too late */

/* Translation Kit Errors */
#define KERRNO_TRANSLATION_BASE_ERROR            (KERRNO_TRANSLATION_ERROR_BASE + 1) /* Generic translation error */
#define KERRNO_NO_TRANSLATOR                     (KERRNO_TRANSLATION_ERROR_BASE + 2) /* No suitable translator is available */
#define KERRNO_ILLEGAL_DATA                      (KERRNO_TRANSLATION_ERROR_BASE + 3) /* Input data is invalid for translation */

