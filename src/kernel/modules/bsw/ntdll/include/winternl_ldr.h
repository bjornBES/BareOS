/*
 * File: winternl_ldr.h
 * File Created: 27 Jun 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 27 Jun 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#include "winternl_types.h"

/**********************************************************************
 * Fundamental types and data structures
 */

typedef struct _PEB_LDR_DATA
{
    ULONG      Length;
    BOOLEAN    Initialized;
    PVOID      SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID      EntryInProgress;
    BOOLEAN    ShutdownInProgress;
    HANDLE     ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

/***********************************************************************
 * The 32-bit/64-bit version of the PEB and TEB for WoW64
 */

typedef struct _PEB_LDR_DATA32
{
    ULONG        Length;
    BOOLEAN      Initialized;
    ULONG        SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    ULONG        EntryInProgress;
    BOOLEAN      ShutdownInProgress;
    ULONG        ShutdownThreadId;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _PEB_LDR_DATA64
{
    ULONG        Length;
    BOOLEAN      Initialized;
    ULONG64      SsHandle;
    LIST_ENTRY64 InLoadOrderModuleList;
    LIST_ENTRY64 InMemoryOrderModuleList;
    LIST_ENTRY64 InInitializationOrderModuleList;
    ULONG64      EntryInProgress;
    BOOLEAN      ShutdownInProgress;
    ULONG64      ShutdownThreadId;
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;

/***********************************************************************
 * Types and data structures
 */

typedef struct _LDR_RESOURCE_INFO
{
    ULONG_PTR Type;
    ULONG_PTR Name;
    ULONG     Language;
} LDR_RESOURCE_INFO, *PLDR_RESOURCE_INFO;

/*************************************************************************
 * Loader structures
 *
 * Those are not part of standard Winternl.h
 */

typedef struct _LDR_SERVICE_TAG_RECORD
{
    struct _LDR_SERVICE_TAG_RECORD *Next;
    ULONG                           ServiceTag;
} LDR_SERVICE_TAG_RECORD, *PLDR_SERVICE_TAG_RECORD;

typedef struct _LDRP_CSLIST
{
    SINGLE_LIST_ENTRY *Tail;
} LDRP_CSLIST, *PLDRP_CSLIST;

typedef struct _LDR_DEPENDENCY
{
    SINGLE_LIST_ENTRY      dependency_to_entry;
    struct _LDR_DDAG_NODE *dependency_to;
    SINGLE_LIST_ENTRY      dependency_from_entry;
    struct _LDR_DDAG_NODE *dependency_from;
} LDR_DEPENDENCY, *PLDR_DEPENDENCY;

typedef enum _LDR_DDAG_STATE
{
    LdrModulesMerged                 = -5,
    LdrModulesInitError              = -4,
    LdrModulesSnapError              = -3,
    LdrModulesUnloaded               = -2,
    LdrModulesUnloading              = -1,
    LdrModulesPlaceHolder            = 0,
    LdrModulesMapping                = 1,
    LdrModulesMapped                 = 2,
    LdrModulesWaitingForDependencies = 3,
    LdrModulesSnapping               = 4,
    LdrModulesSnapped                = 5,
    LdrModulesCondensed              = 6,
    LdrModulesReadyToInit            = 7,
    LdrModulesInitializing           = 8,
    LdrModulesReadyToRun             = 9,
} LDR_DDAG_STATE;

typedef struct _LDR_DDAG_NODE
{
    LIST_ENTRY              Modules;
    LDR_SERVICE_TAG_RECORD *ServiceTagList;
    LONG                    LoadCount;
    ULONG                   LoadWhileUnloadingCount;
    ULONG                   LowestLink;
    LDRP_CSLIST             Dependencies;
    LDRP_CSLIST             IncomingDependencies;
    LDR_DDAG_STATE          State;
    SINGLE_LIST_ENTRY       CondenseLink;
    ULONG                   PreorderNumber;
} LDR_DDAG_NODE, *PLDR_DDAG_NODE;

typedef enum _LDR_DLL_LOAD_REASON
{
    LoadReasonStaticDependency,
    LoadReasonStaticForwarderDependency,
    LoadReasonDynamicForwarderDependency,
    LoadReasonDelayloadDependency,
    LoadReasonDynamicLoad,
    LoadReasonAsImageLoad,
    LoadReasonAsDataLoad,
    LoadReasonUnknown = -1
} LDR_DLL_LOAD_REASON,
    *PLDR_DLL_LOAD_REASON;

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY                  InLoadOrderLinks;
    LIST_ENTRY                  InMemoryOrderLinks;
    LIST_ENTRY                  InInitializationOrderLinks;
    void                       *DllBase;
    void                       *EntryPoint;
    ULONG                       SizeOfImage;
    UNICODE_STRING              FullDllName;
    UNICODE_STRING              BaseDllName;
    ULONG                       Flags;
    SHORT                       LoadCount;
    SHORT                       TlsIndex;
    LIST_ENTRY                  HashLinks;
    ULONG                       TimeDateStamp;
    struct _ACTIVATION_CONTEXT *ActivationContext;
    void                       *Lock;
    LDR_DDAG_NODE              *DdagNode;
    LIST_ENTRY                  NodeModuleLink;
    struct _LDRP_LOAD_CONTEXT  *LoadContext;
    void                       *ParentDllBase;
    void                       *SwitchBackContext;
    RTL_BALANCED_NODE           BaseAddressIndexNode;
    RTL_BALANCED_NODE           MappingInfoIndexNode;
    ULONG_PTR                   OriginalBase;
    LARGE_INTEGER               LoadTime;
    ULONG                       BaseNameHashValue;
    LDR_DLL_LOAD_REASON         LoadReason;
    ULONG                       ImplicitPathOptions;
    ULONG                       ReferenceCount;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
    ULONG                 Flags;
    const UNICODE_STRING *FullDllName;
    const UNICODE_STRING *BaseDllName;
    void                 *DllBase;
    ULONG                 SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
    ULONG                 Flags;
    const UNICODE_STRING *FullDllName;
    const UNICODE_STRING *BaseDllName;
    void                 *DllBase;
    ULONG                 SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
    LDR_DLL_LOADED_NOTIFICATION_DATA   Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef void(CALLBACK *PLDR_DLL_NOTIFICATION_FUNCTION)(ULONG, LDR_DLL_NOTIFICATION_DATA *, void *);

/* those defines are (some of the) regular LDR_DATA_TABLE_ENTRY.Flags values */
#define LDR_DONT_RESOLVE_REFS  0x00000002
#define LDR_IMAGE_IS_DLL       0x00000004
#define LDR_LOAD_IN_PROGRESS   0x00001000
#define LDR_UNLOAD_IN_PROGRESS 0x00002000
#define LDR_NO_DLL_CALLS       0x00040000
#define LDR_PROCESS_ATTACHED   0x00080000
#define LDR_COR_IMAGE          0x00400000
#define LDR_COR_ILONLY         0x01000000
#define LDR_REDIRECTED         0x10000000

/* this one is Wine specific */
#define LDR_WINE_INTERNAL 0x80000000

/* flag for LdrAddRefDll */
#define LDR_ADDREF_DLL_PIN 0x00000001

/* flags for LdrGetDllHandleEx */
#define LDR_GET_DLL_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 0x00000001
#define LDR_GET_DLL_HANDLE_EX_FLAG_PIN                0x00000002

#define LDR_DLL_NOTIFICATION_REASON_LOADED   1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

/***********************************************************************
 * Function declarations
 */

NTSYSAPI NTSTATUS WINAPI LdrAccessResource(HMODULE, const IMAGE_RESOURCE_DATA_ENTRY *, void **, PULONG);
NTSYSAPI NTSTATUS WINAPI LdrAddDllDirectory(const UNICODE_STRING *, void **);
NTSYSAPI NTSTATUS WINAPI LdrAddRefDll(ULONG, HMODULE);
NTSYSAPI NTSTATUS WINAPI LdrDisableThreadCalloutsForDll(HMODULE);
NTSYSAPI NTSTATUS WINAPI LdrFindEntryForAddress(const void *, PLDR_DATA_TABLE_ENTRY *);
NTSYSAPI NTSTATUS WINAPI LdrFindResourceDirectory_U(HMODULE, const LDR_RESOURCE_INFO *, ULONG, const IMAGE_RESOURCE_DIRECTORY **);
NTSYSAPI NTSTATUS WINAPI LdrFindResource_U(HMODULE, const LDR_RESOURCE_INFO *, ULONG, const IMAGE_RESOURCE_DATA_ENTRY **);
NTSYSAPI NTSTATUS WINAPI LdrGetDllDirectory(UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI LdrGetDllFullName(HMODULE, UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandle(LPCWSTR, ULONG, const UNICODE_STRING *, HMODULE *);
NTSYSAPI NTSTATUS WINAPI LdrGetDllHandleEx(ULONG, LPCWSTR, ULONG *, const UNICODE_STRING *, HMODULE *);
NTSYSAPI NTSTATUS WINAPI LdrGetDllPath(PCWSTR, ULONG, PWSTR *, PWSTR *);
NTSYSAPI NTSTATUS WINAPI LdrGetProcedureAddress(HMODULE, const ANSI_STRING *, ULONG, void **);
NTSYSAPI NTSTATUS WINAPI LdrLoadDll(LPCWSTR, DWORD *, const UNICODE_STRING *, HMODULE *);
NTSYSAPI NTSTATUS WINAPI LdrLockLoaderLock(ULONG, ULONG *, ULONG_PTR *);
IMAGE_BASE_RELOCATION *WINAPI LdrProcessRelocationBlock(void *, UINT, USHORT *, INT_PTR);
NTSYSAPI NTSTATUS WINAPI LdrQueryImageFileExecutionOptions(const UNICODE_STRING *, LPCWSTR, ULONG, void *, ULONG, ULONG *);
NTSYSAPI NTSTATUS WINAPI LdrRegisterDllNotification(ULONG, PLDR_DLL_NOTIFICATION_FUNCTION, void *, void **);
NTSYSAPI NTSTATUS WINAPI LdrRemoveDllDirectory(void *);
NTSYSAPI NTSTATUS WINAPI LdrSetDefaultDllDirectories(ULONG);
NTSYSAPI NTSTATUS WINAPI LdrSetDllDirectory(const UNICODE_STRING *);
NTSYSAPI void WINAPI LdrShutdownProcess(void);
NTSYSAPI void WINAPI LdrShutdownThread(void);
NTSYSAPI NTSTATUS WINAPI LdrUnloadDll(HMODULE);
NTSYSAPI NTSTATUS WINAPI LdrUnlockLoaderLock(ULONG, ULONG_PTR);
NTSYSAPI NTSTATUS WINAPI LdrUnregisterDllNotification(void *);
