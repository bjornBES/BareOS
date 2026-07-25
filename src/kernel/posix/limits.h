/*
 * File: limits.h
 * File Created: 21 Jul 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 21 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define IOV_MAX 16

/* _SC_ARG_MAX
The maximum length of the arguments to the exec(3) family of functions.  Must not be less than _POSIX_ARG_MAX (4096). */
#define ARG_MAX 4096

/* _SC_CHILD_MAX
The maximum number of simultaneous processes per user ID.  Must not be less than _POSIX_CHILD_MAX (25). */
#define CHILD_MAX 25

/* _SC_HOST_NAME_MAX
Maximum length of a hostname, not including the terminating null byte, as returned by gethostname(2).  Must not be less than _POSIX_HOST_NAME_MAX (255). */
#define HOST_NAME_MAX 255

/* _SC_LOGIN_NAME_MAX
Maximum length of a login name, including the terminating null byte.  Must not be less than _POSIX_LOGIN_NAME_MAX (9). */
#define LOGIN_NAME_MAX 9

/* _SC_NGROUPS_MAX
Maximum number of supplementary group IDs. */
#define NGROUPS_MAX 9

/* _SC_CLK_TCK
The number of clock ticks per second.  The corresponding variable is obsolete.  It was of course called CLK_TCK.  (Note: the macro CLOCKS_PER_SEC does not give information: it must equal 1000000.) */
#define CLOCKS_PER_SEC 1000000

/* _SC_OPEN_MAX
The maximum number of files that a process can have open at any time.  Must not be less than _POSIX_OPEN_MAX (20). */
#define OPEN_MAX 20

/* _SC_PAGESIZE
Size of a page in bytes.  Must not be less than 1. */
#define PAGESIZE 4096

/* _SC_PAGE_SIZE
A synonym for PAGESIZE/_SC_PAGESIZE.  (Both PAGESIZE and PAGE_SIZE are specified in POSIX.) */
#define PAGE_SIZE POSIX

/* _SC_RE_DUP_MAX
The number of repeated occurrences of a BRE permitted by regexec(3) and regcomp(3).  Must not be less than _POSIX2_RE_DUP_MAX (255). */
#define RE_DUP_MAX 255

/* _SC_STREAM_MAX
The maximum number of streams that a process can have open at any time.  If defined, it has the same value as the standard C macro FOPEN_MAX.  Must not be less than _POSIX_STREAM_MAX (8). */
#define STREAM_MAX 8

/* _SC_SYMLOOP_MAX
The maximum number of symbolic links seen in a pathname before resolution returns ELOOP.  Must not be less than _POSIX_SYMLOOP_MAX (8). */
#define SYMLOOP_MAX 8

/* _SC_TTY_NAME_MAX
The maximum length of terminal device name, including the terminating null byte.  Must not be less than _POSIX_TTY_NAME_MAX (9). */
#define TTY_NAME_MAX 9

/* _SC_TZNAME_MAX
The maximum number of bytes in a timezone name.  Must not be less than _POSIX_TZNAME_MAX (6). */
#define TZNAME_MAX 6

/* _SC_VERSION
indicates the year and month the POSIX.1 standard was approved in the format YYYYMML; the value 199009L indicates the Sept. 1990 revision. */
#define _POSIX_VERSION 202401L
