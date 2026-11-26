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

#include <base_errno.h>

// most (if not all) of the comments in this file are made by an AI,
// this will be changed in later versions of the kernel.

/* Error baselines */
#define KERRNO_POSIX_ERROR_BASE                  0x200  /* POSIX-compatible errors */
#define KERRNO_GENERAL_ERROR_BASE                0x1000 /* General errors */
#define KERRNO_OS_ERROR_BASE                     0x2000 /* Kernel Kit errors */
#define KERRNO_APP_ERROR_BASE                    0x3000 /* Application Kit errors */
#define KERRNO_INTERFACE_ERROR_BASE              0x4000 /* Interface Kit errors */
#define KERRNO_MEDIA_ERROR_BASE                  0x5000 /* Media Kit errors */
#define KERRNO_TRANSLATION_ERROR_BASE            0x6000 /* Translation Kit errors */
#define KERRNO_MIDI_ERROR_BASE                   0x7000 /* MIDI Kit errors */
#define KERRNO_STORAGE_ERROR_BASE                0x8000 /* Storage Kit and file-system errors */
#define KERRNO_MAIL_ERROR_BASE                   0x9000 /* Mail Kit errors */
#define KERRNO_PRINT_ERROR_BASE                  0xA000 /* Printing errors */
#define KERRNO_DEVICE_ERROR_BASE                 0xB000 /* Device Kit errors */
#define KERRNO_END_BASE                          0xC000

/* POSIX errors that can be mapped to BeOS error codes */

#define ESIGPARM                                 KERRNO_POSIX_ERROR_BASE + 15             /* Invalid signal parameter */
#define ENOATTR                                  KERRNO_POSIX_ERROR_BASE + 60             /* Attribute is missing */

#define KERRNO_ALLOC_FAILED                      (KERRNO_POSIX_ERROR_BASE + ENOMEM)       /* Memory allocation failed */
#define KERRNO_IO_ERROR                          (KERRNO_POSIX_ERROR_BASE + EIO)          /* Input/output failure */
#define KERRNO_DEVICE_NOT_EXIST                  (KERRNO_POSIX_ERROR_BASE + ENODEV)       /* Device does not exist */
#define KERRNO_ADDRESS_NOT_EXIST                 (KERRNO_POSIX_ERROR_BASE + ENXIO)        /* Address or Device does not exist */
#define KERRNO_PERMISSION_DENIED                 (KERRNO_POSIX_ERROR_BASE + EACCES)       /* Operation is not permitted */
#define KERRNO_BAD_VALUE                         (KERRNO_POSIX_ERROR_BASE + EINVAL)       /* Value is invalid */
#define KERRNO_TIMED_OUT                         (KERRNO_POSIX_ERROR_BASE + ETIMEDOUT)    /* Operation exceeded its time limit */
#define KERRNO_INTERRUPTED                       (KERRNO_POSIX_ERROR_BASE + EINTR)        /* Operation was interrupted */
#define KERRNO_WOULD_BLOCK                       (KERRNO_POSIX_ERROR_BASE + EAGAIN)       /* Operation would block */
#define KERRNO_BUSY                              (KERRNO_POSIX_ERROR_BASE + EBUSY)        /* Resource is busy */
#define KERRNO_NOT_ALLOWED                       (KERRNO_POSIX_ERROR_BASE + EPERM)        /* Operation is not allowed */
#define KERRNO_SYSCALL_WAS_INVALID               (KERRNO_POSIX_ERROR_BASE + ENOSYS)       /* Invalid system call number */

#define KERRNO_BAD_ADDRESS                       (KERRNO_POSIX_ERROR_BASE + EFAULT)       /* Address is invalid */
#define KERRNO_NOT_AN_EXECUTABLE                 (KERRNO_POSIX_ERROR_BASE + ENOEXEC)      /* File is not executable */

