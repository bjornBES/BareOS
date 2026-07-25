/*
 * File: winternl_types.h
 * File Created: 27 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "ntdef.h"
#include "windef.h"

/**********************************************************************
 * Fundamental types and data structures
 */

#ifndef WINE_NTSTATUS_DECLARED
#define WINE_NTSTATUS_DECLARED
typedef LONG NTSTATUS;
#endif

typedef const char *PCSZ;

typedef short   CSHORT;
typedef CSHORT *PCSHORT;

#ifndef __STRING_DEFINED__
#define __STRING_DEFINED__

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PCHAR  Buffer;
} STRING, *PSTRING;
#endif

typedef STRING        ANSI_STRING;
typedef PSTRING       PANSI_STRING;
typedef const STRING *PCANSI_STRING;

typedef STRING        OEM_STRING;
typedef PSTRING       POEM_STRING;
typedef const STRING *PCOEM_STRING;

#ifndef __UNICODE_STRING_DEFINED__
#define __UNICODE_STRING_DEFINED__

typedef struct _UNICODE_STRING
{
    USHORT Length;        /* bytes */
    USHORT MaximumLength; /* bytes */
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#endif

typedef const UNICODE_STRING *PCUNICODE_STRING;

#ifndef _FILETIME_
#define _FILETIME_

/* 64 bit number of 100 nanoseconds intervals since January 1, 1601 */
typedef struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif /* _FILETIME_ */

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    PVOID          Handle;
} CURDIR, *PCURDIR;