#define KERRNO_FILE_ERROR                        (KERRNO_POSIX_ERROR_BASE + EBADF)        /* Generic file error */
#define KERRNO_FILE_EXISTS                       (KERRNO_POSIX_ERROR_BASE + EEXIST)       /* File or directory already exists */
#define KERRNO_ENTRY_NOT_FOUND                   (KERRNO_POSIX_ERROR_BASE + ENOENT)       /* Directory entry was not found */
#define KERRNO_NAME_TOO_LONG                     (KERRNO_POSIX_ERROR_BASE + ENAMETOOLONG) /* File or directory name is too long */
#define KERRNO_NOT_A_DIRECTORY                   (KERRNO_POSIX_ERROR_BASE + ENOTDIR)      /* Path component is not a directory */
#define KERRNO_DIRECTORY_NOT_EMPTY               (KERRNO_POSIX_ERROR_BASE + ENOTEMPTY)    /* Directory contains entries */
#define KERRNO_DEVICE_FULL                       (KERRNO_POSIX_ERROR_BASE + ENOSPC)       /* Device has no free space */
#define KERRNO_READ_ONLY_DEVICE                  (KERRNO_POSIX_ERROR_BASE + EROFS)        /* Device cannot be written */
#define KERRNO_IS_A_DIRECTORY                    (KERRNO_POSIX_ERROR_BASE + EISDIR)       /* Path refers to a directory */
#define KERRNO_NO_MORE_FDS                       (KERRNO_POSIX_ERROR_BASE + EMFILE)       /* No file descriptors remain */
#define KERRNO_CROSS_DEVICE_LINK                 (KERRNO_POSIX_ERROR_BASE + EXDEV)        /* Link crosses devices */
#define KERRNO_LINK_LIMIT                        (KERRNO_POSIX_ERROR_BASE + ELOOP)        /* Link traversal limit was reached */
#define KERRNO_BUSTED_PIPE                       (KERRNO_POSIX_ERROR_BASE + EPIPE)        /* Pipe has no readers */

#define KERRNO_BUFFER_OVERFLOW                   (KERRNO_POSIX_ERROR_BASE + EOVERFLOW)    /* Buffer or value overflowed */
#define KERRNO_TOO_MANY_ARGS                     (KERRNO_POSIX_ERROR_BASE + E2BIG)        /* Too many arguments */
#define KERRNO_FILE_TOO_LARGE                    (KERRNO_POSIX_ERROR_BASE + EFBIG)        /* File is too large */
#define KERRNO_RESULT_NOT_REPRESENTABLE          (KERRNO_POSIX_ERROR_BASE + ERANGE)       /* Result cannot be represented */
#define KERRNO_NOT_SUPPORTED                     (KERRNO_POSIX_ERROR_BASE + EOPNOTSUPP)   /* Operation is not supported */

#define KERRNO_CANCELED                          (KERRNO_POSIX_ERROR_BASE + ECANCELED)    /* Operation was canceled */
#define KERRNO_ALREADY                           (KERRNO_POSIX_ERROR_BASE + EALREADY)     /* Operation is already in progress */

#define KERRNO_UNSUCCESS                         ((int)~0)
#define KERRNO_SUCCESS                           ((int)0)

/* General Errors */
#define KERRNO_BAD_INDEX                         (KERRNO_GENERAL_ERROR_BASE + 4)  /* Index is outside the valid range */
#define KERRNO_BAD_TYPE                          (KERRNO_GENERAL_ERROR_BASE + 5)  /* Value has the wrong type */
#define KERRNO_BAD_ARGUMENT                      (KERRNO_GENERAL_ERROR_BASE + 6)  /* Argument is invalid */
#define KERRNO_MISMATCHED_VALUES                 (KERRNO_GENERAL_ERROR_BASE + 7)  /* Related values do not match */
#define KERRNO_NAME_NOT_FOUND                    (KERRNO_GENERAL_ERROR_BASE + 8)  /* Requested name was not found */
#define KERRNO_NAME_IN_USE                       (KERRNO_GENERAL_ERROR_BASE + 9)  /* Name is already in use */
#define KERRNO_NO_INIT                           (KERRNO_GENERAL_ERROR_BASE + 14) /* Required initialization is unavailable */
#define KERRNO_NOT_INITIALIZED                   (KERRNO_GENERAL_ERROR_BASE + 15) /* Component has not been initialized */
#define KERRNO_BAD_DATA                          (KERRNO_GENERAL_ERROR_BASE + 18) /* Data is malformed or invalid */
#define KERRNO_ENTRY_IN_USE                      (KERRNO_GENERAL_ERROR_BASE + 19) /* entry is already in use */

/* Kernel Kit Errors */
#define KERRNO_BAD_SEM_ID                        (KERRNO_OS_ERROR_BASE + 1)     /* Semaphore ID is invalid */
#define KERRNO_NO_MORE_SEMS                      (KERRNO_OS_ERROR_BASE + 2)     /* No semaphore IDs remain */

#define KERRNO_DONT_DO_THAT                      (KERRNO_OS_ERROR_BASE + 0x80)  /* Operation is explicitly unsupported */

#define KERRNO_BAD_THREAD_ID                     (KERRNO_OS_ERROR_BASE + 0x100) /* Thread ID is invalid */
#define KERRNO_NO_MORE_THREADS                   (KERRNO_OS_ERROR_BASE + 0x101) /* No thread IDs remain */
#define KERRNO_BAD_THREAD_STATE                  (KERRNO_OS_ERROR_BASE + 0x102) /* Thread state is invalid */
#define KERRNO_BAD_TEAM_ID                       (KERRNO_OS_ERROR_BASE + 0x103) /* Team ID is invalid */
#define KERRNO_NO_MORE_TEAMS                     (KERRNO_OS_ERROR_BASE + 0x104) /* No team IDs remain */

#define KERRNO_BAD_PORT_ID                       (KERRNO_OS_ERROR_BASE + 0x200) /* Port ID is invalid */
#define KERRNO_NO_MORE_PORTS                     (KERRNO_OS_ERROR_BASE + 0x201) /* No port IDs remain */

#define KERRNO_BAD_IMAGE_ID                      (KERRNO_OS_ERROR_BASE + 0x300) /* Image ID is invalid */
#define KERRNO_MISSING_LIBRARY                   (KERRNO_OS_ERROR_BASE + 0x303) /* Required library is missing */
#define KERRNO_MISSING_SYMBOL                    (KERRNO_OS_ERROR_BASE + 0x304) /* Required symbol is missing */
#define KERRNO_UNKNOWN_EXECUTABLE                (KERRNO_OS_ERROR_BASE + 0x305) /* Executable format is unknown */
#define KERRNO_LEGACY_EXECUTABLE                 (KERRNO_OS_ERROR_BASE + 0x306) /* Executable uses a legacy format */

#define KERRNO_DEBUGGER_ALREADY_INSTALLED        (KERRNO_OS_ERROR_BASE + 0x400) /* A debugger is already installed */
#define KERRNO_DEBUG                             (KERRNO_OS_ERROR_BASE + 0x401) /* Only for debug builds */

#define KERRNO_OUT_OF_MEMORY                     (KERRNO_OS_ERROR_BASE + 0x500) /* Out of memory */
#define KERRNO_BAD_MEM_ALIGN                     (KERRNO_OS_ERROR_BASE + 0x501) /* Invalid address alignment */
#define KERRNO_BAD_PADDR                         (KERRNO_OS_ERROR_BASE + 0x502) /* Nonexistent physical address */
#define KERRNO_PHYS_ALLOC_FAILED                 (KERRNO_OS_ERROR_BASE + 0x503) /* Physical memory allocation failed */
#define KERRNO_PAGETABLE_NOT_PRESENT             (KERRNO_OS_ERROR_BASE + 0x504) /* A Page table (or entry) is not present */

#define KERRNO_PROBE_DEFER                       (KERRNO_OS_ERROR_BASE + 0x600) /* Driver requests probe retry */

#define KERRNO_IRQ_ALLOC_FAILED                  (KERRNO_OS_ERROR_BASE + 0x700) /* Couldn't allocate a free vector */

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
#define KERRNO_UNSUPPORTED                       (KERRNO_STORAGE_ERROR_BASE + 14) /* Operation is unsupported */
#define KERRNO_PARTITION_TOO_SMALL               (KERRNO_STORAGE_ERROR_BASE + 15) /* Partition is too small */
#define KERRNO_PARTIAL_READ                      (KERRNO_STORAGE_ERROR_BASE + 16) /* Read completed only partially */
#define KERRNO_PARTIAL_WRITE                     (KERRNO_STORAGE_ERROR_BASE + 17) /* Write completed only partially */

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
#define KERRNO_DEV_NOT_FOUND                     (KERRNO_DEVICE_ERROR_BASE + 32) /* Device was not found */
#define KERRNO_DEV_ALREADY_EXISTS                (KERRNO_DEVICE_ERROR_BASE + 33) /* Device already exists in the registry */

/* Translation Kit Errors */
#define KERRNO_TRANSLATION_BASE_ERROR            (KERRNO_TRANSLATION_ERROR_BASE + 1) /* Generic translation error */
#define KERRNO_NO_TRANSLATOR                     (KERRNO_TRANSLATION_ERROR_BASE + 2) /* No suitable translator is available */
#define KERRNO_ILLEGAL_DATA                      (KERRNO_TRANSLATION_ERROR_BASE + 3) /* Input data is invalid for translation */

/* POSIX Errors */
#define KERRNO_TO_POSIX_ERROR(error)             (-(error)) /* Convert a positive kernel error to a negative POSIX-style result */
#define KERRNO_FROM_POSIX_ERROR(error)           (-(error)) /* Convert a negative POSIX-style result to a positive kernel error */