typedef struct _GDI_TEB_BATCH
{
    ULONG  Offset;
    HANDLE HDC;
    ULONG  Buffer[0x136];
} GDI_TEB_BATCH;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT
{
    ULONG       Flags;
    const char *FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT_EX
{
    TEB_ACTIVE_FRAME_CONTEXT BasicContext;
    const char              *SourceLocation;
} TEB_ACTIVE_FRAME_CONTEXT_EX, *PTEB_ACTIVE_FRAME_CONTEXT_EX;

typedef struct _TEB_ACTIVE_FRAME
{
    ULONG                     Flags;
    struct _TEB_ACTIVE_FRAME *Previous;
    TEB_ACTIVE_FRAME_CONTEXT *Context;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

typedef struct _TEB_ACTIVE_FRAME_EX
{
    TEB_ACTIVE_FRAME BasicFrame;
    void            *ExtensionIdentifier;
} TEB_ACTIVE_FRAME_EX, *PTEB_ACTIVE_FRAME_EX;

typedef struct _FLS_CALLBACK
{
    void                  *unknown;
    PFLS_CALLBACK_FUNCTION callback; /* ~0 if NULL callback is set, NULL if FLS index is free. */
} FLS_CALLBACK, *PFLS_CALLBACK;

typedef struct _FLS_INFO_CHUNK
{
    ULONG        count;        /* number of allocated FLS indexes in the chunk. */
    FLS_CALLBACK callbacks[1]; /* the size is 0x10 for chunk 0 and is twice as
                                * the previous chunk size for the rest. */
} FLS_INFO_CHUNK, *PFLS_INFO_CHUNK;

typedef struct _GLOBAL_FLS_DATA
{
    FLS_INFO_CHUNK *fls_callback_chunks[8];
    LIST_ENTRY      fls_list_head;
    ULONG           fls_high_index;
} GLOBAL_FLS_DATA, *PGLOBAL_FLS_DATA;

typedef struct _TEB_FLS_DATA
{
    LIST_ENTRY fls_list_entry;
    void     **fls_data_chunks[8];
} TEB_FLS_DATA, *PTEB_FLS_DATA;

/* undocumented layout of WOW64INFO.CrossProcessWorkList and CHPEV2_PROCESS_INFO.CrossProcessWorkList */

typedef struct
{
    UINT      next;
    UINT      id;
    ULONGLONG addr;
    ULONGLONG size;
    UINT      args[4];
} CROSS_PROCESS_WORK_ENTRY;

typedef union
{
    struct
    {
        UINT first;
        UINT counter;
    };

    volatile LONGLONG hdr;
} CROSS_PROCESS_WORK_HDR;

typedef struct
{
    CROSS_PROCESS_WORK_HDR   free_list;
    CROSS_PROCESS_WORK_HDR   work_list;
    ULONGLONG                unknown[4];
    CROSS_PROCESS_WORK_ENTRY entries[1];
} CROSS_PROCESS_WORK_LIST;

typedef enum
{
    CrossProcessPreVirtualAlloc    = 0,
    CrossProcessPostVirtualAlloc   = 1,
    CrossProcessPreVirtualFree     = 2,
    CrossProcessPostVirtualFree    = 3,
    CrossProcessPreVirtualProtect  = 4,
    CrossProcessPostVirtualProtect = 5,
    CrossProcessFlushCache         = 6,
    CrossProcessFlushCacheHeavy    = 7,
    CrossProcessMemoryWrite        = 8,
} CROSS_PROCESS_NOTIFICATION;

#define CROSS_PROCESS_LIST_FLUSH 0x80000000
#define CROSS_PROCESS_LIST_ENTRY(list, pos)                                              \
    ((CROSS_PROCESS_WORK_ENTRY *)((char *)(list) + ((pos) & ~CROSS_PROCESS_LIST_FLUSH)))

typedef struct _CHPE_V2_CPU_AREA_INFO
{
    BOOLEAN             InSimulation;         /* 000 */
    BOOLEAN             InSyscallCallback;    /* 001 */
    ULONG64             EmulatorStackBase;    /* 008 */
    ULONG64             EmulatorStackLimit;   /* 010 */
    ARM64EC_NT_CONTEXT *ContextAmd64;         /* 018 */
    ULONG              *SuspendDoorbell;      /* 020 */
    ULONG64             LoadingModuleModflag; /* 028 */
    void               *EmulatorData[4];      /* 030 */
    ULONG64             EmulatorDataInline;   /* 050 */
} CHPE_V2_CPU_AREA_INFO, *PCHPE_V2_CPU_AREA_INFO;

/* equivalent of WOW64INFO, stored after the 64-bit PEB */
typedef struct _CHPEV2_PROCESS_INFO
{
    ULONG                    Wow64ExecuteFlags;    /* 000 */
    USHORT                   NativeMachineType;    /* 004 */
    USHORT                   EmulatedMachineType;  /* 006 */
    HANDLE                   SectionHandle;        /* 008 */
    CROSS_PROCESS_WORK_LIST *CrossProcessWorkList; /* 010 */
    void                    *unknown;              /* 018 */
} CHPEV2_PROCESS_INFO, *PCHPEV2_PROCESS_INFO;

#define TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED 0x00000001
#define TEB_ACTIVE_FRAME_FLAG_EXTENDED         0x00000001

typedef NTSTATUS(WINAPI *KERNEL_CALLBACK_PROC)(void *, ULONG); /* FIXME: not the correct name */

/***********************************************************************
 * The 32-bit/64-bit version of the PEB and TEB for WoW64
 */

typedef struct _CLIENT_ID32
{
    ULONG UniqueProcess;
    ULONG UniqueThread;
} CLIENT_ID32;

typedef struct _CLIENT_ID64
{
    ULONG64 UniqueProcess;
    ULONG64 UniqueThread;
} CLIENT_ID64;

typedef struct _LIST_ENTRY32
{
    ULONG Flink;
    ULONG Blink;
} LIST_ENTRY32;

typedef struct _LIST_ENTRY64
{
    ULONG64 Flink;
    ULONG64 Blink;
} LIST_ENTRY64;

typedef struct _UNICODE_STRING32
{
    USHORT Length;
    USHORT MaximumLength;
    ULONG  Buffer;
} UNICODE_STRING32;

typedef struct _UNICODE_STRING64
{
    USHORT  Length;
    USHORT  MaximumLength;
    ULONG64 Buffer;
} UNICODE_STRING64;

typedef struct _ACTIVATION_CONTEXT_STACK32
{
    ULONG        ActiveFrame;
    LIST_ENTRY32 FrameListCache;
    ULONG        Flags;
    ULONG        NextCookieSequenceNumber;
    ULONG32      StackId;
} ACTIVATION_CONTEXT_STACK32;

typedef struct _ACTIVATION_CONTEXT_STACK64
{
    ULONG64      ActiveFrame;
    LIST_ENTRY64 FrameListCache;
    ULONG        Flags;
    ULONG        NextCookieSequenceNumber;
    ULONG64      StackId;
} ACTIVATION_CONTEXT_STACK64;

typedef struct _CURDIR32
{
    UNICODE_STRING32 DosPath;
    ULONG            Handle;
} CURDIR32;

typedef struct _CURDIR64
{
    UNICODE_STRING64 DosPath;
    ULONG64          Handle;
} CURDIR64;

/***********************************************************************
 * Enums
 */

typedef enum _FILE_INFORMATION_CLASS
{
    FileDirectoryInformation                     = 1,
    FileFullDirectoryInformation                 = 2,
    FileBothDirectoryInformation                 = 3,
    FileBasicInformation                         = 4,
    FileStandardInformation                      = 5,
    FileInternalInformation                      = 6,
    FileEaInformation                            = 7,
    FileAccessInformation                        = 8,
    FileNameInformation                          = 9,
    FileRenameInformation                        = 10,
    FileLinkInformation                          = 11,
    FileNamesInformation                         = 12,
    FileDispositionInformation                   = 13,
    FilePositionInformation                      = 14,
    FileFullEaInformation                        = 15,
    FileModeInformation                          = 16,
    FileAlignmentInformation                     = 17,
    FileAllInformation                           = 18,
    FileAllocationInformation                    = 19,
    FileEndOfFileInformation                     = 20,
    FileAlternateNameInformation                 = 21,
    FileStreamInformation                        = 22,
    FilePipeInformation                          = 23,
    FilePipeLocalInformation                     = 24,
    FilePipeRemoteInformation                    = 25,
    FileMailslotQueryInformation                 = 26,
    FileMailslotSetInformation                   = 27,
    FileCompressionInformation                   = 28,
    FileObjectIdInformation                      = 29,
    FileCompletionInformation                    = 30,
    FileMoveClusterInformation                   = 31,
    FileQuotaInformation                         = 32,
    FileReparsePointInformation                  = 33,
    FileNetworkOpenInformation                   = 34,
    FileAttributeTagInformation                  = 35,
    FileTrackingInformation                      = 36,
    FileIdBothDirectoryInformation               = 37,
    FileIdFullDirectoryInformation               = 38,
    FileValidDataLengthInformation               = 39,
    FileShortNameInformation                     = 40,
    FileIoCompletionNotificationInformation      = 41,
    FileIoStatusBlockRangeInformation            = 42,
    FileIoPriorityHintInformation                = 43,
    FileSfioReserveInformation                   = 44,
    FileSfioVolumeInformation                    = 45,
    FileHardLinkInformation                      = 46,
    FileProcessIdsUsingFileInformation           = 47,
    FileNormalizedNameInformation                = 48,
    FileNetworkPhysicalNameInformation           = 49,
    FileIdGlobalTxDirectoryInformation           = 50,
    FileIsRemoteDeviceInformation                = 51,
    FileAttributeCacheInformation                = 52,
    FileNumaNodeInformation                      = 53,
    FileStandardLinkInformation                  = 54,
    FileRemoteProtocolInformation                = 55,
    FileRenameInformationBypassAccessCheck       = 56,
    FileLinkInformationBypassAccessCheck         = 57,
    FileVolumeNameInformation                    = 58,
    FileIdInformation                            = 59,
    FileIdExtdDirectoryInformation               = 60,
    FileReplaceCompletionInformation             = 61,
    FileHardLinkFullIdInformation                = 62,
    FileIdExtdBothDirectoryInformation           = 63,
    FileDispositionInformationEx                 = 64,
    FileRenameInformationEx                      = 65,
    FileRenameInformationExBypassAccessCheck     = 66,
    FileDesiredStorageClassInformation           = 67,
    FileStatInformation                          = 68,
    FileMemoryPartitionInformation               = 69,
    FileStatLxInformation                        = 70,
    FileCaseSensitiveInformation                 = 71,
    FileLinkInformationEx                        = 72,
    FileLinkInformationExBypassAccessCheck       = 73,
    FileStorageReserveIdInformation              = 74,
    FileCaseSensitiveInformationForceAccessCheck = 75,
    FileKnownFolderInformation                   = 76,
    FileStatBasicInformation                     = 77,
    FileId64ExtdDirectoryInformation             = 78,
    FileId64ExtdBothDirectoryInformation         = 79,
    FileIdAllExtdDirectoryInformation            = 80,
    FileIdAllExtdBothDirectoryInformation        = 81,
    FileStreamReservationInformation             = 82,
    FileMupProviderInfo                          = 83,
    FileMaximumInformation,
#ifdef __WINESRC__
    WineFileUnixNameInformation = 1000
#endif
} FILE_INFORMATION_CLASS,
    *PFILE_INFORMATION_CLASS;

typedef struct _FILE_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_FULL_DIRECTORY_INFORMATION, *PFILE_FULL_DIRECTORY_INFORMATION,
    FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    LARGE_INTEGER FileId;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_ID_FULL_DIRECTORY_INFORMATION, *PFILE_ID_FULL_DIRECTORY_INFORMATION;

typedef struct _FILE_BOTH_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    CHAR          ShortNameLength;
    WCHAR         ShortName[12];
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_BOTH_DIRECTORY_INFORMATION, *PFILE_BOTH_DIRECTORY_INFORMATION,
    FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    CHAR          ShortNameLength;
    WCHAR         ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_ID_BOTH_DIRECTORY_INFORMATION, *PFILE_ID_BOTH_DIRECTORY_INFORMATION;

typedef struct _FILE_ID_GLOBAL_TX_DIR_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    LARGE_INTEGER FileId;
    GUID          LockingTransactionId;
    ULONG         TxInfoFlags;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_ID_GLOBAL_TX_DIR_INFORMATION, *PFILE_ID_GLOBAL_TX_DIR_INFORMATION;

typedef struct _FILE_BASIC_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG         FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION
{
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG         NumberOfLinks;
    BOOLEAN       DeletePending;
    BOOLEAN       Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION
{
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_ID_128
{
    UCHAR Identifier[16];
} FILE_ID_128, *PFILE_ID_128;

typedef struct _FILE_ID_INFORMATION
{
    ULONGLONG   VolumeSerialNumber;
    FILE_ID_128 FileId;
} FILE_ID_INFORMATION, *PFILE_ID_INFORMATION;

typedef struct _FILE_EA_INFORMATION
{
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION
{
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_NAME_INFORMATION
{
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_RENAME_INFORMATION
{
    union
    {
        BOOLEAN ReplaceIfExists;
        ULONG   Flags;
    } DUMMYUNIONNAME;

    HANDLE RootDirectory;
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

#define FILE_RENAME_REPLACE_IF_EXISTS                    0x00000001
#define FILE_RENAME_POSIX_SEMANTICS                      0x00000002
#define FILE_RENAME_SUPPRESS_PIN_STATE_INHERITANCE       0x00000004
#define FILE_RENAME_SUPPRESS_STORAGE_RESERVE_INHERITANCE 0x00000008
#define FILE_RENAME_NO_INCREASE_AVAILABLE_SPACE          0x00000010
#define FILE_RENAME_NO_DECREASE_AVAILABLE_SPACE          0x00000020
#define FILE_RENAME_PRESERVE_AVAILABLE_SPACE             0x00000030
#define FILE_RENAME_IGNORE_READONLY_ATTRIBUTE            0x00000040

typedef struct _FILE_LINK_INFORMATION
{
    union
    {
        BOOLEAN ReplaceIfExists;
        ULONG   Flags;
    } DUMMYUNIONNAME;

    HANDLE RootDirectory;
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;

#define FILE_LINK_REPLACE_IF_EXISTS                    0x00000001
#define FILE_LINK_POSIX_SEMANTICS                      0x00000002
#define FILE_LINK_SUPPRESS_STORAGE_RESERVE_INHERITANCE 0x00000008
#define FILE_LINK_NO_INCREASE_AVAILABLE_SPACE          0x00000010
#define FILE_LINK_NO_DECREASE_AVAILABLE_SPACE          0x00000020
#define FILE_LINK_PRESERVE_AVAILABLE_SPACE             0x00000030
#define FILE_LINK_IGNORE_READONLY_ATTRIBUTE            0x00000040
#define FILE_LINK_FORCE_RESIZE_TARGET_SR               0x00000080
#define FILE_LINK_FORCE_RESIZE_SOURCE_SR               0x00000100
#define FILE_LINK_FORCE_RESIZE_SR                      0x00000180

typedef struct _FILE_NAMES_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_DISPOSITION_INFORMATION
{
    BOOLEAN DoDeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

typedef struct _FILE_DISPOSITION_INFORMATION_EX
{
    ULONG Flags;
} FILE_DISPOSITION_INFORMATION_EX, *PFILE_DISPOSITION_INFORMATION_EX;

#define FILE_DISPOSITION_DO_NOT_DELETE             0x00000000
#define FILE_DISPOSITION_DELETE                    0x00000001
#define FILE_DISPOSITION_POSIX_SEMANTICS           0x00000002
#define FILE_DISPOSITION_FORCE_IMAGE_SECTION_CHECK 0x00000004
#define FILE_DISPOSITION_ON_CLOSE                  0x00000008
#define FILE_DISPOSITION_IGNORE_READONLY_ATTRIBUTE 0x00000010

typedef struct _FILE_POSITION_INFORMATION
{
    LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION
{
    ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_ALLOCATION_INFORMATION
{
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;

typedef struct _FILE_END_OF_FILE_INFORMATION
{
    LARGE_INTEGER EndOfFile;
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION;

typedef struct _FILE_NETWORK_OPEN_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG         FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION
{
    ULONG  NextEntryOffset;
    UCHAR  Flags;
    UCHAR  EaNameLength;
    USHORT EaValueLength;
    CHAR   EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

typedef struct _FILE_MODE_INFORMATION
{
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_STREAM_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR         StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION
{
    ULONG FileAttributes;
    ULONG ReparseTag;
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;

typedef struct _FILE_MAILSLOT_QUERY_INFORMATION
{
    ULONG         MaximumMessageSize;
    ULONG         MailslotQuota;
    ULONG         NextMessageSize;
    ULONG         MessagesAvailable;
    LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_QUERY_INFORMATION, *PFILE_MAILSLOT_QUERY_INFORMATION;

typedef struct _FILE_MAILSLOT_SET_INFORMATION
{
    LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_SET_INFORMATION, *PFILE_MAILSLOT_SET_INFORMATION;

typedef struct _FILE_PIPE_INFORMATION
{
    ULONG ReadMode;
    ULONG CompletionMode;
} FILE_PIPE_INFORMATION, *PFILE_PIPE_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION
{
    ULONG NamedPipeType;
    ULONG NamedPipeConfiguration;
    ULONG MaximumInstances;
    ULONG CurrentInstances;
    ULONG InboundQuota;
    ULONG ReadDataAvailable;
    ULONG OutboundQuota;
    ULONG WriteQuotaAvailable;
    ULONG NamedPipeState;
    ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

#define FILE_PIPE_DISCONNECTED_STATE 0x00000001
#define FILE_PIPE_LISTENING_STATE    0x00000002
#define FILE_PIPE_CONNECTED_STATE    0x00000003
#define FILE_PIPE_CLOSING_STATE      0x00000004

typedef struct _FILE_OBJECTID_BUFFER
{
    BYTE ObjectId[16];

    union
    {
        struct
        {
            BYTE BirthVolumeId[16];
            BYTE BirthObjectId[16];
            BYTE DomainId[16];
        } DUMMYSTRUCTNAME;

        BYTE ExtendedInfo[48];
    } DUMMYUNIONNAME;
} FILE_OBJECTID_BUFFER, *PFILE_OBJECTID_BUFFER;

typedef struct _FILE_OBJECTID_INFORMATION
{
    LONGLONG FileReference;
    UCHAR    ObjectId[16];

    union
    {
        struct
        {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } DUMMYSTRUCTNAME;

        UCHAR ExtendedInfo[48];
    } DUMMYUNIONNAME;
} FILE_OBJECTID_INFORMATION, *PFILE_OBJECTID_INFORMATION;

typedef struct _FILE_QUOTA_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         SidLength;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER QuotaUsed;
    LARGE_INTEGER QuotaThreshold;
    LARGE_INTEGER QuotaLimit;
    SID           Sid;
} FILE_QUOTA_INFORMATION, *PFILE_QUOTA_INFORMATION;

typedef struct _FILE_REPARSE_POINT_INFORMATION
{
    LONGLONG FileReference;
    ULONG    Tag;
} FILE_REPARSE_POINT_INFORMATION, *PFILE_REPARSE_POINT_INFORMATION;

typedef struct _FILE_ALL_INFORMATION
{
    FILE_BASIC_INFORMATION     BasicInformation;
    FILE_STANDARD_INFORMATION  StandardInformation;
    FILE_INTERNAL_INFORMATION  InternalInformation;
    FILE_EA_INFORMATION        EaInformation;
    FILE_ACCESS_INFORMATION    AccessInformation;
    FILE_POSITION_INFORMATION  PositionInformation;
    FILE_MODE_INFORMATION      ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION      NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_STAT_INFORMATION
{
    LARGE_INTEGER FileId;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG         FileAttributes;
    ULONG         ReparseTag;
    ULONG         NumberOfLinks;
    ULONG         EffectiveAccess;
} FILE_STAT_INFORMATION, *PFILE_STAT_INFORMATION;

typedef struct _FILE_IO_COMPLETION_NOTIFICATION_INFORMATION
{
    ULONG Flags;
} FILE_IO_COMPLETION_NOTIFICATION_INFORMATION, *PFILE_IO_COMPLETION_NOTIFICATION_INFORMATION;

#define FILE_SKIP_COMPLETION_PORT_ON_SUCCESS 0x1
#define FILE_SKIP_SET_EVENT_ON_HANDLE        0x2
#define FILE_SKIP_SET_USER_EVENT_ON_FAST_IO  0x4

typedef struct _FILE_ID_EXTD_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    ULONG         ReparsePointTag;
    FILE_ID_128   FileId;
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_ID_EXTD_DIRECTORY_INFORMATION, *PFILE_ID_EXTD_DIRECTORY_INFORMATION;

typedef struct _FILE_ID_EXTD_BOTH_DIRECTORY_INFORMATION
{
    ULONG         NextEntryOffset;
    ULONG         FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG         FileAttributes;
    ULONG         FileNameLength;
    ULONG         EaSize;
    ULONG         ReparsePointTag;
    FILE_ID_128   FileId;
    CHAR          ShortNameLength;
    WCHAR         ShortName[12];
    WCHAR         FileName[ANYSIZE_ARRAY];
} FILE_ID_EXTD_BOTH_DIRECTORY_INFORMATION, *PFILE_ID_EXTD_BOTH_DIRECTORY_INFORMATION;

#ifdef __WINESRC__
/* data for WineFileUnixNameInformation */
typedef struct
{
    ULONG Length;
    char  Name[];
} WINE_FILE_UNIX_NAME_INFORMATION;
#endif

typedef struct _PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION
{
    ULONG Version;
    ULONG Reserved;
    VOID *Callback;
} PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION, *PPROCESS_INSTRUMENTATION_CALLBACK_INFORMATION;

typedef enum _FSINFOCLASS
{
    FileFsVolumeInformation = 1,
    FileFsLabelInformation,
    FileFsSizeInformation,
    FileFsDeviceInformation,
    FileFsAttributeInformation,
    FileFsControlInformation,
    FileFsFullSizeInformation,
    FileFsObjectIdInformation,
    FileFsDriverPathInformation,
    FileFsVolumeFlagsInformation,
    FileFsSectorSizeInformation,
    FileFsDataCopyInformation,
    FileFsMetadataSizeInformation,
    FileFsFullSizeInformationEx,
    FileFsMaximumInformation
} FS_INFORMATION_CLASS,
    *PFS_INFORMATION_CLASS;

typedef enum _KEY_INFORMATION_CLASS
{
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    KeyTrustInformation,
    KeyLayerInformation,
    MaxKeyInfoClass
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS
{
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    KeyValueLayerInformation,
} KEY_VALUE_INFORMATION_CLASS;

typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectTypesInformation,
    ObjectHandleFlagInformation,
    ObjectSessionInformation,
    ObjectSessionObjectInformation,
} OBJECT_INFORMATION_CLASS,
    *POBJECT_INFORMATION_CLASS;

typedef enum _PROCESSINFOCLASS
{
    ProcessBasicInformation                     = 0,
    ProcessQuotaLimits                          = 1,
    ProcessIoCounters                           = 2,
    ProcessVmCounters                           = 3,
    ProcessTimes                                = 4,
    ProcessBasePriority                         = 5,
    ProcessRaisePriority                        = 6,
    ProcessDebugPort                            = 7,
    ProcessExceptionPort                        = 8,
    ProcessAccessToken                          = 9,
    ProcessLdtInformation                       = 10,
    ProcessLdtSize                              = 11,
    ProcessDefaultHardErrorMode                 = 12,
    ProcessIoPortHandlers                       = 13,
    ProcessPooledUsageAndLimits                 = 14,
    ProcessWorkingSetWatch                      = 15,
    ProcessUserModeIOPL                         = 16,
    ProcessEnableAlignmentFaultFixup            = 17,
    ProcessPriorityClass                        = 18,
    ProcessWx86Information                      = 19,
    ProcessHandleCount                          = 20,
    ProcessAffinityMask                         = 21,
    ProcessPriorityBoost                        = 22,
    ProcessDeviceMap                            = 23,
    ProcessSessionInformation                   = 24,
    ProcessForegroundInformation                = 25,
    ProcessWow64Information                     = 26,
    ProcessImageFileName                        = 27,
    ProcessLUIDDeviceMapsEnabled                = 28,
    ProcessBreakOnTermination                   = 29,
    ProcessDebugObjectHandle                    = 30,
    ProcessDebugFlags                           = 31,
    ProcessHandleTracing                        = 32,
    ProcessIoPriority                           = 33,
    ProcessExecuteFlags                         = 34,
    ProcessTlsInformation                       = 35,
    ProcessCookie                               = 36,
    ProcessImageInformation                     = 37,
    ProcessCycleTime                            = 38,
    ProcessPagePriority                         = 39,
    ProcessInstrumentationCallback              = 40,
    ProcessThreadStackAllocation                = 41,
    ProcessWorkingSetWatchEx                    = 42,
    ProcessImageFileNameWin32                   = 43,
    ProcessImageFileMapping                     = 44,
    ProcessAffinityUpdateMode                   = 45,
    ProcessMemoryAllocationMode                 = 46,
    ProcessGroupInformation                     = 47,
    ProcessTokenVirtualizationEnabled           = 48,
    ProcessConsoleHostProcess                   = 49,
    ProcessWindowInformation                    = 50,
    ProcessHandleInformation                    = 51,
    ProcessMitigationPolicy                     = 52,
    ProcessDynamicFunctionTableInformation      = 53,
    ProcessHandleCheckingMode                   = 54,
    ProcessKeepAliveCount                       = 55,
    ProcessRevokeFileHandles                    = 56,
    ProcessWorkingSetControl                    = 57,
    ProcessHandleTable                          = 58,
    ProcessCheckStackExtentsMode                = 59,
    ProcessCommandLineInformation               = 60,
    ProcessProtectionInformation                = 61,
    ProcessMemoryExhaustion                     = 62,
    ProcessFaultInformation                     = 63,
    ProcessTelemetryIdInformation               = 64,
    ProcessCommitReleaseInformation             = 65,
    ProcessDefaultCpuSetsInformation            = 66,
    ProcessAllowedCpuSetsInformation            = 67,
    ProcessSubsystemProcess                     = 68,
    ProcessJobMemoryInformation                 = 69,
    ProcessInPrivate                            = 70,
    ProcessRaiseUMExceptionOnInvalidHandleClose = 71,
    ProcessIumChallengeResponse                 = 72,
    ProcessChildProcessInformation              = 73,
    ProcessHighGraphicsPriorityInformation      = 74,
    ProcessSubsystemInformation                 = 75,
    ProcessEnergyValues                         = 76,
    ProcessPowerThrottlingState                 = 77,
    ProcessReserved3Information                 = 78,
    ProcessWin32kSyscallFilterInformation       = 79,
    ProcessDisableSystemAllowedCpuSets          = 80,
    ProcessWakeInformation                      = 81,
    ProcessEnergyTrackingState                  = 82,
    ProcessManageWritesToExecutableMemory       = 83,
    ProcessCaptureTrustletLiveDump              = 84,
    ProcessTelemetryCoverage                    = 85,
    ProcessEnclaveInformation                   = 86,
    ProcessEnableReadWriteVmLogging             = 87,
    ProcessUptimeInformation                    = 88,
    ProcessImageSection                         = 89,
    ProcessDebugAuthInformation                 = 90,
    ProcessSystemResourceManagement             = 91,
    ProcessSequenceNumber                       = 92,
    ProcessLoaderDetour                         = 93,
    ProcessSecurityDomainInformation            = 94,
    ProcessCombineSecurityDomainsInformation    = 95,
    ProcessEnableLogging                        = 96,
    ProcessLeapSecondInformation                = 97,
    ProcessFiberShadowStackAllocation           = 98,
    ProcessFreeFiberShadowStackAllocation       = 99,
    ProcessAltSystemCallInformation             = 100,
    ProcessDynamicEHContinuationTargets         = 101,
    ProcessDynamicEnforcedCetCompatibleRanges   = 102,
    ProcessCreateStateChange                    = 103,
    ProcessApplyStateChange                     = 104,
    ProcessEnableOptionalXStateFeatures         = 105,
    ProcessAltPrefetchParam                     = 106,
    ProcessAssignCpuPartitions                  = 107,
    ProcessPriorityClassEx                      = 108,
    ProcessMembershipInformation                = 109,
    ProcessEffectiveIoPriority                  = 110,
    ProcessEffectivePagePriority                = 111,
    ProcessSchedulerSharedData                  = 112,
    ProcessSlistRollbackInformation             = 113,
    ProcessNetworkIoCounters                    = 114,
    ProcessFindFirstThreadByTebValue            = 115,
    ProcessEnclaveAddressSpaceRestriction       = 116,
    ProcessAvailableCpus                        = 117,
    MaxProcessInfoClass,
#ifdef __WINESRC__
    ProcessWineMakeProcessSystem = 1000,
    ProcessWineGrantAdminToken   = 1002,
#endif
} PROCESSINFOCLASS;

#define MEM_EXECUTE_OPTION_DISABLE                 0x01
#define MEM_EXECUTE_OPTION_ENABLE                  0x02
#define MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION 0x04
#define MEM_EXECUTE_OPTION_PERMANENT               0x08

typedef enum _SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation                                = 0,
    SystemCpuInformation                                  = 1,
    SystemPerformanceInformation                          = 2,
    SystemTimeOfDayInformation                            = 3, /* was SystemTimeInformation */
    SystemPathInformation                                 = 4,
    SystemProcessInformation                              = 5,
    SystemCallCountInformation                            = 6,
    SystemDeviceInformation                               = 7,
    SystemProcessorPerformanceInformation                 = 8,
    SystemFlagsInformation                                = 9,
    SystemCallTimeInformation                             = 10,
    SystemModuleInformation                               = 11,
    SystemLocksInformation                                = 12,
    SystemStackTraceInformation                           = 13,
    SystemPagedPoolInformation                            = 14,
    SystemNonPagedPoolInformation                         = 15,
    SystemHandleInformation                               = 16,
    SystemObjectInformation                               = 17,
    SystemPageFileInformation                             = 18,
    SystemVdmInstemulInformation                          = 19,
    SystemVdmBopInformation                               = 20,
    SystemFileCacheInformation                            = 21,
    SystemPoolTagInformation                              = 22,
    SystemInterruptInformation                            = 23,
    SystemDpcBehaviorInformation                          = 24,
    SystemFullMemoryInformation                           = 25,
    SystemNotImplemented6                                 = 25,
    SystemLoadGdiDriverInformation                        = 26,
    SystemUnloadGdiDriverInformation                      = 27,
    SystemTimeAdjustmentInformation                       = 28,
    SystemTimeAdjustment                                  = 28,
    SystemSummaryMemoryInformation                        = 29,
    SystemMirrorMemoryInformation                         = 30,
    SystemPerformanceTraceInformation                     = 31,
    SystemObsolete0                                       = 32,
    SystemExceptionInformation                            = 33,
    SystemCrashDumpStateInformation                       = 34,
    SystemKernelDebuggerInformation                       = 35,
    SystemContextSwitchInformation                        = 36,
    SystemRegistryQuotaInformation                        = 37,
    SystemExtendServiceTableInformation                   = 38,
    SystemPrioritySeparation                              = 39,
    SystemVerifierAddDriverInformation                    = 40,
    SystemVerifierRemoveDriverInformation                 = 41,
    SystemProcessorIdleInformation                        = 42,
    SystemLegacyDriverInformation                         = 43,
    SystemCurrentTimeZoneInformation                      = 44,
    SystemLookasideInformation                            = 45,
    SystemTimeSlipNotification                            = 46,
    SystemSessionCreate                                   = 47,
    SystemSessionDetach                                   = 48,
    SystemSessionInformation                              = 49,
    SystemRangeStartInformation                           = 50,
    SystemVerifierInformation                             = 51,
    SystemVerifierThunkExtend                             = 52,
    SystemSessionProcessesInformation                     = 53,
    SystemLoadGdiDriverInSystemSpace                      = 54,
    SystemNumaProcessorMap                                = 55,
    SystemPrefetcherInformation                           = 56,
    SystemExtendedProcessInformation                      = 57,
    SystemRecommendedSharedDataAlignment                  = 58,
    SystemComPlusPackage                                  = 59,
    SystemNumaAvailableMemory                             = 60,
    SystemProcessorPowerInformation                       = 61,
    SystemEmulationBasicInformation                       = 62,
    SystemEmulationProcessorInformation                   = 63,
    SystemExtendedHandleInformation                       = 64,
    SystemLostDelayedWriteInformation                     = 65,
    SystemBigPoolInformation                              = 66,
    SystemSessionPoolTagInformation                       = 67,
    SystemSessionMappedViewInformation                    = 68,
    SystemHotpatchInformation                             = 69,
    SystemObjectSecurityMode                              = 70,
    SystemWatchdogTimerHandler                            = 71,
    SystemWatchdogTimerInformation                        = 72,
    SystemLogicalProcessorInformation                     = 73,
    SystemWow64SharedInformationObsolete                  = 74,
    SystemRegisterFirmwareTableInformationHandler         = 75,
    SystemFirmwareTableInformation                        = 76,
    SystemModuleInformationEx                             = 77,
    SystemVerifierTriageInformation                       = 78,
    SystemSuperfetchInformation                           = 79,
    SystemMemoryListInformation                           = 80,
    SystemFileCacheInformationEx                          = 81,
    SystemThreadPriorityClientIdInformation               = 82,
    SystemProcessorIdleCycleTimeInformation               = 83,
    SystemVerifierCancellationInformation                 = 84,
    SystemProcessorPowerInformationEx                     = 85,
    SystemRefTraceInformation                             = 86,
    SystemSpecialPoolInformation                          = 87,
    SystemProcessIdInformation                            = 88,
    SystemErrorPortInformation                            = 89,
    SystemBootEnvironmentInformation                      = 90,
    SystemHypervisorInformation                           = 91,
    SystemVerifierInformationEx                           = 92,
    SystemTimeZoneInformation                             = 93,
    SystemImageFileExecutionOptionsInformation            = 94,
    SystemCoverageInformation                             = 95,
    SystemPrefetchPatchInformation                        = 96,
    SystemVerifierFaultsInformation                       = 97,
    SystemSystemPartitionInformation                      = 98,
    SystemSystemDiskInformation                           = 99,
    SystemProcessorPerformanceDistribution                = 100,
    SystemNumaProximityNodeInformation                    = 101,
    SystemDynamicTimeZoneInformation                      = 102,
    SystemCodeIntegrityInformation                        = 103,
    SystemProcessorMicrocodeUpdateInformation             = 104,
    SystemProcessorBrandString                            = 105,
    SystemVirtualAddressInformation                       = 106,
    SystemLogicalProcessorInformationEx                   = 107,
    SystemProcessorCycleTimeInformation                   = 108,
    SystemStoreInformation                                = 109,
    SystemRegistryAppendString                            = 110,
    SystemAitSamplingValue                                = 111,
    SystemVhdBootInformation                              = 112,
    SystemCpuQuotaInformation                             = 113,
    SystemNativeBasicInformation                          = 114,
    SystemErrorPortTimeouts                               = 115,
    SystemLowPriorityIoInformation                        = 116,
    SystemTpmBootEntropyInformation                       = 117,
    SystemVerifierCountersInformation                     = 118,
    SystemPagedPoolInformationEx                          = 119,
    SystemSystemPtesInformationEx                         = 120,
    SystemNodeDistanceInformation                         = 121,
    SystemAcpiAuditInformation                            = 122,
    SystemBasicPerformanceInformation                     = 123,
    SystemQueryPerformanceCounterInformation              = 124,
    SystemSessionBigPoolInformation                       = 125,
    SystemBootGraphicsInformation                         = 126,
    SystemScrubPhysicalMemoryInformation                  = 127,
    SystemBadPageInformation                              = 128,
    SystemProcessorProfileControlArea                     = 129,
    SystemCombinePhysicalMemoryInformation                = 130,
    SystemEntropyInterruptTimingInformation               = 131,
    SystemConsoleInformation                              = 132,
    SystemPlatformBinaryInformation                       = 133,
    SystemPolicyInformation                               = 134,
    SystemHypervisorProcessorCountInformation             = 135,
    SystemDeviceDataInformation                           = 136,
    SystemDeviceDataEnumerationInformation                = 137,
    SystemMemoryTopologyInformation                       = 138,
    SystemMemoryChannelInformation                        = 139,
    SystemBootLogoInformation                             = 140,
    SystemProcessorPerformanceInformationEx               = 141,
    SystemCriticalProcessErrorLogInformation              = 142,
    SystemSecureBootPolicyInformation                     = 143,
    SystemPageFileInformationEx                           = 144,
    SystemSecureBootInformation                           = 145,
    SystemEntropyInterruptTimingRawInformation            = 146,
    SystemPortableWorkspaceEfiLauncherInformation         = 147,
    SystemFullProcessInformation                          = 148,
    SystemKernelDebuggerInformationEx                     = 149,
    SystemBootMetadataInformation                         = 150,
    SystemSoftRebootInformation                           = 151,
    SystemElamCertificateInformation                      = 152,
    SystemOfflineDumpConfigInformation                    = 153,
    SystemProcessorFeaturesInformation                    = 154,
    SystemRegistryReconciliationInformation               = 155,
    SystemEdidInformation                                 = 156,
    SystemManufacturingInformation                        = 157,
    SystemEnergyEstimationConfigInformation               = 158,
    SystemHypervisorDetailInformation                     = 159,
    SystemProcessorCycleStatsInformation                  = 160,
    SystemVmGenerationCountInformation                    = 161,
    SystemTrustedPlatformModuleInformation                = 162,
    SystemKernelDebuggerFlags                             = 163,
    SystemCodeIntegrityPolicyInformation                  = 164,
    SystemIsolatedUserModeInformation                     = 165,
    SystemHardwareSecurityTestInterfaceResultsInformation = 166,
    SystemSingleModuleInformation                         = 167,
    SystemAllowedCpuSetsInformation                       = 168,
    SystemVsmProtectionInformation                        = 169,
    SystemInterruptCpuSetsInformation                     = 170,
    SystemSecureBootPolicyFullInformation                 = 171,
    SystemCodeIntegrityPolicyFullInformation              = 172,
    SystemAffinitizedInterruptProcessorInformation        = 173,
    SystemRootSiloInformation                             = 174,
    SystemCpuSetInformation                               = 175,
    SystemCpuSetTagInformation                            = 176,
    SystemWin32WerStartCallout                            = 177,
    SystemSecureKernelProfileInformation                  = 178,
    SystemCodeIntegrityPlatformManifestInformation        = 179,
    SystemInterruptSteeringInformation                    = 180,
    SystemSupportedProcessorArchitectures                 = 181,
    SystemMemoryUsageInformation                          = 182,
    SystemCodeIntegrityCertificateInformation             = 183,
    SystemPhysicalMemoryInformation                       = 184,
    SystemControlFlowTransition                           = 185,
    SystemKernelDebuggingAllowed                          = 186,
    SystemActivityModerationExeState                      = 187,
    SystemActivityModerationUserSettings                  = 188,
    SystemCodeIntegrityPoliciesFullInformation            = 189,
    SystemCodeIntegrityUnlockInformation                  = 190,
    SystemIntegrityQuotaInformation                       = 191,
    SystemFlushInformation                                = 192,
    SystemProcessorIdleMaskInformation                    = 193,
    SystemSecureDumpEncryptionInformation                 = 194,
    SystemWriteConstraintInformation                      = 195,
    SystemKernelVaShadowInformation                       = 196,
    SystemHypervisorSharedPageInformation                 = 197,
    SystemFirmwareBootPerformanceInformation              = 198,
    SystemCodeIntegrityVerificationInformation            = 199,
    SystemFirmwarePartitionInformation                    = 200,
    SystemSpeculationControlInformation                   = 201,
    SystemDmaGuardPolicyInformation                       = 202,
    SystemEnclaveLaunchControlInformation                 = 203,
    SystemWorkloadAllowedCpuSetsInformation               = 204,
    SystemCodeIntegrityUnlockModeInformation              = 205,
    SystemLeapSecondInformation                           = 206,
    SystemFlags2Information                               = 207,
    SystemSecurityModelInformation                        = 208,
    SystemCodeIntegritySyntheticCacheInformation          = 209,
    SystemFeatureConfigurationInformation                 = 210,
    SystemFeatureConfigurationSectionInformation          = 211,
    SystemFeatureUsageSubscriptionInformation             = 212,
    SystemSecureSpeculationControlInformation             = 213,
    SystemSpacesBootInformation                           = 214,
    SystemFwRamdiskInformation                            = 215,
    SystemWheaIpmiHardwareInformation                     = 216,
    SystemDifSetRuleClassInformation                      = 217,
    SystemDifClearRuleClassInformation                    = 218,
    SystemDifApplyPluginVerificationOnDriver              = 219,
    SystemDifRemovePluginVerificationOnDriver             = 220,
    SystemShadowStackInformation                          = 221,
    SystemBuildVersionInformation                         = 222,
    SystemPoolLimitInformation                            = 223,
    SystemCodeIntegrityAddDynamicStore                    = 224,
    SystemCodeIntegrityClearDynamicStores                 = 225,
    SystemDifPoolTrackingInformation                      = 226,
    SystemPoolZeroingInformation                          = 227,
    SystemDpcWatchdogInformation                          = 228,
    SystemDpcWatchdogInformation2                         = 229,
    SystemSupportedProcessorArchitectures2                = 230,
    SystemSingleProcessorRelationshipInformation          = 231,
    SystemXfgCheckFailureInformation                      = 232,
    SystemIommuStateInformation                           = 233,
    SystemHypervisorMinrootInformation                    = 234,
    SystemHypervisorBootPagesInformation                  = 235,
    SystemPointerAuthInformation                          = 236,
    SystemSecureKernelDebuggerInformation                 = 237,
    SystemOriginalImageFeatureInformation                 = 238,
    SystemMemoryNumaInformation                           = 239,
    SystemMemoryNumaPerformanceInformation                = 240,
    SystemCodeIntegritySignedPoliciesFullInformation      = 241,
    SystemSecureCoreInformation                           = 242,
    SystemTrustedAppsRuntimeInformation                   = 243,
    SystemBadPageInformationEx                            = 244,
    SystemResourceDeadlockTimeout                         = 245,
    SystemBreakOnContextUnwindFailureInformation          = 246,
    SystemOslRamdiskInformation                           = 247,
    SystemCodeIntegrityPolicyManagementInformation        = 248,
    SystemMemoryNumaCacheInformation                      = 249,
    SystemProcessorFeaturesBitMapInformation              = 250,
    SystemRefTraceInformationEx                           = 251,
    SystemBasicProcessInformation                         = 252,
    SystemHandleCountInformation                          = 253,
#ifdef __WINESRC__
    SystemWineVersionInformation = 1000,
#endif
} SYSTEM_INFORMATION_CLASS,
    *PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_CODEINTEGRITY_INFORMATION
{
    ULONG Length;
    ULONG CodeIntegrityOptions;
} SYSTEM_CODEINTEGRITY_INFORMATION, *PSYSTEM_CODEINTEGRITY_INFORMATION;

#define CODEINTEGRITY_OPTION_ENABLED                      0x0001
#define CODEINTEGRITY_OPTION_TESTSIGN                     0x0002
#define CODEINTEGRITY_OPTION_UMCI_ENABLED                 0x0004
#define CODEINTEGRITY_OPTION_UMCI_AUDITMODE_ENABLED       0x0008
#define CODEINTEGRITY_OPTION_UMCI_EXCLUSIONPATHS_ENABLED  0x0010
#define CODEINTEGRITY_OPTION_TEST_BUILD                   0x0020
#define CODEINTEGRITY_OPTION_PREPRODUCTION_BUILD          0x0040
#define CODEINTEGRITY_OPTION_DEBUGMODE_ENABLED            0x0080
#define CODEINTEGRITY_OPTION_FLIGHT_BUILD                 0x0100
#define CODEINTEGRITY_OPTION_FLIGHTING_ENABLED            0x0200
#define CODEINTEGRITY_OPTION_HVCI_KMCI_ENABLED            0x0400
#define CODEINTEGRITY_OPTION_HVCI_KMCI_AUDITMODE_ENABLED  0x0800
#define CODEINTEGRITY_OPTION_HVCI_KMCI_STRICTMODE_ENABLED 0x1000
#define CODEINTEGRITY_OPTION_HVCI_IUM_ENABLED             0x2000

typedef enum _THREADINFOCLASS
{
    ThreadBasicInformation               = 0,
    ThreadTimes                          = 1,
    ThreadPriority                       = 2,
    ThreadBasePriority                   = 3,
    ThreadAffinityMask                   = 4,
    ThreadImpersonationToken             = 5,
    ThreadDescriptorTableEntry           = 6,
    ThreadEnableAlignmentFaultFixup      = 7,
    ThreadEventPair_Reusable             = 8,
    ThreadQuerySetWin32StartAddress      = 9,
    ThreadZeroTlsCell                    = 10,
    ThreadPerformanceCount               = 11,
    ThreadAmILastThread                  = 12,
    ThreadIdealProcessor                 = 13,
    ThreadPriorityBoost                  = 14,
    ThreadSetTlsArrayAddress             = 15,
    ThreadIsIoPending                    = 16,
    ThreadHideFromDebugger               = 17,
    ThreadBreakOnTermination             = 18,
    ThreadSwitchLegacyState              = 19,
    ThreadIsTerminated                   = 20,
    ThreadLastSystemCall                 = 21,
    ThreadIoPriority                     = 22,
    ThreadCycleTime                      = 23,
    ThreadPagePriority                   = 24,
    ThreadActualBasePriority             = 25,
    ThreadTebInformation                 = 26,
    ThreadCSwitchMon                     = 27,
    ThreadCSwitchPmu                     = 28,
    ThreadWow64Context                   = 29,
    ThreadGroupInformation               = 30,
    ThreadUmsInformation                 = 31,
    ThreadCounterProfiling               = 32,
    ThreadIdealProcessorEx               = 33,
    ThreadCpuAccountingInformation       = 34,
    ThreadSuspendCount                   = 35,
    ThreadHeterogeneousCpuPolicy         = 36,
    ThreadContainerId                    = 37,
    ThreadNameInformation                = 38,
    ThreadSelectedCpuSets                = 39,
    ThreadSystemThreadInformation        = 40,
    ThreadActualGroupAffinity            = 41,
    ThreadDynamicCodePolicyInfo          = 42,
    ThreadExplicitCaseSensitivity        = 43,
    ThreadWorkOnBehalfTicket             = 44,
    ThreadSubsystemInformation           = 45,
    ThreadDbgkWerReportActive            = 46,
    ThreadAttachContainer                = 47,
    ThreadManageWritesToExecutableMemory = 48,
    ThreadPowerThrottlingState           = 49,
    ThreadWorkloadClass                  = 50,
    ThreadCreateStateChange              = 51,
    ThreadApplyStateChange               = 52,
    ThreadStrongerBadHandleChecks        = 53,
    ThreadEffectiveIoPriority            = 54,
    ThreadEffectivePagePriority          = 55,
    ThreadUpdateLockOwnership            = 56,
    ThreadSchedulerSharedDataSlot        = 57,
    ThreadTebInformationAtomic           = 58,
    ThreadIndexInformation               = 59,
    MaxThreadInfoClass,
#ifdef __WINESRC__
    ThreadWineNativeThreadName = 1000,
#endif
} THREADINFOCLASS;

typedef struct _THREAD_BASIC_INFORMATION
{
    NTSTATUS  ExitStatus;
    PVOID     TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    LONG      Priority;
    LONG      BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef struct _THREAD_DESCRIPTOR_INFORMATION
{
    DWORD     Selector;
    LDT_ENTRY Entry;
} THREAD_DESCRIPTOR_INFORMATION, *PTHREAD_DESCRIPTOR_INFORMATION;

typedef struct _THREAD_NAME_INFORMATION
{
    UNICODE_STRING ThreadName;
} THREAD_NAME_INFORMATION, *PTHREAD_NAME_INFORMATION;

typedef struct _MANAGE_WRITES_TO_EXECUTABLE_MEMORY
{
    ULONG Version                      : 8;
    ULONG ProcessEnableWriteExceptions : 1;
    ULONG ThreadAllowWrites            : 1;
    ULONG Spare                        : 22;
    PVOID KernelWriteToExecutableSignal;
} MANAGE_WRITES_TO_EXECUTABLE_MEMORY, *PMANAGE_WRITES_TO_EXECUTABLE_MEMORY;

typedef struct _KERNEL_USER_TIMES
{
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES, *PKERNEL_USER_TIMES;

typedef enum _WINSTATIONINFOCLASS
{
    WinStationInformation = 8
} WINSTATIONINFOCLASS;

typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation              = 0,
    MemoryWorkingSetInformation         = 1,
    MemoryMappedFilenameInformation     = 2,
    MemoryRegionInformation             = 3,
    MemoryWorkingSetExInformation       = 4,
    MemorySharedCommitInformation       = 5,
    MemoryImageInformation              = 6,
    MemoryRegionInformationEx           = 7,
    MemoryPrivilegedBasicInformation    = 8,
    MemoryEnclaveImageInformation       = 9,
    MemoryBasicInformationCapped        = 10,
    MemoryPhysicalContiguityInformation = 11,
    MemoryBadInformation                = 12,
    MemoryBadInformationAllProcesses    = 13,
    MemoryImageExtensionInformation     = 14,
#ifdef __WINESRC__
    MemoryWineLoadUnixLib = 1000,
    MemoryWineLoadUnixLibWow64,
    MemoryWineLoadUnixLibByName,
    MemoryWineLoadUnixLibByNameWow64,
    MemoryWineUnloadUnixLib,
#endif
} MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_SECTION_NAME
{
    UNICODE_STRING SectionFileName;
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

typedef union _MEMORY_WORKING_SET_EX_BLOCK
{
    ULONG_PTR Flags;

    struct
    {
        ULONG_PTR Valid           : 1;
        ULONG_PTR ShareCount      : 3;
        ULONG_PTR Win32Protection : 11;
        ULONG_PTR Shared          : 1;
        ULONG_PTR Node            : 6;
        ULONG_PTR Locked          : 1;
        ULONG_PTR LargePage       : 1;
    } DUMMYSTRUCTNAME;
} MEMORY_WORKING_SET_EX_BLOCK, *PMEMORY_WORKING_SET_EX_BLOCK;

typedef struct _MEMORY_WORKING_SET_EX_INFORMATION
{
    PVOID                       VirtualAddress;
    MEMORY_WORKING_SET_EX_BLOCK VirtualAttributes;
} MEMORY_WORKING_SET_EX_INFORMATION, *PMEMORY_WORKING_SET_EX_INFORMATION;

typedef struct _MEMORY_REGION_INFORMATION
{
    PVOID AllocationBase;
    ULONG AllocationProtect;

    union
    {
        ULONG RegionType;

        struct
        {
            ULONG Private        : 1;
            ULONG MappedDataFile : 1;
            ULONG MappedImage    : 1;
            ULONG MappedPageFile : 1;
            ULONG MappedPhysical : 1;
            ULONG DirectMapped   : 1;
            ULONG Reserved       : 26;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    SIZE_T    RegionSize;
    SIZE_T    CommitSize;
    ULONG_PTR PartitionId;
    ULONG_PTR NodePreference;
} MEMORY_REGION_INFORMATION, *PMEMORY_REGION_INFORMATION;

typedef struct _MEMORY_IMAGE_INFORMATION
{
    PVOID  ImageBase;
    SIZE_T SizeOfImage;

    union
    {
        ULONG ImageFlags;

        struct
        {
            ULONG ImagePartialMap    : 1;
            ULONG ImageNotExecutable : 1;
            ULONG ImageSigningLevel  : 4;
            ULONG Reserved           : 26;
        };
    };
} MEMORY_IMAGE_INFORMATION, *PMEMORY_IMAGE_INFORMATION;

typedef enum _MUTANT_INFORMATION_CLASS
{
    MutantBasicInformation
} MUTANT_INFORMATION_CLASS,
    *PMUTANT_INFORMATION_CLASS;

typedef struct _MUTANT_BASIC_INFORMATION
{
    LONG    CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

typedef enum _TIMER_INFORMATION_CLASS
{
    TimerBasicInformation = 0
} TIMER_INFORMATION_CLASS;

typedef struct _TIMER_BASIC_INFORMATION
{
    LARGE_INTEGER RemainingTime;
    BOOLEAN       TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

typedef enum
{
    VmPrefetchInformation,
    VmPagePriorityInformation,
    VmCfgCallTargetInformation,
    VmPageDirtyStateInformation,
    VmImageHotPatchInformation,
    VmPhysicalContiguityInformation,
    VmVirtualMachinePrepopulateInformation,
    VmRemoveFromWorkingSetInformation,
} VIRTUAL_MEMORY_INFORMATION_CLASS,
    *PVIRTUAL_MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_RANGE_ENTRY
{
    PVOID  VirtualAddress;
    SIZE_T NumberOfBytes;
} MEMORY_RANGE_ENTRY, *PMEMORY_RANGE_ENTRY;

/***********************************************************************
 * Types and data structures
 */

/* This is used by NtQuerySystemInformation */
typedef struct _SYSTEM_THREAD_INFORMATION
{                                    /* win32/win64 */
    LARGE_INTEGER KernelTime;        /* 00/00 */
    LARGE_INTEGER UserTime;          /* 08/08 */
    LARGE_INTEGER CreateTime;        /* 10/10 */
    DWORD         dwTickCount;       /* 18/18 */
    LPVOID        StartAddress;      /* 1c/20 */
    CLIENT_ID     ClientId;          /* 20/28 */
    DWORD         dwCurrentPriority; /* 28/38 */
    DWORD         dwBasePriority;    /* 2c/3c */
    DWORD         dwContextSwitches; /* 30/40 */
    DWORD         dwThreadState;     /* 34/44 */
    DWORD         dwWaitReason;      /* 38/48 */
    DWORD         dwUnknown;         /* 3c/4c */
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION
{
    SYSTEM_THREAD_INFORMATION ThreadInfo;        /* 00/00 */
    void                     *StackBase;         /* 40/50 */
    void                     *StackLimit;        /* 44/58 */
    void                     *Win32StartAddress; /* 48/60 */
    void                     *TebBase;           /* 4c/68 */
    ULONG_PTR                 Reserved2;         /* 50/70 */
    ULONG_PTR                 Reserved3;         /* 54/78 */
    ULONG_PTR                 Reserved4;         /* 58/80 */
} SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

typedef struct _IO_STATUS_BLOCK
{
    union
    {
        NTSTATUS Status;
        PVOID    Pointer;
    } DUMMYUNIONNAME;

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef void(WINAPI *PIO_APC_ROUTINE)(PVOID, PIO_STATUS_BLOCK, ULONG);

typedef struct _KEY_BASIC_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG         TitleIndex;
    ULONG         NameLength;
    WCHAR         Name[1];
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG         TitleIndex;
    ULONG         ClassOffset;
    ULONG         ClassLength;
    ULONG         NameLength;
    WCHAR         Name[1];
    /* Class[1]; */
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG         TitleIndex;
    ULONG         ClassOffset;
    ULONG         ClassLength;
    ULONG         SubKeys;
    ULONG         MaxNameLen;
    ULONG         MaxClassLen;
    ULONG         Values;
    ULONG         MaxValueNameLen;
    ULONG         MaxValueDataLen;
    WCHAR         Class[1];
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

typedef struct _KEY_NAME_INFORMATION
{
    ULONG NameLength;
    WCHAR Name[1];
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG         TitleIndex;
    ULONG         SubKeys;
    ULONG         MaxNameLen;
    ULONG         Values;
    ULONG         MaxValueNameLen;
    ULONG         MaxValueDataLen;
    ULONG         NameLength;
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_VALUE_ENTRY
{
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef struct _KEY_VALUE_BASIC_INFORMATION
{
    ULONG TitleIndex;
    ULONG Type;
    ULONG NameLength;
    WCHAR Name[1];
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION
{
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataOffset;
    ULONG DataLength;
    ULONG NameLength;
    WCHAR Name[1];
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION
{
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataLength;
    UCHAR Data[1];
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64
{
    ULONG Type;
    ULONG DataLength;
    UCHAR Data[1];
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef enum _MEMORY_RESERVE_OBJECT_TYPE
{
    MemoryReserveObjectTypeUserApc,
    MemoryReserveObjectTypeIoCompletion
} MEMORY_RESERVE_OBJECT_TYPE,
    PMEMORY_RESERVE_OBJECT_TYPE;

#ifndef __OBJECT_ATTRIBUTES_DEFINED__
#define __OBJECT_ATTRIBUTES_DEFINED__

typedef struct _OBJECT_ATTRIBUTES
{
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;       /* type SECURITY_DESCRIPTOR */
    PVOID           SecurityQualityOfService; /* type SECURITY_QUALITY_OF_SERVICE */
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
#endif

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION
{
    BOOLEAN Inherit;
    BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;

typedef struct _OBJECT_BASIC_INFORMATION
{
    ULONG         Attributes;
    ACCESS_MASK   GrantedAccess;
    ULONG         HandleCount;
    ULONG         PointerCount;
    ULONG         PagedPoolUsage;
    ULONG         NonPagedPoolUsage;
    ULONG         Reserved[3];
    ULONG         NameInformationLength;
    ULONG         TypeInformationLength;
    ULONG         SecurityDescriptorLength;
    LARGE_INTEGER CreateTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION
{
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct __OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING  TypeName;
    ULONG           TotalNumberOfObjects;
    ULONG           TotalNumberOfHandles;
    ULONG           TotalPagedPoolUsage;
    ULONG           TotalNonPagedPoolUsage;
    ULONG           TotalNamePoolUsage;
    ULONG           TotalHandleTableUsage;
    ULONG           HighWaterNumberOfObjects;
    ULONG           HighWaterNumberOfHandles;
    ULONG           HighWaterPagedPoolUsage;
    ULONG           HighWaterNonPagedPoolUsage;
    ULONG           HighWaterNamePoolUsage;
    ULONG           HighWaterHandleTableUsage;
    ULONG           InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG           ValidAccessMask;
    BOOLEAN         SecurityRequired;
    BOOLEAN         MaintainHandleCount;
    UCHAR           TypeIndex;
    CHAR            ReservedByte;
    ULONG           PoolType;
    ULONG           DefaultPagedPoolCharge;
    ULONG           DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_TYPES_INFORMATION
{
    ULONG NumberOfTypes;
} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;

typedef struct _PROCESS_BASIC_INFORMATION64
{
    NTSTATUS ExitStatus;
    UINT64   PebBaseAddress;
    UINT64   AffinityMask;
    LONG     BasePriority;
    UINT64   UniqueProcessId;
    UINT64   InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;

#define PROCESS_PRIOCLASS_IDLE         1
#define PROCESS_PRIOCLASS_NORMAL       2
#define PROCESS_PRIOCLASS_HIGH         3
#define PROCESS_PRIOCLASS_REALTIME     4
#define PROCESS_PRIOCLASS_BELOW_NORMAL 5
#define PROCESS_PRIOCLASS_ABOVE_NORMAL 6

typedef struct _PROCESS_PRIORITY_CLASS
{
    BOOLEAN Foreground;
    UCHAR   PriorityClass;
} PROCESS_PRIORITY_CLASS, *PPROCESS_PRIORITY_CLASS;

typedef struct _PROCESS_CYCLE_TIME_INFORMATION
{
    ULONGLONG AccumulatedCycles;
    ULONGLONG CurrentCycleCount;
} PROCESS_CYCLE_TIME_INFORMATION, *PPROCESS_CYCLE_TIME_INFORMATION;

typedef struct _PROCESS_STACK_ALLOCATION_INFORMATION
{
    SIZE_T ReserveSize;
    SIZE_T ZeroBits;
    PVOID  StackBase;
} PROCESS_STACK_ALLOCATION_INFORMATION, *PPROCESS_STACK_ALLOCATION_INFORMATION;

typedef struct _PROCESS_STACK_ALLOCATION_INFORMATION_EX
{
    ULONG                                PreferredNode;
    ULONG                                Reserved0;
    ULONG                                Reserved1;
    ULONG                                Reserved2;
    PROCESS_STACK_ALLOCATION_INFORMATION AllocInfo;
} PROCESS_STACK_ALLOCATION_INFORMATION_EX, *PPROCESS_STACK_ALLOCATION_INFORMATION_EX;

/* System Information Class 0x00 */

typedef struct _SYSTEM_BASIC_INFORMATION
{
#ifdef __WINESRC__
    DWORD     unknown;
    ULONG     KeMaximumIncrement;
    ULONG     PageSize;
    ULONG     MmNumberOfPhysicalPages;
    ULONG     MmLowestPhysicalPage;
    ULONG     MmHighestPhysicalPage;
    ULONG_PTR AllocationGranularity;
    PVOID     LowestUserAddress;
    PVOID     HighestUserAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    BYTE      NumberOfProcessors;
#else
    BYTE  Reserved1[24];
    PVOID Reserved2[4];
    CCHAR NumberOfProcessors;
#endif
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

/* System Information Class 0x01 */

typedef struct _SYSTEM_CPU_INFORMATION
{
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT MaximumProcessors;
    ULONG  ProcessorFeatureBits;
} SYSTEM_CPU_INFORMATION, *PSYSTEM_CPU_INFORMATION;

/* System Information Class 0x02 */

/* Documented in "Windows NT/2000 Native API Reference" by Gary Nebbett. */
typedef struct _SYSTEM_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    ULONG         ReadOperationCount;
    ULONG         WriteOperationCount;
    ULONG         OtherOperationCount;
    ULONG         AvailablePages;
    ULONG         TotalCommittedPages;
    ULONG         TotalCommitLimit;
    ULONG         PeakCommitment;
    ULONG         PageFaults;
    ULONG         WriteCopyFaults;
    ULONG         TransitionFaults;
    ULONG         Reserved1;
    ULONG         DemandZeroFaults;
    ULONG         PagesRead;
    ULONG         PageReadIos;
    ULONG         Reserved2[2];
    ULONG         PagefilePagesWritten;
    ULONG         PagefilePageWriteIos;
    ULONG         MappedFilePagesWritten;
    ULONG         MappedFilePageWriteIos;
    ULONG         PagedPoolUsage;
    ULONG         NonPagedPoolUsage;
    ULONG         PagedPoolAllocs;
    ULONG         PagedPoolFrees;
    ULONG         NonPagedPoolAllocs;
    ULONG         NonPagedPoolFrees;
    ULONG         TotalFreeSystemPtes;
    ULONG         SystemCodePage;
    ULONG         TotalSystemDriverPages;
    ULONG         TotalSystemCodePages;
    ULONG         SmallNonPagedLookasideListAllocateHits;
    ULONG         SmallPagedLookasideListAllocateHits;
    ULONG         Reserved3;
    ULONG         MmSystemCachePage;
    ULONG         PagedPoolPage;
    ULONG         SystemDriverPage;
    ULONG         FastReadNoWait;
    ULONG         FastReadWait;
    ULONG         FastReadResourceMiss;
    ULONG         FastReadNotPossible;
    ULONG         FastMdlReadNoWait;
    ULONG         FastMdlReadWait;
    ULONG         FastMdlReadResourceMiss;
    ULONG         FastMdlReadNotPossible;
    ULONG         MapDataNoWait;
    ULONG         MapDataWait;
    ULONG         MapDataNoWaitMiss;
    ULONG         MapDataWaitMiss;
    ULONG         PinMappedDataCount;
    ULONG         PinReadNoWait;
    ULONG         PinReadWait;
    ULONG         PinReadNoWaitMiss;
    ULONG         PinReadWaitMiss;
    ULONG         CopyReadNoWait;
    ULONG         CopyReadWait;
    ULONG         CopyReadNoWaitMiss;
    ULONG         CopyReadWaitMiss;
    ULONG         MdlReadNoWait;
    ULONG         MdlReadWait;
    ULONG         MdlReadNoWaitMiss;
    ULONG         MdlReadWaitMiss;
    ULONG         ReadAheadIos;
    ULONG         LazyWriteIos;
    ULONG         LazyWritePages;
    ULONG         DataFlushes;
    ULONG         DataPages;
    ULONG         ContextSwitches;
    ULONG         FirstLevelTbFills;
    ULONG         SecondLevelTbFills;
    ULONG         SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

/* System Information Class 0x03 */

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION
{
#ifdef __WINESRC__
    LARGE_INTEGER BootTime;
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG         TimeZoneId;
    ULONG         Reserved;
    ULONGLONG     BootTimeBias;
    ULONGLONG     SleepTimeBias;
#else
    BYTE Reserved1[48];
#endif
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION; /* was SYSTEM_TIME_INFORMATION */

/* System Information Class 0x08 */

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG         Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

/* System Information Class 0x0b */

typedef struct _SYSTEM_DRIVER_INFORMATION
{
    PVOID pvAddress;
    DWORD dwUnknown1;
    DWORD dwUnknown2;
    DWORD dwEntryIndex;
    DWORD dwUnknown3;
    char  szName[MAX_PATH + 1];
} SYSTEM_DRIVER_INFORMATION, *PSYSTEM_DRIVER_INFORMATION;

/* System Information Class 0x10 */

typedef struct _SYSTEM_HANDLE_ENTRY
{
    ULONG  OwnerPid;
    BYTE   ObjectType;
    BYTE   HandleFlags;
    USHORT HandleValue;
    PVOID  ObjectPointer;
    ULONG  AccessMask;
} SYSTEM_HANDLE_ENTRY, *PSYSTEM_HANDLE_ENTRY;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG               Count;
    SYSTEM_HANDLE_ENTRY Handle[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
    void     *Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG     GrantedAccess;
    USHORT    CreatorBackTraceIndex;
    USHORT    ObjectTypeIndex;
    ULONG     HandleAttributes;
    ULONG     Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
    ULONG_PTR                         NumberOfHandles;
    ULONG_PTR                         Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX;

/* System Information Class 0x15 */

typedef struct _SYSTEM_CACHE_INFORMATION
{
    SIZE_T CurrentSize;
    SIZE_T PeakSize;
    ULONG  PageFaultCount;
    SIZE_T MinimumWorkingSet;
    SIZE_T MaximumWorkingSet;
    SIZE_T CurrentSizeIncludingTransitionInPages;
    SIZE_T PeakSizeIncludingTransitionInPages;
    ULONG  TransitionRePurposeCount;
    ULONG  Flags;
} SYSTEM_CACHE_INFORMATION, *PSYSTEM_CACHE_INFORMATION;

/* System Information Class 0x17 */

typedef struct _SYSTEM_INTERRUPT_INFORMATION
{
    ULONG ContextSwitches;
    ULONG DpcCount;
    ULONG DpcRate;
    ULONG TimeIncrement;
    ULONG DpcBypassCount;
    ULONG ApcBypassCount;
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef struct _SYSTEM_CONFIGURATION_INFO
{
    union
    {
        ULONG OemId;

        struct
        {
            WORD ProcessorArchitecture;
            WORD Reserved;
        } tag1;
    } tag2;

    ULONG PageSize;
    PVOID MinimumApplicationAddress;
    PVOID MaximumApplicationAddress;
    ULONG ActiveProcessorMask;
    ULONG NumberOfProcessors;
    ULONG ProcessorType;
    ULONG AllocationGranularity;
    WORD  ProcessorLevel;
    WORD  ProcessorRevision;
} SYSTEM_CONFIGURATION_INFO, *PSYSTEM_CONFIGURATION_INFO;

typedef struct _SYSTEM_EXCEPTION_INFORMATION
{
    BYTE Reserved1[16];
} SYSTEM_EXCEPTION_INFORMATION, *PSYSTEM_EXCEPTION_INFORMATION;

typedef struct _SYSTEM_LOOKASIDE_INFORMATION
{
    BYTE Reserved1[32];
} SYSTEM_LOOKASIDE_INFORMATION, *PSYSTEM_LOOKASIDE_INFORMATION;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION
{
    BOOLEAN DebuggerEnabled;
    BOOLEAN DebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION_EX
{
    BOOLEAN DebuggerAllowed;
    BOOLEAN DebuggerEnabled;
    BOOLEAN DebuggerPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION_EX, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION_EX;

typedef struct _SYSTEM_LEAP_SECOND_INFORMATION
{
    BOOLEAN Enabled;
    ULONG   Flags;
} SYSTEM_LEAP_SECOND_INFORMATION, *PSYSTEM_LEAP_SECOND_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_FEATURES_INFORMATION
{
    ULONGLONG ProcessorFeatureBits;
    ULONGLONG Reserved[3];
} SYSTEM_PROCESSOR_FEATURES_INFORMATION, *PSYSTEM_PROCESSOR_FEATURES_INFORMATION;

typedef struct _VM_COUNTERS
{
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _VM_COUNTERS_EX
{
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivateUsage;
} VM_COUNTERS_EX, *PVM_COUNTERS_EX;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
#ifdef __WINESRC__                                          /* win32/win64 */
    ULONG                     NextEntryOffset;              /* 00/00 */
    DWORD                     dwThreadCount;                /* 04/04 */
    LARGE_INTEGER             WorkingSetPrivateSize;        /* 08/08 */
    ULONG                     HardFaultCount;               /* 10/10 */
    ULONG                     NumberOfThreadsHighWatermark; /* 14/14 */
    ULONGLONG                 CycleTime;                    /* 18/18 */
    LARGE_INTEGER             CreationTime;                 /* 20/20 */
    LARGE_INTEGER             UserTime;                     /* 28/28 */
    LARGE_INTEGER             KernelTime;                   /* 30/30 */
    UNICODE_STRING            ProcessName;                  /* 38/38 */
    DWORD                     dwBasePriority;               /* 40/48 */
    HANDLE                    UniqueProcessId;              /* 44/50 */
    HANDLE                    ParentProcessId;              /* 48/58 */
    ULONG                     HandleCount;                  /* 4c/60 */
    ULONG                     SessionId;                    /* 50/64 */
    ULONG_PTR                 UniqueProcessKey;             /* 54/68 */
    VM_COUNTERS_EX            vmCounters;                   /* 58/70 */
    IO_COUNTERS               ioCounters;                   /* 88/d0 */
    SYSTEM_THREAD_INFORMATION ti[1];                        /* b8/100 */
#else
    ULONG         NextEntryOffset;   /* 00/00 */
    BYTE          Reserved1[52];     /* 04/04 */
    PVOID         Reserved2[3];      /* 38/38 */
    HANDLE        UniqueProcessId;   /* 44/50 */
    PVOID         Reserved3;         /* 48/58 */
    ULONG         HandleCount;       /* 4c/60 */
    BYTE          Reserved4[4];      /* 50/64 */
    PVOID         Reserved5[11];     /* 54/68 */
    SIZE_T        PeakPagefileUsage; /* 80/c0 */
    SIZE_T        PrivatePageCount;  /* 84/c8 */
    LARGE_INTEGER Reserved6[6];      /* 88/d0 */
#endif
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_REGISTRY_QUOTA_INFORMATION
{
    ULONG RegistryQuotaAllowed;
    ULONG RegistryQuotaUsed;
    PVOID Reserved1;
} SYSTEM_REGISTRY_QUOTA_INFORMATION, *PSYSTEM_REGISTRY_QUOTA_INFORMATION;

typedef struct _SYSTEM_TIME_ADJUSTMENT_QUERY
{
    ULONG   TimeAdjustment;
    ULONG   TimeIncrement;
    BOOLEAN TimeAdjustmentDisabled;
} SYSTEM_TIME_ADJUSTMENT_QUERY, *PSYSTEM_TIME_ADJUSTMENT_QUERY;

typedef struct _SYSTEM_TIME_ADJUSTMENT
{
    ULONG   TimeAdjustment;
    BOOLEAN TimeAdjustmentDisabled;
} SYSTEM_TIME_ADJUSTMENT, *PSYSTEM_TIME_ADJUSTMENT;

typedef enum _SYSTEM_FIRMWARE_TABLE_ACTION
{
    SystemFirmwareTable_Enumerate = 0,
    SystemFirmwareTable_Get       = 1
} SYSTEM_FIRMWARE_TABLE_ACTION,
    *PSYSTEM_FIRMWARE_TABLE_ACTION;

/* System Information Class 0x4C */

typedef struct _SYSTEM_FIRMWARE_TABLE_INFORMATION
{
    ULONG                        ProviderSignature;
    SYSTEM_FIRMWARE_TABLE_ACTION Action;
    ULONG                        TableID;
    ULONG                        TableBufferLength;
    UCHAR                        TableBuffer[1];
} SYSTEM_FIRMWARE_TABLE_INFORMATION, *PSYSTEM_FIRMWARE_TABLE_INFORMATION;

typedef struct _SYSTEM_PROCESS_ID_INFORMATION
{
    ULONG_PTR      ProcessId;
    UNICODE_STRING ImageName;
} SYSTEM_PROCESS_ID_INFORMATION, *PSYSTEM_PROCESS_ID_INFORMATION;

typedef struct _TIME_FIELDS
{
    CSHORT Year;
    CSHORT Month;
    CSHORT Day;
    CSHORT Hour;
    CSHORT Minute;
    CSHORT Second;
    CSHORT Milliseconds;
    CSHORT Weekday;
} TIME_FIELDS, *PTIME_FIELDS;

typedef struct _WINSTATIONINFORMATIONW
{
    BYTE  Reserved2[70];
    ULONG LogonId;
    BYTE  Reserved3[1140];
} WINSTATIONINFORMATIONW, *PWINSTATIONINFORMATIONW;

typedef BOOLEAN(WINAPI *PWINSTATIONQUERYINFORMATIONW)(HANDLE, ULONG, WINSTATIONINFOCLASS, PVOID, ULONG, PULONG);

/* debug buffer definitions */

typedef struct _DEBUG_BUFFER
{
    HANDLE SectionHandle;
    PVOID  SectionBase;
    PVOID  RemoteSectionBase;
    ULONG  SectionBaseDelta;
    HANDLE EventPairHandle;
    SIZE_T Unknown[2];
    HANDLE RemoteThreadHandle;
    ULONG  InfoClassMask;
    SIZE_T SizeOfInfo;
    SIZE_T AllocatedSize;
    ULONG  SectionSize;
    PVOID  ModuleInformation;
    PVOID  BackTraceInformation;
    PVOID  HeapInformation;
    PVOID  LockInformation;
    PVOID  Reserved[8];
} DEBUG_BUFFER, *PDEBUG_BUFFER;

#define PDI_MODULES     0x01
#define PDI_BACKTRACE   0x02
#define PDI_HEAPS       0x04
#define PDI_HEAP_TAGS   0x08
#define PDI_HEAP_BLOCKS 0x10
#define PDI_LOCKS       0x20

typedef struct _DEBUG_MODULE_INFORMATION
{
    ULONG  Reserved[2];
    ULONG  Base;
    ULONG  Size;
    ULONG  Flags;
    USHORT Index;
    USHORT Unknown;
    USHORT LoadCount;
    USHORT ModuleNameOffset;
    CHAR   ImageName[256];
} DEBUG_MODULE_INFORMATION, *PDEBUG_MODULE_INFORMATION;

typedef struct _DEBUG_HEAP_INFORMATION
{
    ULONG  Base;
    ULONG  Flags;
    USHORT Granularity;
    USHORT Unknown;
    ULONG  Allocated;
    ULONG  Committed;
    ULONG  TagCount;
    ULONG  BlockCount;
    ULONG  Reserved[7];
    PVOID  Tags;
    PVOID  Blocks;
} DEBUG_HEAP_INFORMATION, *PDEBUG_HEAP_INFORMATION;

typedef struct _DEBUG_LOCK_INFORMATION
{
    PVOID  Address;
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    ULONG  OwnerThreadId;
    ULONG  ActiveCount;
    ULONG  ContentionCount;
    ULONG  EntryCount;
    ULONG  RecursionCount;
    ULONG  NumberOfSharedWaiters;
    ULONG  NumberOfExclusiveWaiters;
} DEBUG_LOCK_INFORMATION, *PDEBUG_LOCK_INFORMATION;

typedef struct _PORT_MESSAGE_HEADER
{
    USHORT    DataSize;
    USHORT    MessageSize;
    USHORT    MessageType;
    USHORT    VirtualRangesOffset;
    CLIENT_ID ClientId;
    ULONG     MessageId;
    ULONG     SectionSize;
} PORT_MESSAGE_HEADER, *PPORT_MESSAGE_HEADER, PORT_MESSAGE, *PPORT_MESSAGE;

typedef unsigned short RTL_ATOM, *PRTL_ATOM;

typedef enum _ATOM_INFORMATION_CLASS
{
    AtomBasicInformation = 0,
    AtomTableInformation = 1,
} ATOM_INFORMATION_CLASS;

typedef struct _ATOM_BASIC_INFORMATION
{
    USHORT ReferenceCount;
    USHORT Pinned;
    USHORT NameLength;
    WCHAR  Name[1];
} ATOM_BASIC_INFORMATION, *PATOM_BASIC_INFORMATION;

/***********************************************************************
 * Defines
 */

/* flags for NtCreateFile and NtOpenFile */
#define FILE_DIRECTORY_FILE            0x00000001
#define FILE_WRITE_THROUGH             0x00000002
#define FILE_SEQUENTIAL_ONLY           0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING 0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT      0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT   0x00000020
#define FILE_NON_DIRECTORY_FILE        0x00000040
#define FILE_CREATE_TREE_CONNECTION    0x00000080
#define FILE_COMPLETE_IF_OPLOCKED      0x00000100
#define FILE_NO_EA_KNOWLEDGE           0x00000200
#define FILE_OPEN_FOR_RECOVERY         0x00000400
#define FILE_RANDOM_ACCESS             0x00000800
#define FILE_DELETE_ON_CLOSE           0x00001000
#define FILE_OPEN_BY_FILE_ID           0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT    0x00004000
#define FILE_NO_COMPRESSION            0x00008000
#define FILE_RESERVE_OPFILTER          0x00100000
#define FILE_OPEN_REPARSE_POINT        0x00200000
#define FILE_OPEN_OFFLINE_FILE         0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY 0x00800000

#define FILE_ATTRIBUTE_VALID_FLAGS     0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS 0x000031a7

/* status for NtCreateFile or NtOpenFile */
#define FILE_SUPERSEDED     0
#define FILE_OPENED         1
#define FILE_CREATED        2
#define FILE_OVERWRITTEN    3
#define FILE_EXISTS         4
#define FILE_DOES_NOT_EXIST 5

/* disposition for NtCreateFile */
#define FILE_SUPERSEDE           0
#define FILE_OPEN                1
#define FILE_CREATE              2
#define FILE_OPEN_IF             3
#define FILE_OVERWRITE           4
#define FILE_OVERWRITE_IF        5
#define FILE_MAXIMUM_DISPOSITION 5

/* Characteristics of a File System */
#define FILE_REMOVABLE_MEDIA                     0x00000001
#define FILE_READ_ONLY_DEVICE                    0x00000002
#define FILE_FLOPPY_DISKETTE                     0x00000004
#define FILE_WRITE_ONE_MEDIA                     0x00000008
#define FILE_REMOTE_DEVICE                       0x00000010
#define FILE_DEVICE_IS_MOUNTED                   0x00000020
#define FILE_VIRTUAL_VOLUME                      0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME           0x00000080
#define FILE_DEVICE_SECURE_OPEN                  0x00000100
#define FILE_CHARACTERISTIC_PNP_DEVICE           0x00000800
#define FILE_CHARACTERISTIC_TS_DEVICE            0x00001000
#define FILE_CHARACTERISTIC_WEBDAV_DEVICE        0x00002000
#define FILE_CHARACTERISTIC_CSV                  0x00010000
#define FILE_DEVICE_ALLOW_APPCONTAINER_TRAVERSAL 0x00020000
#define FILE_PORTABLE_DEVICE                     0x00040000

/* options for NtCreateNamedPipeFile */
#define FILE_PIPE_INBOUND     0x00000000
#define FILE_PIPE_OUTBOUND    0x00000001
#define FILE_PIPE_FULL_DUPLEX 0x00000002

/* options for pipe's type */
#define FILE_PIPE_TYPE_MESSAGE 0x00000001
#define FILE_PIPE_TYPE_BYTE    0x00000000
/* options for pipe's message mode */
#define FILE_PIPE_MESSAGE_MODE     0x00000001
#define FILE_PIPE_BYTE_STREAM_MODE 0x00000000
/* options for pipe's blocking mode */
#define FILE_PIPE_COMPLETE_OPERATION 0x00000001
#define FILE_PIPE_QUEUE_OPERATION    0x00000000
/* and client / server end */
#define FILE_PIPE_SERVER_END 0x00000001
#define FILE_PIPE_CLIENT_END 0x00000000

#define INTERNAL_TS_ACTIVE_CONSOLE_ID (*((volatile ULONG *)(0x7ffe02d8)))

#define LOGONID_CURRENT ((ULONG) - 1)

#define OBJ_PROTECT_CLOSE    0x00000001
#define OBJ_INHERIT          0x00000002
#define OBJ_PERMANENT        0x00000010
#define OBJ_EXCLUSIVE        0x00000020
#define OBJ_CASE_INSENSITIVE 0x00000040
#define OBJ_OPENIF           0x00000080
#define OBJ_OPENLINK         0x00000100
#define OBJ_KERNEL_HANDLE    0x00000200
#define OBJ_VALID_ATTRIBUTES 0x000003F2

#define SERVERNAME_CURRENT ((HANDLE)NULL)

typedef void(CALLBACK *PNTAPCFUNC)(ULONG_PTR, ULONG_PTR, ULONG_PTR); /* FIXME: not the right name */

/* DbgPrintEx default levels */
#define DPFLTR_ERROR_LEVEL   0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL   2
#define DPFLTR_INFO_LEVEL    3
#define DPFLTR_MASK          0x8000000

/* Well-known LUID values */
#define SE_MIN_WELL_KNOWN_PRIVILEGE      2
#define SE_CREATE_TOKEN_PRIVILEGE        2
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE  3
#define SE_LOCK_MEMORY_PRIVILEGE         4
#define SE_INCREASE_QUOTA_PRIVILEGE      5
#define SE_UNSOLICITED_INPUT_PRIVILEGE   6 /* obsolete */
#define SE_MACHINE_ACCOUNT_PRIVILEGE     6
#define SE_TCB_PRIVILEGE                 7
#define SE_SECURITY_PRIVILEGE            8
#define SE_TAKE_OWNERSHIP_PRIVILEGE      9
#define SE_LOAD_DRIVER_PRIVILEGE         10
#define SE_SYSTEM_PROFILE_PRIVILEGE      11
#define SE_SYSTEMTIME_PRIVILEGE          12
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE 13
#define SE_INC_BASE_PRIORITY_PRIVILEGE   14
#define SE_CREATE_PAGEFILE_PRIVILEGE     15
#define SE_CREATE_PERMANENT_PRIVILEGE    16
#define SE_BACKUP_PRIVILEGE              17
#define SE_RESTORE_PRIVILEGE             18
#define SE_SHUTDOWN_PRIVILEGE            19
#define SE_DEBUG_PRIVILEGE               20
#define SE_AUDIT_PRIVILEGE               21
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE  22
#define SE_CHANGE_NOTIFY_PRIVILEGE       23
#define SE_REMOTE_SHUTDOWN_PRIVILEGE     24
#define SE_UNDOCK_PRIVILEGE              25
#define SE_SYNC_AGENT_PRIVILEGE          26
#define SE_ENABLE_DELEGATION_PRIVILEGE   27
#define SE_MANAGE_VOLUME_PRIVILEGE       28
#define SE_IMPERSONATE_PRIVILEGE         29
#define SE_CREATE_GLOBAL_PRIVILEGE       30
#define SE_MAX_WELL_KNOWN_PRIVILEGE      SE_CREATE_GLOBAL_PRIVILEGE

/* NtGlobalFlag bits */
#define FLG_STOP_ON_EXCEPTION          0x00000001
#define FLG_SHOW_LDR_SNAPS             0x00000002
#define FLG_DEBUG_INITIAL_COMMAND      0x00000004
#define FLG_STOP_ON_HUNG_GUI           0x00000008
#define FLG_HEAP_ENABLE_TAIL_CHECK     0x00000010
#define FLG_HEAP_ENABLE_FREE_CHECK     0x00000020
#define FLG_HEAP_VALIDATE_PARAMETERS   0x00000040
#define FLG_HEAP_VALIDATE_ALL          0x00000080
#define FLG_APPLICATION_VERIFIER       0x00000100
#define FLG_POOL_ENABLE_TAGGING        0x00000400
#define FLG_HEAP_ENABLE_TAGGING        0x00000800
#define FLG_USER_STACK_TRACE_DB        0x00001000
#define FLG_KERNEL_STACK_TRACE_DB      0x00002000
#define FLG_MAINTAIN_OBJECT_TYPELIST   0x00004000
#define FLG_HEAP_ENABLE_TAG_BY_DLL     0x00008000
#define FLG_DISABLE_STACK_EXTENSION    0x00010000
#define FLG_ENABLE_CSRDEBUG            0x00020000
#define FLG_ENABLE_KDEBUG_SYMBOL_LOAD  0x00040000
#define FLG_DISABLE_PAGE_KERNEL_STACKS 0x00080000
#define FLG_ENABLE_SYSTEM_CRIT_BREAKS  0x00100000
#define FLG_HEAP_DISABLE_COALESCING    0x00200000
#define FLG_ENABLE_CLOSE_EXCEPTIONS    0x00400000
#define FLG_ENABLE_EXCEPTION_LOGGING   0x00800000
#define FLG_ENABLE_HANDLE_TYPE_TAGGING 0x01000000
#define FLG_HEAP_PAGE_ALLOCS           0x02000000
#define FLG_DEBUG_INITIAL_COMMAND_EX   0x04000000
#define FLG_DISABLE_DBGPRINT           0x08000000
#define FLG_CRITSEC_EVENT_CREATION     0x10000000
#define FLG_LDR_TOP_DOWN               0x20000000
#define FLG_ENABLE_HANDLE_EXCEPTIONS   0x40000000
#define FLG_DISABLE_PROTDLLS           0x80000000

typedef struct _KEY_MULTIPLE_VALUE_INFORMATION
{
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_MULTIPLE_VALUE_INFORMATION, *PKEY_MULTIPLE_VALUE_INFORMATION;

typedef VOID(CALLBACK *PTIMER_APC_ROUTINE)(PVOID, ULONG, LONG);

typedef enum _EVENT_INFORMATION_CLASS
{
    EventBasicInformation
} EVENT_INFORMATION_CLASS,
    *PEVENT_INFORMATION_CLASS;

typedef struct _EVENT_BASIC_INFORMATION
{
    EVENT_TYPE EventType;
    LONG       EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

typedef enum _SEMAPHORE_INFORMATION_CLASS
{
    SemaphoreBasicInformation
} SEMAPHORE_INFORMATION_CLASS,
    *PSEMAPHORE_INFORMATION_CLASS;

typedef struct _SEMAPHORE_BASIC_INFORMATION
{
    ULONG CurrentCount;
    ULONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS
{
    SectionBasicInformation,
    SectionImageInformation,
    SectionRelocationInformation,
    SectionOriginalBaseInformation,
    SectionInternalImageInformation
} SECTION_INFORMATION_CLASS;

typedef struct _SECTION_BASIC_INFORMATION
{
    PVOID         BaseAddress;
    ULONG         Attributes;
    LARGE_INTEGER Size;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION
{
    PVOID   TransferAddress;
    ULONG   ZeroBits;
    SIZE_T  MaximumStackSize;
    SIZE_T  CommittedStackSize;
    ULONG   SubSystemType;
    USHORT  MinorSubsystemVersion;
    USHORT  MajorSubsystemVersion;
    USHORT  MajorOperatingSystemVersion;
    USHORT  MinorOperatingSystemVersion;
    USHORT  ImageCharacteristics;
    USHORT  DllCharacteristics;
    USHORT  Machine;
    BOOLEAN ImageContainsCode;

    union
    {
        UCHAR ImageFlags;

        struct
        {
            UCHAR ComPlusNativeReady        : 1;
            UCHAR ComPlusILOnly             : 1;
            UCHAR ImageDynamicallyRelocated : 1;
            UCHAR ImageMappedFlat           : 1;
            UCHAR BaseBelow4gb              : 1;
            UCHAR ComPlusPrefer32bit        : 1;
            UCHAR Reserved                  : 2;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

typedef struct _LPC_SECTION_WRITE
{
    ULONG  Length;
    HANDLE SectionHandle;
    ULONG  SectionOffset;
    ULONG  ViewSize;
    PVOID  ViewBase;
    PVOID  TargetViewBase;
} LPC_SECTION_WRITE, *PLPC_SECTION_WRITE;

typedef struct _LPC_SECTION_READ
{
    ULONG Length;
    ULONG ViewSize;
    PVOID ViewBase;
} LPC_SECTION_READ, *PLPC_SECTION_READ;

typedef struct _LPC_MESSAGE
{
    USHORT    DataSize;
    USHORT    MessageSize;
    USHORT    MessageType;
    USHORT    VirtualRangesOffset;
    CLIENT_ID ClientId;
    ULONG_PTR MessageId;
    ULONG_PTR SectionSize;
    UCHAR     Data[ANYSIZE_ARRAY];
} LPC_MESSAGE, *PLPC_MESSAGE;

typedef enum _SHUTDOWN_ACTION
{
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION,
    *PSHUTDOWN_ACTION;

typedef enum _KPROFILE_SOURCE
{
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE,
    *PKPROFILE_SOURCE;

typedef struct _DIRECTORY_BASIC_INFORMATION
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING ObjectTypeName;
} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;

typedef struct _INITIAL_TEB
{
    void *OldStackBase;
    void *OldStackLimit;
    void *StackBase;
    void *StackLimit;
    void *DeallocationStack;
} INITIAL_TEB, *PINITIAL_TEB;

typedef enum _PORT_INFORMATION_CLASS
{
    PortNoInformation
} PORT_INFORMATION_CLASS,
    *PPORT_INFORMATION_CLASS;

typedef enum _IO_COMPLETION_INFORMATION_CLASS
{
    IoCompletionBasicInformation
} IO_COMPLETION_INFORMATION_CLASS,
    *PIO_COMPLETION_INFORMATION_CLASS;

typedef struct _FILE_COMPLETION_INFORMATION
{
    HANDLE    CompletionPort;
    ULONG_PTR CompletionKey;
} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

#define IO_COMPLETION_QUERY_STATE  0x0001
#define IO_COMPLETION_MODIFY_STATE 0x0002
#define IO_COMPLETION_ALL_ACCESS   (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3)

typedef struct _FILE_IO_COMPLETION_INFORMATION
{
    ULONG_PTR       CompletionKey;
    ULONG_PTR       CompletionValue;
    IO_STATUS_BLOCK IoStatusBlock;
} FILE_IO_COMPLETION_INFORMATION, *PFILE_IO_COMPLETION_INFORMATION;

typedef enum _HARDERROR_RESPONSE_OPTION
{
    OptionAbortRetryIgnore,
    OptionOk,
    OptionOkCancel,
    OptionRetryCancel,
    OptionYesNo,
    OptionYesNoCancel,
    OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION,
    *PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE
{
    ResponseReturnToCaller,
    ResponseNotHandled,
    ResponseAbort,
    ResponseCancel,
    ResponseIgnore,
    ResponseNo,
    ResponseOk,
    ResponseRetry,
    ResponseYes
} HARDERROR_RESPONSE,
    *PHARDERROR_RESPONSE;

typedef enum _SYSDBG_COMMAND
{
    SysDbgQueryModuleInformation,
    SysDbgQueryTraceInformation,
    SysDbgSetTracepoint,
    SysDbgSetSpecialCall,
    SysDbgClearSpecialCalls,
    SysDbgQuerySpecialCalls,
    SysDbgBreakPoint,
    SysDbgQueryVersion,
    SysDbgReadVirtual,
    SysDbgWriteVirtual,
    SysDbgReadPhysical,
    SysDbgWritePhysical,
    SysDbgReadControlSpace,
    SysDbgWriteControlSpace,
    SysDbgReadIoSpace,
    SysDbgWriteIoSpace,
    SysDbgReadMsr,
    SysDbgWriteMsr,
    SysDbgReadBusData,
    SysDbgWriteBusData,
    SysDbgCheckLowMemory,
    SysDbgEnableKernelDebugger,
    SysDbgDisableKernelDebugger,
    SysDbgGetAutoKdEnable,
    SysDbgSetAutoKdEnable,
    SysDbgGetPrintBufferSize,
    SysDbgSetPrintBufferSize,
    SysDbgGetKdUmExceptionEnable,
    SysDbgSetKdUmExceptionEnable,
    SysDbgGetTriageDump,
    SysDbgGetKdBlockEnable,
    SysDbgSetKdBlockEnable,
    SysDbgRegisterForUmBreakInfo,
    SysDbgGetUmBreakPid,
    SysDbgClearUmBreakPid,
    SysDbgGetUmAttachPid,
    SysDbgClearUmAttachPid,
    SysDbgGetLiveKernelDump,
    SysDbgKdPullRemoteFile,
    SysDbgMaxInfoClass
} SYSDBG_COMMAND,
    *PSYSDBG_COMMAND;

typedef struct _CPTABLEINFO
{
    USHORT  CodePage;
    USHORT  MaximumCharacterSize;
    USHORT  DefaultChar;
    USHORT  UniDefaultChar;
    USHORT  TransDefaultChar;
    USHORT  TransUniDefaultChar;
    USHORT  DBCSCodePage;
    UCHAR   LeadByte[12];
    USHORT *MultiByteTable;
    void   *WideCharTable;
    USHORT *DBCSRanges;
    USHORT *DBCSOffsets;
} CPTABLEINFO, *PCPTABLEINFO;

typedef struct _NLSTABLEINFO
{
    CPTABLEINFO OemTableInfo;
    CPTABLEINFO AnsiTableInfo;
    USHORT     *UpperCaseTable;
    USHORT     *LowerCaseTable;
} NLSTABLEINFO, *PNLSTABLEINFO;

/*************************************************************************
 * Loader structures
 *
 * Those are not part of standard Winternl.h
 */

/* FIXME: to be checked */
#define MAXIMUM_FILENAME_LENGTH 256

#define PS_ATTRIBUTE_THREAD   0x00010000
#define PS_ATTRIBUTE_INPUT    0x00020000
#define PS_ATTRIBUTE_ADDITIVE 0x00040000

typedef enum _PS_ATTRIBUTE_NUM
{
    PsAttributeParentProcess,
    PsAttributeDebugPort,
    PsAttributeToken,
    PsAttributeClientId,
    PsAttributeTebAddress,
    PsAttributeImageName,
    PsAttributeImageInfo,
    PsAttributeMemoryReserve,
    PsAttributePriorityClass,
    PsAttributeErrorMode,
    PsAttributeStdHandleInfo,
    PsAttributeHandleList,
    PsAttributeGroupAffinity,
    PsAttributePreferredNode,
    PsAttributeIdealProcessor,
    PsAttributeUmsThread,
    PsAttributeMitigationOptions,
    PsAttributeProtectionLevel,
    PsAttributeSecureProcess,
    PsAttributeJobList,
    PsAttributeChildProcessPolicy,
    PsAttributeAllApplicationPackagesPolicy,
    PsAttributeWin32kFilter,
    PsAttributeSafeOpenPromptOriginClaim,
    PsAttributeBnoIsolation,
    PsAttributeDesktopAppPolicy,
    PsAttributeChpe,
    PsAttributeMitigationAuditOptions,
    PsAttributeMachineType,
    PsAttributeComponentFilter,
    PsAttributeEnableOptionalXStateFeatures,
    PsAttributeSupportedMachines,
    PsAttributeSveVectorLength,
    PsAttributeMax
} PS_ATTRIBUTE_NUM;

#define PS_ATTRIBUTE_PARENT_PROCESS                  (PsAttributeParentProcess | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_DEBUG_PORT                      (PsAttributeDebugPort | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_TOKEN                           (PsAttributeToken | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_CLIENT_ID                       (PsAttributeClientId | PS_ATTRIBUTE_THREAD)
#define PS_ATTRIBUTE_TEB_ADDRESS                     (PsAttributeTebAddress | PS_ATTRIBUTE_THREAD)
#define PS_ATTRIBUTE_IMAGE_NAME                      (PsAttributeImageName | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_IMAGE_INFO                      (PsAttributeImageInfo)
#define PS_ATTRIBUTE_MEMORY_RESERVE                  (PsAttributeMemoryReserve | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_PRIORITY_CLASS                  (PsAttributePriorityClass | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_ERROR_MODE                      (PsAttributeErrorMode | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_STD_HANDLE_INFO                 (PsAttributeStdHandleInfo | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_HANDLE_LIST                     (PsAttributeHandleList | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_GROUP_AFFINITY                  (PsAttributeGroupAffinity | PS_ATTRIBUTE_THREAD | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_PREFERRED_NODE                  (PsAttributePreferredNode | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_IDEAL_PROCESSOR                 (PsAttributeIdealProcessor | PS_ATTRIBUTE_THREAD | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_MITIGATION_OPTIONS              (PsAttributeMitigationOptions | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_PROTECTION_LEVEL                (PsAttributeProtectionLevel | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_SECURE_PROCESS                  (PsAttributeSecureProcess | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_JOB_LIST                        (PsAttributeJobList | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_CHILD_PROCESS_POLICY            (PsAttributeChildProcessPolicy | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_ALL_APPLICATION_PACKAGES_POLICY (PsAttributeAllApplicationPackagesPolicy | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_WIN32K_FILTER                   (PsAttributeWin32kFilter | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_SAFE_OPEN_PROMPT_ORIGIN_CLAIM   (PsAttributeSafeOpenPromptOriginClaim | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_BNO_ISOLATION                   (PsAttributeBnoIsolation | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_DESKTOP_APP_POLICY              (PsAttributeDesktopAppPolicy | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_CHPE                            (PsAttributeChpe | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_MITIGATION_AUDIT_OPTIONS        (PsAttributeMitigationAuditOptions | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_MACHINE_TYPE                    (PsAttributeMachineType | PS_ATTRIBUTE_INPUT | PS_ATTRIBUTE_ADDITIVE)
#define PS_ATTRIBUTE_COMPONENT_FILTER                (PsAttributeComponentFilter | PS_ATTRIBUTE_INPUT)
#define PS_ATTRIBUTE_ENABLE_OPTIONAL_XSTATE_FEATURES (PsAttributeEnableOptionalXStateFeatures | PS_ATTRIBUTE_THREAD | PS_ATTRIBUTE_INPUT)

typedef struct _PS_ATTRIBUTE
{
    ULONG_PTR Attribute;
    SIZE_T    Size;

    union
    {
        ULONG_PTR Value;
        void     *ValuePtr;
    };

    SIZE_T *ReturnLength;
} PS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST
{
    SIZE_T       TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

typedef enum _PS_CREATE_STATE
{
    PsCreateInitialState,
    PsCreateFailOnFileOpen,
    PsCreateFailOnSectionCreate,
    PsCreateFailExeFormat,
    PsCreateFailMachineMismatch,
    PsCreateFailExeName,
    PsCreateSuccess,
    PsCreateMaximumStates
} PS_CREATE_STATE;

typedef struct _PS_CREATE_INFO
{
    SIZE_T          Size;
    PS_CREATE_STATE State;

    union
    {
        struct
        {
            union
            {
                ULONG InitFlags;

                struct
                {
                    UCHAR  WriteOutputOnExit              : 1;
                    UCHAR  DetectManifest                 : 1;
                    UCHAR  IFEOSkipDebugger               : 1;
                    UCHAR  IFEODoNotPropagateKeyState     : 1;
                    UCHAR  SpareBits1                     : 4;
                    UCHAR  SpareBits2                     : 8;
                    USHORT ProhibitedImageCharacteristics : 16;
                };
            };

            ACCESS_MASK AdditionalFileAccess;
        } InitState;

        struct
        {
            HANDLE FileHandle;
        } FailSection;

        struct
        {
            USHORT DllCharacteristics;
        } ExeFormat;

        struct
        {
            HANDLE IFEOKey;
        } ExeName;

        struct
        {
            union
            {
                ULONG OutputFlags;

                struct
                {
                    UCHAR  ProtectedProcess      : 1;
                    UCHAR  AddressSpaceOverride  : 1;
                    UCHAR  DevOverrideEnabled    : 1;
                    UCHAR  ManifestDetected      : 1;
                    UCHAR  ProtectedProcessLight : 1;
                    UCHAR  SpareBits1            : 3;
                    UCHAR  SpareBits2            : 8;
                    USHORT SpareBits3            : 16;
                };
            };

            HANDLE    FileHandle;
            HANDLE    SectionHandle;
            ULONGLONG UserProcessParametersNative;
            ULONG     UserProcessParametersWow64;
            ULONG     CurrentParameterFlags;
            ULONGLONG PebAddressNative;
            ULONG     PebAddressWow64;
            ULONGLONG ManifestAddress;
            ULONG     ManifestSize;
        } SuccessState;
    };
} PS_CREATE_INFO, *PPS_CREATE_INFO;

typedef struct _DBGKM_EXCEPTION
{
    EXCEPTION_RECORD ExceptionRecord;
    ULONG            FirstChance;
} DBGKM_EXCEPTION, *PDBGKM_EXCEPTION;

typedef struct _DBGKM_CREATE_THREAD
{
    ULONG SubSystemKey;
    PVOID StartAddress;
} DBGKM_CREATE_THREAD, *PDBGKM_CREATE_THREAD;

typedef struct _DBGKM_CREATE_PROCESS
{
    ULONG               SubSystemKey;
    HANDLE              FileHandle;
    PVOID               BaseOfImage;
    ULONG               DebugInfoFileOffset;
    ULONG               DebugInfoSize;
    DBGKM_CREATE_THREAD InitialThread;
} DBGKM_CREATE_PROCESS, *PDBGKM_CREATE_PROCESS;

typedef struct _DBGKM_EXIT_THREAD
{
    NTSTATUS ExitStatus;
} DBGKM_EXIT_THREAD, *PDBGKM_EXIT_THREAD;

typedef struct _DBGKM_EXIT_PROCESS
{
    NTSTATUS ExitStatus;
} DBGKM_EXIT_PROCESS, *PDBGKM_EXIT_PROCESS;

typedef struct _DBGKM_LOAD_DLL
{
    HANDLE FileHandle;
    PVOID  BaseOfDll;
    ULONG  DebugInfoFileOffset;
    ULONG  DebugInfoSize;
    PVOID  NamePointer;
} DBGKM_LOAD_DLL, *PDBGKM_LOAD_DLL;

typedef struct _DBGKM_UNLOAD_DLL
{
    PVOID BaseAddress;
} DBGKM_UNLOAD_DLL, *PDBGKM_UNLOAD_DLL;

typedef enum _DBG_STATE
{
    DbgIdle,
    DbgReplyPending,
    DbgCreateThreadStateChange,
    DbgCreateProcessStateChange,
    DbgExitThreadStateChange,
    DbgExitProcessStateChange,
    DbgExceptionStateChange,
    DbgBreakpointStateChange,
    DbgSingleStepStateChange,
    DbgLoadDllStateChange,
    DbgUnloadDllStateChange
} DBG_STATE,
    *PDBG_STATE;

typedef struct _DBGUI_CREATE_THREAD
{
    HANDLE              HandleToThread;
    DBGKM_CREATE_THREAD NewThread;
} DBGUI_CREATE_THREAD, *PDBGUI_CREATE_THREAD;

typedef struct _DBGUI_CREATE_PROCESS
{
    HANDLE               HandleToProcess;
    HANDLE               HandleToThread;
    DBGKM_CREATE_PROCESS NewProcess;
} DBGUI_CREATE_PROCESS, *PDBGUI_CREATE_PROCESS;

typedef struct _DBGUI_WAIT_STATE_CHANGE
{
    DBG_STATE NewState;
    CLIENT_ID AppClientId;

    union
    {
        DBGKM_EXCEPTION      Exception;
        DBGUI_CREATE_THREAD  CreateThread;
        DBGUI_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD    ExitThread;
        DBGKM_EXIT_PROCESS   ExitProcess;
        DBGKM_LOAD_DLL       LoadDll;
        DBGKM_UNLOAD_DLL     UnloadDll;
    } StateInfo;
} DBGUI_WAIT_STATE_CHANGE, *PDBGUI_WAIT_STATE_CHANGE;

struct _DEBUG_EVENT;

#define DEBUG_READ_EVENT        0x0001
#define DEBUG_PROCESS_ASSIGN    0x0002
#define DEBUG_SET_INFORMATION   0x0004
#define DEBUG_QUERY_INFORMATION 0x0008
#define DEBUG_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x0f)

#define DEBUG_KILL_ON_CLOSE 0x1

typedef enum _DEBUGOBJECTINFOCLASS
{
    DebugObjectKillProcessOnExitInformation = 1,
    MaxDebugObjectInfoClass
} DEBUGOBJECTINFOCLASS,
    *PDEBUGOBJECTINFOCLASS;

typedef struct _WOW64_CPURESERVED
{
    USHORT Flags;
    USHORT Machine;
    /* CONTEXT context */
    /* CONTEXT_EX *context_ex */
} WOW64_CPURESERVED, *PWOW64_CPURESERVED;

#define WOW64_CPURESERVED_FLAG_RESET_STATE 1

typedef struct _WOW64_CPU_AREA_INFO
{
    void              *Context;
    void              *ContextEx;
    void              *ContextFlagsLocation;
    WOW64_CPURESERVED *CpuReserved;
    ULONG              ContextFlag;
    USHORT             Machine;
} WOW64_CPU_AREA_INFO, *PWOW64_CPU_AREA_INFO;

typedef struct _WOW64INFO
{
    ULONG     NativeSystemPageSize;
    ULONG     CpuFlags;
    ULONG     Wow64ExecuteFlags;
    ULONG     unknown;
    ULONGLONG SectionHandle;
    ULONGLONG CrossProcessWorkList;
    USHORT    NativeMachineType;
    USHORT    EmulatedMachineType;
} WOW64INFO;

C_ASSERT(sizeof(WOW64INFO) == 40);

#define WOW64_CPUFLAGS_MSFT64   0x01
#define WOW64_CPUFLAGS_SOFTWARE 0x02

/* wow64.dll functions */
void *WINAPI Wow64AllocateTemp(SIZE_T);
void WINAPI Wow64ApcRoutine(ULONG_PTR, ULONG_PTR, ULONG_PTR, CONTEXT *);
NTSTATUS WINAPI Wow64KiUserCallbackDispatcher(ULONG, void *, ULONG, void **, ULONG *);
void WINAPI Wow64PassExceptionToGuest(EXCEPTION_POINTERS *);
void WINAPI Wow64PrepareForException(EXCEPTION_RECORD *, CONTEXT *);
void WINAPI Wow64ProcessPendingCrossProcessItems(void);
NTSTATUS WINAPI Wow64RaiseException(int, EXCEPTION_RECORD *);
NTSTATUS WINAPI Wow64SystemServiceEx(UINT, UINT *);

#ifdef __WINESRC__
/* undocumented layout of LdrSystemDllInitBlock */
/* this varies across Windows version; we are using the win10-2004 layout */
typedef struct
{
    ULONG   version;
    ULONG   unknown1[3];
    ULONG64 unknown2;
    ULONG64 pLdrInitializeThunk;
    ULONG64 pKiUserExceptionDispatcher;
    ULONG64 pKiUserApcDispatcher;
    ULONG64 pKiUserCallbackDispatcher;
    ULONG64 pRtlUserThreadStart;
    ULONG64 pRtlpQueryProcessDebugInformationRemote;
    ULONG64 ntdll_handle;
    ULONG64 pLdrSystemDllInitBlock;
    ULONG64 pRtlpFreezeTimeBias;
} SYSTEM_DLL_INIT_BLOCK;
#endif

typedef struct
{
    ULONG_PTR *ServiceTable;
    ULONG_PTR *CounterTable;
    ULONG_PTR  ServiceLimit;
    BYTE      *ArgumentTable;
} SYSTEM_SERVICE_TABLE;

/* ApiSet structures (format for version 6) */

typedef struct _API_SET_NAMESPACE
{
    ULONG Version;
    ULONG Size;
    ULONG Flags;
    ULONG Count;
    ULONG EntryOffset;
    ULONG HashOffset;
    ULONG HashFactor;
} API_SET_NAMESPACE;

typedef struct _API_SET_HASH_ENTRY
{
    ULONG Hash;
    ULONG Index;
} API_SET_HASH_ENTRY;

typedef struct _API_SET_NAMESPACE_ENTRY
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG HashedLength;
    ULONG ValueOffset;
    ULONG ValueCount;
} API_SET_NAMESPACE_ENTRY;

typedef struct _API_SET_VALUE_ENTRY
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG ValueOffset;
    ULONG ValueLength;
} API_SET_VALUE_ENTRY;

typedef enum _KCONTINUE_TYPE
{
    KCONTINUE_UNWIND,
    KCONTINUE_RESUME,
    KCONTINUE_LONGJUMP,
    KCONTINUE_SET,
    KCONTINUE_LAST,
} KCONTINUE_TYPE;

typedef struct _KCONTINUE_ARGUMENT
{
    KCONTINUE_TYPE ContinueType;
    ULONG          ContinueFlags;
    ULONGLONG      Reserved[2];
} KCONTINUE_ARGUMENT, *PKCONTINUE_ARGUMENT;

#define KCONTINUE_FLAG_TEST_ALERT  0x01
#define KCONTINUE_FLAG_DELIVER_APC 0x02

#define HASH_STRING_ALGORITHM_DEFAULT 0
#define HASH_STRING_ALGORITHM_X65599  1
#define HASH_STRING_ALGORITHM_INVALID 0xffffffff

// NEXT
