/*
 * File: winternl_rtl.h
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

/*
 * RTL_SYSTEM_TIME and RTL_TIME_ZONE_INFORMATION are the same as
 * the SYSTEMTIME and TIME_ZONE_INFORMATION structures defined
 * in winbase.h, however we need to define them separately so
 * winternl.h doesn't depend on winbase.h.  They are used by
 * RtlQueryTimeZoneInformation and RtlSetTimeZoneInformation.
 * The names are guessed; if anybody knows the real names, let me know.
 */
typedef struct _RTL_SYSTEM_TIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} RTL_SYSTEM_TIME, *PRTL_SYSTEM_TIME;

typedef struct _RTL_TIME_ZONE_INFORMATION
{
    LONG            Bias;
    WCHAR           StandardName[32];
    RTL_SYSTEM_TIME StandardDate;
    LONG            StandardBias;
    WCHAR           DaylightName[32];
    RTL_SYSTEM_TIME DaylightDate;
    LONG            DaylightBias;
} RTL_TIME_ZONE_INFORMATION, *PRTL_TIME_ZONE_INFORMATION;

typedef struct _RTL_TIME_DYNAMIC_ZONE_INFORMATION
{
    LONG            Bias;
    WCHAR           StandardName[32];
    RTL_SYSTEM_TIME StandardDate;
    LONG            StandardBias;
    WCHAR           DaylightName[32];
    RTL_SYSTEM_TIME DaylightDate;
    LONG            DaylightBias;
    WCHAR           TimeZoneKeyName[128];
    BOOLEAN         DynamicDaylightTimeDisabled;
} RTL_DYNAMIC_TIME_ZONE_INFORMATION, *PRTL_DYNAMIC_TIME_ZONE_INFORMATION;

typedef struct RTL_DRIVE_LETTER_CURDIR
{
    USHORT         Flags;
    USHORT         Length;
    ULONG          TimeStamp;
    UNICODE_STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_RELATIVE_NAME
{
    UNICODE_STRING RelativeName;
    HANDLE         ContainerDirectory;
    void          *CurDirRef;
} RTL_RELATIVE_NAME, *PRTL_RELATIVE_NAME;

typedef struct tagRTL_BITMAP
{
    ULONG  SizeOfBitMap; /* Number of bits in the bitmap */
    PULONG Buffer;       /* Bitmap data, assumed sized to a DWORD boundary */
} RTL_BITMAP, *PRTL_BITMAP;

typedef const RTL_BITMAP *PCRTL_BITMAP;

typedef struct tagRTL_BITMAP_RUN
{
    ULONG StartingIndex; /* Bit position at which run starts */
    ULONG NumberOfBits;  /* Size of the run in bits */
} RTL_BITMAP_RUN, *PRTL_BITMAP_RUN;

typedef const RTL_BITMAP_RUN *PCRTL_BITMAP_RUN;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    ULONG                   AllocationSize;
    ULONG                   Size;
    ULONG                   Flags;
    ULONG                   DebugFlags;
    HANDLE                  ConsoleHandle;
    ULONG                   ConsoleFlags;
    HANDLE                  hStdInput;
    HANDLE                  hStdOutput;
    HANDLE                  hStdError;
    CURDIR                  CurrentDirectory;
    UNICODE_STRING          DllPath;
    UNICODE_STRING          ImagePathName;
    UNICODE_STRING          CommandLine;
    PWSTR                   Environment;
    ULONG                   dwX;
    ULONG                   dwY;
    ULONG                   dwXSize;
    ULONG                   dwYSize;
    ULONG                   dwXCountChars;
    ULONG                   dwYCountChars;
    ULONG                   dwFillAttribute;
    ULONG                   dwFlags;
    ULONG                   wShowWindow;
    UNICODE_STRING          WindowTitle;
    UNICODE_STRING          Desktop;
    UNICODE_STRING          ShellInfo;
    UNICODE_STRING          RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
    ULONG_PTR               EnvironmentSize;
    ULONG_PTR               EnvironmentVersion;
    PVOID                   PackageDependencyData;
    ULONG                   ProcessGroupId;
    ULONG                   LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

/* value for Flags field (FIXME: not the correct names) */
#define PROCESS_PARAMS_FLAG_NORMALIZED   0x00000001
#define PROCESS_PARAMS_IMAGE_KEY_MISSING 0x00004000

typedef struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME
{
    struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *Previous;
    struct _ACTIVATION_CONTEXT                 *ActivationContext;
    ULONG                                       Flags;
} RTL_ACTIVATION_CONTEXT_STACK_FRAME, *PRTL_ACTIVATION_CONTEXT_STACK_FRAME;

typedef struct _RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED
{
    SIZE_T                             Size;
    ULONG                              Format;
    RTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    PVOID                              Extra1;
    PVOID                              Extra2;
    PVOID                              Extra3;
    PVOID                              Extra4;
} RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED, *PRTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED;

typedef struct _ACTIVATION_CONTEXT_STACK
{
    RTL_ACTIVATION_CONTEXT_STACK_FRAME *ActiveFrame;
    LIST_ENTRY                          FrameListCache;
    ULONG                               Flags;
    ULONG                               NextCookieSequenceNumber;
    ULONG_PTR                           StackId;
} ACTIVATION_CONTEXT_STACK, *PACTIVATION_CONTEXT_STACK;

/***********************************************************************
 * PEB data structure
 */
typedef struct _PEB
{                                                                  /* win32/win64 */
    BOOLEAN                      InheritedAddressSpace;            /* 000/000 */
    BOOLEAN                      ReadImageFileExecOptions;         /* 001/001 */
    BOOLEAN                      BeingDebugged;                    /* 002/002 */
    UCHAR                        ImageUsedLargePages          : 1; /* 003/003 */
    UCHAR                        IsProtectedProcess           : 1;
    UCHAR                        IsImageDynamicallyRelocated  : 1;
    UCHAR                        SkipPatchingUser32Forwarders : 1;
    UCHAR                        IsPackagedProcess            : 1;
    UCHAR                        IsAppContainer               : 1;
    UCHAR                        IsProtectedProcessLight      : 1;
    UCHAR                        IsLongPathAwareProcess       : 1;
    HANDLE                       Mutant;                         /* 004/008 */
    HMODULE                      ImageBaseAddress;               /* 008/010 */
    PPEB_LDR_DATA                LdrData;                        /* 00c/018 */
    RTL_USER_PROCESS_PARAMETERS *ProcessParameters;              /* 010/020 */
    PVOID                        SubSystemData;                  /* 014/028 */
    HANDLE                       ProcessHeap;                    /* 018/030 */
    PRTL_CRITICAL_SECTION        FastPebLock;                    /* 01c/038 */
    PVOID                        AtlThunkSListPtr;               /* 020/040 */
    PVOID                        IFEOKey;                        /* 024/048 */
    ULONG                        ProcessInJob               : 1; /* 028/050 */
    ULONG                        ProcessInitializing        : 1;
    ULONG                        ProcessUsingVEH            : 1;
    ULONG                        ProcessUsingVCH            : 1;
    ULONG                        ProcessUsingFTH            : 1;
    ULONG                        ProcessPreviouslyThrottled : 1;
    ULONG                        ProcessCurrentlyThrottled  : 1;
    ULONG                        ProcessImagesHotPatched    : 1;
    ULONG                        ReservedBits0              : 24;
    KERNEL_CALLBACK_PROC        *KernelCallbackTable;            /* 02c/058 */
    ULONG                        Reserved;                       /* 030/060 */
    ULONG                        AtlThunkSListPtr32;             /* 034/064 */
    PVOID                        ApiSetMap;                      /* 038/068 */
    ULONG                        TlsExpansionCounter;            /* 03c/070 */
    PRTL_BITMAP                  TlsBitmap;                      /* 040/078 */
    ULONG                        TlsBitmapBits[2];               /* 044/080 */
    PVOID                        ReadOnlySharedMemoryBase;       /* 04c/088 */
    PVOID                        SharedData;                     /* 050/090 */
    PVOID                       *ReadOnlyStaticServerData;       /* 054/098 */
    PVOID                        AnsiCodePageData;               /* 058/0a0 */
    PVOID                        OemCodePageData;                /* 05c/0a8 */
    PVOID                        UnicodeCaseTableData;           /* 060/0b0 */
    ULONG                        NumberOfProcessors;             /* 064/0b8 */
    ULONG                        NtGlobalFlag;                   /* 068/0bc */
    LARGE_INTEGER                CriticalSectionTimeout;         /* 070/0c0 */
    SIZE_T                       HeapSegmentReserve;             /* 078/0c8 */
    SIZE_T                       HeapSegmentCommit;              /* 07c/0d0 */
    SIZE_T                       HeapDeCommitTotalFreeThreshold; /* 080/0d8 */
    SIZE_T                       HeapDeCommitFreeBlockThreshold; /* 084/0e0 */
    ULONG                        NumberOfHeaps;                  /* 088/0e8 */
    ULONG                        MaximumNumberOfHeaps;           /* 08c/0ec */
    PVOID                       *ProcessHeaps;                   /* 090/0f0 */
    PVOID                        GdiSharedHandleTable;           /* 094/0f8 */
    PVOID                        ProcessStarterHelper;           /* 098/100 */
    PVOID                        GdiDCAttributeList;             /* 09c/108 */
    PVOID                        LoaderLock;                     /* 0a0/110 */
    ULONG                        OSMajorVersion;                 /* 0a4/118 */
    ULONG                        OSMinorVersion;                 /* 0a8/11c */
    ULONG                        OSBuildNumber;                  /* 0ac/120 */
    ULONG                        OSPlatformId;                   /* 0b0/124 */
    ULONG                        ImageSubSystem;                 /* 0b4/128 */
    ULONG                        ImageSubSystemMajorVersion;     /* 0b8/12c */
    ULONG                        ImageSubSystemMinorVersion;     /* 0bc/130 */
    KAFFINITY                    ActiveProcessAffinityMask;      /* 0c0/138 */
#ifdef _WIN64
    ULONG GdiHandleBuffer[60];                                   /*    /140 */
#else
    ULONG GdiHandleBuffer[34]; /* 0c4/    */
#endif
    PVOID                PostProcessInitRoutine;      /* 14c/230 */
    PRTL_BITMAP          TlsExpansionBitmap;          /* 150/238 */
    ULONG                TlsExpansionBitmapBits[32];  /* 154/240 */
    ULONG                SessionId;                   /* 1d4/2c0 */
    ULARGE_INTEGER       AppCompatFlags;              /* 1d8/2c8 */
    ULARGE_INTEGER       AppCompatFlagsUser;          /* 1e0/2d0 */
    PVOID                ShimData;                    /* 1e8/2d8 */
    PVOID                AppCompatInfo;               /* 1ec/2e0 */
    UNICODE_STRING       CSDVersion;                  /* 1f0/2e8 */
    PVOID                ActivationContextData;       /* 1f8/2f8 */
    PVOID                ProcessAssemblyStorageMap;   /* 1fc/300 */
    PVOID                SystemDefaultActivationData; /* 200/308 */
    PVOID                SystemAssemblyStorageMap;    /* 204/310 */
    SIZE_T               MinimumStackCommit;          /* 208/318 */
    PVOID                SparePointers[2];            /* 20c/320 */
    PVOID                PatchLoaderData;             /* 214/330 */
    CHPEV2_PROCESS_INFO *ChpeV2ProcessInfo;           /* 218/338 */
    ULONG                AppModelFeatureState;        /* 21c/340 */
    ULONG                SpareUlongs[2];              /* 220/344 */
    USHORT               ActiveCodePage;              /* 228/34c */
    USHORT               OemCodePage;                 /* 22a/34e */
    USHORT               UseCaseMapping;              /* 22c/350 */
    USHORT               UnusedNlsField;              /* 22e/352 */
    PVOID                WerRegistrationData;         /* 230/358 */
    PVOID                WerShipAssertPtr;            /* 234/360 */
    PVOID                EcCodeBitMap;                /* 238/368 */
    PVOID                pImageHeaderHash;            /* 23c/370 */
    ULONG                HeapTracingEnabled      : 1; /* 240/378 */
    ULONG                CritSecTracingEnabled   : 1;
    ULONG                LibLoaderTracingEnabled : 1;
    ULONG                SpareTracingBits        : 29;
    ULONGLONG            CsrServerReadOnlySharedMemoryBase;       /* 248/380 */
    ULONG                TppWorkerpListLock;                      /* 250/388 */
    LIST_ENTRY           TppWorkerpList;                          /* 254/390 */
    PVOID                WaitOnAddressHashTable[0x80];            /* 25c/3a0 */
    PVOID                TelemetryCoverageHeader;                 /* 45c/7a0 */
    ULONG                CloudFileFlags;                          /* 460/7a8 */
    ULONG                CloudFileDiagFlags;                      /* 464/7ac */
    CHAR                 PlaceholderCompatibilityMode;            /* 468/7b0 */
    CHAR                 PlaceholderCompatibilityModeReserved[7]; /* 469/7b1 */
    PVOID                LeapSecondData;                          /* 470/7b8 */
    ULONG                LeapSecondFlags;                         /* 474/7c0 */
    ULONG                NtGlobalFlag2;                           /* 478/7c4 */
} PEB, *PPEB;

/***********************************************************************
 * TEB data structure
 */
typedef struct _TEB
{                                                     /* win32/win64 */
    NT_TIB    Tib;                                    /* 000/0000 */
    PVOID     EnvironmentPointer;                     /* 01c/0038 */
    CLIENT_ID ClientId;                               /* 020/0040 */
    PVOID     ActiveRpcHandle;                        /* 028/0050 */
    PVOID     ThreadLocalStoragePointer;              /* 02c/0058 */
    PPEB      Peb;                                    /* 030/0060 */
    ULONG     LastErrorValue;                         /* 034/0068 */
    ULONG     CountOfOwnedCriticalSections;           /* 038/006c */
    PVOID     CsrClientThread;                        /* 03c/0070 */
    PVOID     Win32ThreadInfo;                        /* 040/0078 */
    ULONG     User32Reserved[26];                     /* 044/0080 */
    ULONG     UserReserved[5];                        /* 0ac/00e8 */
    PVOID     WOW32Reserved;                          /* 0c0/0100 */
    ULONG     CurrentLocale;                          /* 0c4/0108 */
    ULONG     FpSoftwareStatusRegister;               /* 0c8/010c */
    PVOID     ReservedForDebuggerInstrumentation[16]; /* 0cc/0110 */
#ifdef _WIN64
    PVOID SystemReserved1[30];                        /*    /0190 */
#else
    PVOID SystemReserved1[26]; /* 10c/     used for krnl386 private data in Wine */
#endif
    char                      PlaceholderCompatibilityMode;       /* 174/0280 */
    BOOLEAN                   PlaceholderHydrationAlwaysExplicit; /* 175/0281 */
    char                      PlaceholderReserved[10];            /* 176/0282 */
    DWORD                     ProxiedProcessId;                   /* 180/028c */
    ACTIVATION_CONTEXT_STACK  ActivationContextStack;             /* 184/0290 */
    UCHAR                     WorkingOnBehalfOfTicket[8];         /* 19c/02b8 */
    LONG                      ExceptionCode;                      /* 1a4/02c0 */
    ACTIVATION_CONTEXT_STACK *ActivationContextStackPointer;      /* 1a8/02c8 */
    ULONG_PTR                 InstrumentationCallbackSp;          /* 1ac/02d0 */
    ULONG_PTR                 InstrumentationCallbackPreviousPc;  /* 1b0/02d8 */
    ULONG_PTR                 InstrumentationCallbackPreviousSp;  /* 1b4/02e0 */
#ifdef _WIN64
    ULONG   TxFsContext;                                          /*    /02e8 */
    BOOLEAN InstrumentationCallbackDisabled;                      /*    /02ec */
    BOOLEAN UnalignedLoadStoreExceptions;                         /*    /02ed */
#else
    BOOLEAN InstrumentationCallbackDisabled; /* 1b8/     */
    BYTE    SpareBytes1[23];                 /* 1b9/     */
    ULONG   TxFsContext;                     /* 1d0/     */
#endif
    GDI_TEB_BATCH  GdiTebBatch;              /* 1d4/02f0 used for ntdll private data in Wine */
    CLIENT_ID      RealClientId;             /* 6b4/07d8 */
    HANDLE         GdiCachedProcessHandle;   /* 6bc/07e8 */
    ULONG          GdiClientPID;             /* 6c0/07f0 */
    ULONG          GdiClientTID;             /* 6c4/07f4 */
    PVOID          GdiThreadLocaleInfo;      /* 6c8/07f8 */
    ULONG_PTR      Win32ClientInfo[62];      /* 6cc/0800 used for user32 private data in Wine */
    PVOID          glDispatchTable[233];     /* 7c4/09f0 */
    PVOID          glReserved1[29];          /* b68/1138 */
    PVOID          glReserved2;              /* bdc/1220 */
    PVOID          glSectionInfo;            /* be0/1228 */
    PVOID          glSection;                /* be4/1230 */
    PVOID          glTable;                  /* be8/1238 */
    PVOID          glCurrentRC;              /* bec/1240 */
    PVOID          glContext;                /* bf0/1248 */
    ULONG          LastStatusValue;          /* bf4/1250 */
    UNICODE_STRING StaticUnicodeString;      /* bf8/1258 */
    WCHAR          StaticUnicodeBuffer[261]; /* c00/1268 */
    PVOID          DeallocationStack;        /* e0c/1478 */
    PVOID          TlsSlots[64];             /* e10/1480 */
    LIST_ENTRY     TlsLinks;                 /* f10/1680 */
    PVOID          Vdm;                      /* f18/1690 */
    PVOID          ReservedForNtRpc;         /* f1c/1698 */
    PVOID          DbgSsReserved[2];         /* f20/16a0 */
    ULONG          HardErrorMode;            /* f28/16b0 */
#ifdef _WIN64
    PVOID Instrumentation[11];               /*    /16b8 */
#else
    PVOID Instrumentation[9]; /* f2c/ */
#endif
    GUID      ActivityId;              /* f50/1710 */
    PVOID     SubProcessTag;           /* f60/1720 */
    PVOID     PerflibData;             /* f64/1728 */
    PVOID     EtwTraceData;            /* f68/1730 */
    PVOID     WinSockData;             /* f6c/1738 */
    ULONG     GdiBatchCount;           /* f70/1740 */
    ULONG     IdealProcessorValue;     /* f74/1744 */
    ULONG     GuaranteedStackBytes;    /* f78/1748 */
    PVOID     ReservedForPerf;         /* f7c/1750 */
    PVOID     ReservedForOle;          /* f80/1758 */
    ULONG     WaitingOnLoaderLock;     /* f84/1760 */
    PVOID     SavedPriorityState;      /* f88/1768 */
    ULONG_PTR ReservedForCodeCoverage; /* f8c/1770 */
    PVOID     ThreadPoolData;          /* f90/1778 */
    PVOID    *TlsExpansionSlots;       /* f94/1780 */
#ifdef _WIN64
    union
    {
        PVOID                  DeallocationBStore; /*    /1788 */
        CHPE_V2_CPU_AREA_INFO *ChpeV2CpuAreaInfo;  /*    /1788 */
    } DUMMYUNIONNAME;

    PVOID BStoreLimit;                          /*    /1790 */
#endif
    ULONG             MuiGeneration;            /* f98/1798 */
    ULONG             IsImpersonating;          /* f9c/179c */
    PVOID             NlsCache;                 /* fa0/17a0 */
    PVOID             ShimData;                 /* fa4/17a8 */
    ULONG             HeapVirtualAffinity;      /* fa8/17b0 */
    PVOID             CurrentTransactionHandle; /* fac/17b8 */
    TEB_ACTIVE_FRAME *ActiveFrame;              /* fb0/17c0 */
    TEB_FLS_DATA     *FlsSlots;                 /* fb4/17c8 */
    PVOID             PreferredLanguages;       /* fb8/17d0 */
    PVOID             UserPrefLanguages;        /* fbc/17d8 */
    PVOID             MergedPrefLanguages;      /* fc0/17e0 */
    ULONG             MuiImpersonation;         /* fc4/17e8 */
    USHORT            CrossTebFlags;            /* fc8/17ec */

    union
    {
        USHORT SameTebFlags; /* fca/17ee */

        struct
        {
            USHORT SafeThunkCall        : 1;
            USHORT InDebugPrint         : 1;
            USHORT HasFiberData         : 1;
            USHORT SkipThreadAttach     : 1;
            USHORT WerInShipAssertCode  : 1;
            USHORT RanProcessInit       : 1;
            USHORT ClonedThread         : 1;
            USHORT SuppressDebugMsg     : 1;
            USHORT DisableUserStackWalk : 1;
            USHORT RtlExceptionAttached : 1;
            USHORT InitialThread        : 1;
            USHORT SessionAware         : 1;
            USHORT LoadOwner            : 1;
            USHORT LoaderWorker         : 1;
            USHORT SkipLoaderInit       : 1;
            USHORT SkipFileAPIBrokering : 1;
        };
    };

    PVOID     TxnScopeEnterCallback; /* fcc/17f0 */
    PVOID     TxnScopeExitCallback;  /* fd0/17f8 */
    PVOID     TxnScopeContext;       /* fd4/1800 */
    ULONG     LockCount;             /* fd8/1808 */
    LONG      WowTebOffset;          /* fdc/180c */
    PVOID     ResourceRetValue;      /* fe0/1810 */
    PVOID     ReservedForWdf;        /* fe4/1818 */
    ULONGLONG ReservedForCrt;        /* fe8/1820 */
    GUID      EffectiveContainerId;  /* ff0/1828 */
} TEB, *PTEB;

/***********************************************************************
 * The 32-bit/64-bit version of the PEB and TEB for WoW64
 */

typedef struct RTL_DRIVE_LETTER_CURDIR32
{
    USHORT           Flags;
    USHORT           Length;
    ULONG            TimeStamp;
    UNICODE_STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32;

typedef struct RTL_DRIVE_LETTER_CURDIR64
{
    USHORT           Flags;
    USHORT           Length;
    ULONG            TimeStamp;
    UNICODE_STRING64 DosPath;
} RTL_DRIVE_LETTER_CURDIR64;

typedef struct _RTL_USER_PROCESS_PARAMETERS32
{
    ULONG                     AllocationSize;
    ULONG                     Size;
    ULONG                     Flags;
    ULONG                     DebugFlags;
    ULONG                     ConsoleHandle;
    ULONG                     ConsoleFlags;
    ULONG                     hStdInput;
    ULONG                     hStdOutput;
    ULONG                     hStdError;
    CURDIR32                  CurrentDirectory;
    UNICODE_STRING32          DllPath;
    UNICODE_STRING32          ImagePathName;
    UNICODE_STRING32          CommandLine;
    ULONG                     Environment;
    ULONG                     dwX;
    ULONG                     dwY;
    ULONG                     dwXSize;
    ULONG                     dwYSize;
    ULONG                     dwXCountChars;
    ULONG                     dwYCountChars;
    ULONG                     dwFillAttribute;
    ULONG                     dwFlags;
    ULONG                     wShowWindow;
    UNICODE_STRING32          WindowTitle;
    UNICODE_STRING32          Desktop;
    UNICODE_STRING32          ShellInfo;
    UNICODE_STRING32          RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR32 DLCurrentDirectory[0x20];
    ULONG                     EnvironmentSize;
    ULONG                     EnvironmentVersion;
    ULONG                     PackageDependencyData;
    ULONG                     ProcessGroupId;
    ULONG                     LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS32;

typedef struct _RTL_USER_PROCESS_PARAMETERS64
{
    ULONG                     AllocationSize;
    ULONG                     Size;
    ULONG                     Flags;
    ULONG                     DebugFlags;
    ULONG64                   ConsoleHandle;
    ULONG                     ConsoleFlags;
    ULONG64                   hStdInput;
    ULONG64                   hStdOutput;
    ULONG64                   hStdError;
    CURDIR64                  CurrentDirectory;
    UNICODE_STRING64          DllPath;
    UNICODE_STRING64          ImagePathName;
    UNICODE_STRING64          CommandLine;
    ULONG64                   Environment;
    ULONG                     dwX;
    ULONG                     dwY;
    ULONG                     dwXSize;
    ULONG                     dwYSize;
    ULONG                     dwXCountChars;
    ULONG                     dwYCountChars;
    ULONG                     dwFillAttribute;
    ULONG                     dwFlags;
    ULONG                     wShowWindow;
    UNICODE_STRING64          WindowTitle;
    UNICODE_STRING64          Desktop;
    UNICODE_STRING64          ShellInfo;
    UNICODE_STRING64          RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR64 DLCurrentDirectory[0x20];
    ULONG64                   EnvironmentSize;
    ULONG64                   EnvironmentVersion;
    ULONG64                   PackageDependencyData;
    ULONG                     ProcessGroupId;
    ULONG                     LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS64;

typedef struct _PEB32
{
    BOOLEAN          InheritedAddressSpace;            /* 0000 */
    BOOLEAN          ReadImageFileExecOptions;         /* 0001 */
    BOOLEAN          BeingDebugged;                    /* 0002 */
    UCHAR            ImageUsedLargePages          : 1; /* 0003 */
    UCHAR            IsProtectedProcess           : 1;
    UCHAR            IsImageDynamicallyRelocated  : 1;
    UCHAR            SkipPatchingUser32Forwarders : 1;
    UCHAR            IsPackagedProcess            : 1;
    UCHAR            IsAppContainer               : 1;
    UCHAR            IsProtectedProcessLight      : 1;
    UCHAR            IsLongPathAwareProcess       : 1;
    ULONG            Mutant;                         /* 0004 */
    ULONG            ImageBaseAddress;               /* 0008 */
    ULONG            LdrData;                        /* 000c */
    ULONG            ProcessParameters;              /* 0010 */
    ULONG            SubSystemData;                  /* 0014 */
    ULONG            ProcessHeap;                    /* 0018 */
    ULONG            FastPebLock;                    /* 001c */
    ULONG            AtlThunkSListPtr;               /* 0020 */
    ULONG            IFEOKey;                        /* 0024 */
    ULONG            ProcessInJob               : 1; /* 0028 */
    ULONG            ProcessInitializing        : 1;
    ULONG            ProcessUsingVEH            : 1;
    ULONG            ProcessUsingVCH            : 1;
    ULONG            ProcessUsingFTH            : 1;
    ULONG            ProcessPreviouslyThrottled : 1;
    ULONG            ProcessCurrentlyThrottled  : 1;
    ULONG            ProcessImagesHotPatched    : 1;
    ULONG            ReservedBits0              : 24;
    ULONG            KernelCallbackTable;            /* 002c */
    ULONG            Reserved;                       /* 0030 */
    ULONG            AtlThunkSListPtr32;             /* 0034 */
    ULONG            ApiSetMap;                      /* 0038 */
    ULONG            TlsExpansionCounter;            /* 003c */
    ULONG            TlsBitmap;                      /* 0040 */
    ULONG            TlsBitmapBits[2];               /* 0044 */
    ULONG            ReadOnlySharedMemoryBase;       /* 004c */
    ULONG            SharedData;                     /* 0050 */
    ULONG            ReadOnlyStaticServerData;       /* 0054 */
    ULONG            AnsiCodePageData;               /* 0058 */
    ULONG            OemCodePageData;                /* 005c */
    ULONG            UnicodeCaseTableData;           /* 0060 */
    ULONG            NumberOfProcessors;             /* 0064 */
    ULONG            NtGlobalFlag;                   /* 0068 */
    LARGE_INTEGER    CriticalSectionTimeout;         /* 0070 */
    ULONG            HeapSegmentReserve;             /* 0078 */
    ULONG            HeapSegmentCommit;              /* 007c */
    ULONG            HeapDeCommitTotalFreeThreshold; /* 0080 */
    ULONG            HeapDeCommitFreeBlockThreshold; /* 0084 */
    ULONG            NumberOfHeaps;                  /* 0088 */
    ULONG            MaximumNumberOfHeaps;           /* 008c */
    ULONG            ProcessHeaps;                   /* 0090 */
    ULONG            GdiSharedHandleTable;           /* 0094 */
    ULONG            ProcessStarterHelper;           /* 0098 */
    ULONG            GdiDCAttributeList;             /* 009c */
    ULONG            LoaderLock;                     /* 00a0 */
    ULONG            OSMajorVersion;                 /* 00a4 */
    ULONG            OSMinorVersion;                 /* 00a8 */
    ULONG            OSBuildNumber;                  /* 00ac */
    ULONG            OSPlatformId;                   /* 00b0 */
    ULONG            ImageSubSystem;                 /* 00b4 */
    ULONG            ImageSubSystemMajorVersion;     /* 00b8 */
    ULONG            ImageSubSystemMinorVersion;     /* 00bc */
    ULONG            ActiveProcessAffinityMask;      /* 00c0 */
    ULONG            GdiHandleBuffer[34];            /* 00c4 */
    ULONG            PostProcessInitRoutine;         /* 014c */
    ULONG            TlsExpansionBitmap;             /* 0150 */
    ULONG            TlsExpansionBitmapBits[32];     /* 0154 */
    ULONG            SessionId;                      /* 01d4 */
    ULARGE_INTEGER   AppCompatFlags;                 /* 01d8 */
    ULARGE_INTEGER   AppCompatFlagsUser;             /* 01e0 */
    ULONG            ShimData;                       /* 01e8 */
    ULONG            AppCompatInfo;                  /* 01ec */
    UNICODE_STRING32 CSDVersion;                     /* 01f0 */
    ULONG            ActivationContextData;          /* 01f8 */
    ULONG            ProcessAssemblyStorageMap;      /* 01fc */
    ULONG            SystemDefaultActivationData;    /* 0200 */
    ULONG            SystemAssemblyStorageMap;       /* 0204 */
    ULONG            MinimumStackCommit;             /* 0208 */
    ULONG            SparePointers[2];               /* 020c */
    ULONG            PatchLoaderData;                /* 0214 */
    ULONG            ChpeV2ProcessInfo;              /* 0218 */
    ULONG            AppModelFeatureState;           /* 021c */
    ULONG            SpareUlongs[2];                 /* 0220 */
    USHORT           ActiveCodePage;                 /* 0228 */
    USHORT           OemCodePage;                    /* 022a */
    USHORT           UseCaseMapping;                 /* 022c */
    USHORT           UnusedNlsField;                 /* 022e */
    ULONG            WerRegistrationData;            /* 0230 */
    ULONG            WerShipAssertPtr;               /* 0234 */
    ULONG            pUnused;                        /* 0238 */
    ULONG            pImageHeaderHash;               /* 023c */
    ULONG            HeapTracingEnabled      : 1;    /* 0240 */
    ULONG            CritSecTracingEnabled   : 1;
    ULONG            LibLoaderTracingEnabled : 1;
    ULONG            SpareTracingBits        : 29;
    ULONGLONG        CsrServerReadOnlySharedMemoryBase;       /* 0248 */
    ULONG            TppWorkerpListLock;                      /* 0250 */
    LIST_ENTRY32     TppWorkerpList;                          /* 0254 */
    ULONG            WaitOnAddressHashTable[0x80];            /* 025c */
    ULONG            TelemetryCoverageHeader;                 /* 045c */
    ULONG            CloudFileFlags;                          /* 0460 */
    ULONG            CloudFileDiagFlags;                      /* 0464 */
    CHAR             PlaceholderCompatibilityMode;            /* 0468 */
    CHAR             PlaceholderCompatibilityModeReserved[7]; /* 0469 */
    ULONG            LeapSecondData;                          /* 0470 */
    ULONG            LeapSecondFlags;                         /* 0474 */
    ULONG            NtGlobalFlag2;                           /* 0478 */
} PEB32;

C_ASSERT(sizeof(PEB32) == 0x480);

typedef struct _PEB64
{
    BOOLEAN          InheritedAddressSpace;            /* 0000 */
    BOOLEAN          ReadImageFileExecOptions;         /* 0001 */
    BOOLEAN          BeingDebugged;                    /* 0002 */
    UCHAR            ImageUsedLargePages          : 1; /* 0003 */
    UCHAR            IsProtectedProcess           : 1;
    UCHAR            IsImageDynamicallyRelocated  : 1;
    UCHAR            SkipPatchingUser32Forwarders : 1;
    UCHAR            IsPackagedProcess            : 1;
    UCHAR            IsAppContainer               : 1;
    UCHAR            IsProtectedProcessLight      : 1;
    UCHAR            IsLongPathAwareProcess       : 1;
    ULONG64          Mutant;                         /* 0008 */
    ULONG64          ImageBaseAddress;               /* 0010 */
    ULONG64          LdrData;                        /* 0018 */
    ULONG64          ProcessParameters;              /* 0020 */
    ULONG64          SubSystemData;                  /* 0028 */
    ULONG64          ProcessHeap;                    /* 0030 */
    ULONG64          FastPebLock;                    /* 0038 */
    ULONG64          AtlThunkSListPtr;               /* 0040 */
    ULONG64          IFEOKey;                        /* 0048 */
    ULONG            ProcessInJob               : 1; /* 0050 */
    ULONG            ProcessInitializing        : 1;
    ULONG            ProcessUsingVEH            : 1;
    ULONG            ProcessUsingVCH            : 1;
    ULONG            ProcessUsingFTH            : 1;
    ULONG            ProcessPreviouslyThrottled : 1;
    ULONG            ProcessCurrentlyThrottled  : 1;
    ULONG            ProcessImagesHotPatched    : 1;
    ULONG            ReservedBits0              : 24;
    ULONG64          KernelCallbackTable;            /* 0058 */
    ULONG            Reserved;                       /* 0060 */
    ULONG            AtlThunkSListPtr32;             /* 0064 */
    ULONG64          ApiSetMap;                      /* 0068 */
    ULONG            TlsExpansionCounter;            /* 0070 */
    ULONG64          TlsBitmap;                      /* 0078 */
    ULONG            TlsBitmapBits[2];               /* 0080 */
    ULONG64          ReadOnlySharedMemoryBase;       /* 0088 */
    ULONG64          SharedData;                     /* 0090 */
    ULONG64          ReadOnlyStaticServerData;       /* 0098 */
    ULONG64          AnsiCodePageData;               /* 00a0 */
    ULONG64          OemCodePageData;                /* 00a8 */
    ULONG64          UnicodeCaseTableData;           /* 00b0 */
    ULONG            NumberOfProcessors;             /* 00b8 */
    ULONG            NtGlobalFlag;                   /* 00bc */
    LARGE_INTEGER    CriticalSectionTimeout;         /* 00c0 */
    ULONG64          HeapSegmentReserve;             /* 00c8 */
    ULONG64          HeapSegmentCommit;              /* 00d0 */
    ULONG64          HeapDeCommitTotalFreeThreshold; /* 00d8 */
    ULONG64          HeapDeCommitFreeBlockThreshold; /* 00e0 */
    ULONG            NumberOfHeaps;                  /* 00e8 */
    ULONG            MaximumNumberOfHeaps;           /* 00ec */
    ULONG64          ProcessHeaps;                   /* 00f0 */
    ULONG64          GdiSharedHandleTable;           /* 00f8 */
    ULONG64          ProcessStarterHelper;           /* 0100 */
    ULONG64          GdiDCAttributeList;             /* 0108 */
    ULONG64          LoaderLock;                     /* 0110 */
    ULONG            OSMajorVersion;                 /* 0118 */
    ULONG            OSMinorVersion;                 /* 011c */
    ULONG            OSBuildNumber;                  /* 0120 */
    ULONG            OSPlatformId;                   /* 0124 */
    ULONG            ImageSubSystem;                 /* 0128 */
    ULONG            ImageSubSystemMajorVersion;     /* 012c */
    ULONG            ImageSubSystemMinorVersion;     /* 0130 */
    ULONG64          ActiveProcessAffinityMask;      /* 0138 */
    ULONG            GdiHandleBuffer[60];            /* 0140 */
    ULONG64          PostProcessInitRoutine;         /* 0230 */
    ULONG64          TlsExpansionBitmap;             /* 0238 */
    ULONG            TlsExpansionBitmapBits[32];     /* 0240 */
    ULONG            SessionId;                      /* 02c0 */
    ULARGE_INTEGER   AppCompatFlags;                 /* 02c8 */
    ULARGE_INTEGER   AppCompatFlagsUser;             /* 02d0 */
    ULONG64          ShimData;                       /* 02d8 */
    ULONG64          AppCompatInfo;                  /* 02e0 */
    UNICODE_STRING64 CSDVersion;                     /* 02e8 */
    ULONG64          ActivationContextData;          /* 02f8 */
    ULONG64          ProcessAssemblyStorageMap;      /* 0300 */
    ULONG64          SystemDefaultActivationData;    /* 0308 */
    ULONG64          SystemAssemblyStorageMap;       /* 0310 */
    ULONG64          MinimumStackCommit;             /* 0318 */
    ULONG64          SparePointers[2];               /* 0320 */
    ULONG64          PatchLoaderData;                /* 0330 */
    ULONG64          ChpeV2ProcessInfo;              /* 0338 */
    ULONG            AppModelFeatureState;           /* 0340 */
    ULONG            SpareUlongs[2];                 /* 0344 */
    USHORT           ActiveCodePage;                 /* 034c */
    USHORT           OemCodePage;                    /* 034e */
    USHORT           UseCaseMapping;                 /* 0350 */
    USHORT           UnusedNlsField;                 /* 0352 */
    ULONG64          WerRegistrationData;            /* 0358 */
    ULONG64          WerShipAssertPtr;               /* 0360 */
    ULONG64          pUnused;                        /* 0368 */
    ULONG64          pImageHeaderHash;               /* 0370 */
    ULONG            HeapTracingEnabled      : 1;    /* 0378 */
    ULONG            CritSecTracingEnabled   : 1;
    ULONG            LibLoaderTracingEnabled : 1;
    ULONG            SpareTracingBits        : 29;
    ULONGLONG        CsrServerReadOnlySharedMemoryBase;       /* 0380 */
    ULONG            TppWorkerpListLock;                      /* 0388 */
    LIST_ENTRY64     TppWorkerpList;                          /* 0390 */
    ULONG64          WaitOnAddressHashTable[0x80];            /* 03a0 */
    ULONG64          TelemetryCoverageHeader;                 /* 07a0 */
    ULONG            CloudFileFlags;                          /* 07a8 */
    ULONG            CloudFileDiagFlags;                      /* 07ac */
    CHAR             PlaceholderCompatibilityMode;            /* 07b0 */
    CHAR             PlaceholderCompatibilityModeReserved[7]; /* 07b1 */
    ULONG64          LeapSecondData;                          /* 07b8 */
    ULONG            LeapSecondFlags;                         /* 07c0 */
    ULONG            NtGlobalFlag2;                           /* 07c4 */
} PEB64;

C_ASSERT(sizeof(PEB64) == 0x7c8);

typedef struct _TEB32
{
    NT_TIB32                   Tib;                                    /* 0000 */
    ULONG                      EnvironmentPointer;                     /* 001c */
    CLIENT_ID32                ClientId;                               /* 0020 */
    ULONG                      ActiveRpcHandle;                        /* 0028 */
    ULONG                      ThreadLocalStoragePointer;              /* 002c */
    ULONG                      Peb;                                    /* 0030 */
    ULONG                      LastErrorValue;                         /* 0034 */
    ULONG                      CountOfOwnedCriticalSections;           /* 0038 */
    ULONG                      CsrClientThread;                        /* 003c */
    ULONG                      Win32ThreadInfo;                        /* 0040 */
    ULONG                      User32Reserved[26];                     /* 0044 */
    ULONG                      UserReserved[5];                        /* 00ac */
    ULONG                      WOW32Reserved;                          /* 00c0 */
    ULONG                      CurrentLocale;                          /* 00c4 */
    ULONG                      FpSoftwareStatusRegister;               /* 00c8 */
    ULONG                      ReservedForDebuggerInstrumentation[16]; /* 00cc */
    ULONG                      SystemReserved1[26];                    /* 010c */
    char                       PlaceholderCompatibilityMode;           /* 0174 */
    BOOLEAN                    PlaceholderHydrationAlwaysExplicit;     /* 0175 */
    char                       PlaceholderReserved[10];                /* 0176 */
    DWORD                      ProxiedProcessId;                       /* 0180 */
    ACTIVATION_CONTEXT_STACK32 ActivationContextStack;                 /* 0184 */
    UCHAR                      WorkingOnBehalfOfTicket[8];             /* 019c */
    LONG                       ExceptionCode;                          /* 01a4 */
    ULONG                      ActivationContextStackPointer;          /* 01a8 */
    ULONG                      InstrumentationCallbackSp;              /* 01ac */
    ULONG                      InstrumentationCallbackPreviousPc;      /* 01b0 */
    ULONG                      InstrumentationCallbackPreviousSp;      /* 01b4 */
    BOOLEAN                    InstrumentationCallbackDisabled;        /* 01b8 */
    BYTE                       SpareBytes1[23];                        /* 01b9 */
    ULONG                      TxFsContext;                            /* 01d0 */
    ULONG                      GdiTebBatch[0x138];                     /* 01d4 */
    CLIENT_ID32                RealClientId;                           /* 06b4 */
    ULONG                      GdiCachedProcessHandle;                 /* 06bc */
    ULONG                      GdiClientPID;                           /* 06c0 */
    ULONG                      GdiClientTID;                           /* 06c4 */
    ULONG                      GdiThreadLocaleInfo;                    /* 06c8 */
    ULONG                      Win32ClientInfo[62];                    /* 06cc */
    ULONG                      glDispatchTable[233];                   /* 07c4 */
    ULONG                      glReserved1[29];                        /* 0b68 */
    ULONG                      glReserved2;                            /* 0bdc */
    ULONG                      glSectionInfo;                          /* 0be0 */
    ULONG                      glSection;                              /* 0be4 */
    ULONG                      glTable;                                /* 0be8 */
    ULONG                      glCurrentRC;                            /* 0bec */
    ULONG                      glContext;                              /* 0bf0 */
    ULONG                      LastStatusValue;                        /* 0bf4 */
    UNICODE_STRING32           StaticUnicodeString;                    /* 0bf8 */
    WCHAR                      StaticUnicodeBuffer[261];               /* 0c00 */
    ULONG                      DeallocationStack;                      /* 0e0c */
    ULONG                      TlsSlots[64];                           /* 0e10 */
    LIST_ENTRY32               TlsLinks;                               /* 0f10 */
    ULONG                      Vdm;                                    /* 0f18 */
    ULONG                      ReservedForNtRpc;                       /* 0f1c */
    ULONG                      DbgSsReserved[2];                       /* 0f20 */
    ULONG                      HardErrorMode;                          /* 0f28 */
    ULONG                      Instrumentation[9];                     /* 0f2c */
    GUID                       ActivityId;                             /* 0f50 */
    ULONG                      SubProcessTag;                          /* 0f60 */
    ULONG                      PerflibData;                            /* 0f64 */
    ULONG                      EtwTraceData;                           /* 0f68 */
    ULONG                      WinSockData;                            /* 0f6c */
    ULONG                      GdiBatchCount;                          /* 0f70 */
    ULONG                      IdealProcessorValue;                    /* 0f74 */
    ULONG                      GuaranteedStackBytes;                   /* 0f78 */
    ULONG                      ReservedForPerf;                        /* 0f7c */
    ULONG                      ReservedForOle;                         /* 0f80 */
    ULONG                      WaitingOnLoaderLock;                    /* 0f84 */
    ULONG                      SavedPriorityState;                     /* 0f88 */
    ULONG                      ReservedForCodeCoverage;                /* 0f8c */
    ULONG                      ThreadPoolData;                         /* 0f90 */
    ULONG                      TlsExpansionSlots;                      /* 0f94 */
    ULONG                      MuiGeneration;                          /* 0f98 */
    ULONG                      IsImpersonating;                        /* 0f9c */
    ULONG                      NlsCache;                               /* 0fa0 */
    ULONG                      ShimData;                               /* 0fa4 */
    ULONG                      HeapVirtualAffinity;                    /* 0fa8 */
    ULONG                      CurrentTransactionHandle;               /* 0fac */
    ULONG                      ActiveFrame;                            /* 0fb0 */
    ULONG                      FlsSlots;                               /* 0fb4 */
    ULONG                      PreferredLanguages;                     /* 0fb8 */
    ULONG                      UserPrefLanguages;                      /* 0fbc */
    ULONG                      MergedPrefLanguages;                    /* 0fc0 */
    ULONG                      MuiImpersonation;                       /* 0fc4 */
    USHORT                     CrossTebFlags;                          /* 0fc8 */

    union
    {
        USHORT SameTebFlags; /* 0fca */

        struct
        {
            USHORT SafeThunkCall        : 1;
            USHORT InDebugPrint         : 1;
            USHORT HasFiberData         : 1;
            USHORT SkipThreadAttach     : 1;
            USHORT WerInShipAssertCode  : 1;
            USHORT RanProcessInit       : 1;
            USHORT ClonedThread         : 1;
            USHORT SuppressDebugMsg     : 1;
            USHORT DisableUserStackWalk : 1;
            USHORT RtlExceptionAttached : 1;
            USHORT InitialThread        : 1;
            USHORT SessionAware         : 1;
            USHORT LoadOwner            : 1;
            USHORT LoaderWorker         : 1;
            USHORT SkipLoaderInit       : 1;
            USHORT SkipFileAPIBrokering : 1;
        };
    };

    ULONG     TxnScopeEnterCallback; /* 0fcc */
    ULONG     TxnScopeExitCallback;  /* 0fd0 */
    ULONG     TxnScopeContext;       /* 0fd4 */
    ULONG     LockCount;             /* 0fd8 */
    LONG      WowTebOffset;          /* 0fdc */
    ULONG     ResourceRetValue;      /* 0fe0 */
    ULONG     ReservedForWdf;        /* 0fe4 */
    ULONGLONG ReservedForCrt;        /* 0fe8 */
    GUID      EffectiveContainerId;  /* 0ff0 */
} TEB32;

C_ASSERT(sizeof(TEB32) == 0x1000);

typedef struct _TEB64
{
    NT_TIB64                   Tib;                                    /* 0000 */
    ULONG64                    EnvironmentPointer;                     /* 0038 */
    CLIENT_ID64                ClientId;                               /* 0040 */
    ULONG64                    ActiveRpcHandle;                        /* 0050 */
    ULONG64                    ThreadLocalStoragePointer;              /* 0058 */
    ULONG64                    Peb;                                    /* 0060 */
    ULONG                      LastErrorValue;                         /* 0068 */
    ULONG                      CountOfOwnedCriticalSections;           /* 006c */
    ULONG64                    CsrClientThread;                        /* 0070 */
    ULONG64                    Win32ThreadInfo;                        /* 0078 */
    ULONG                      User32Reserved[26];                     /* 0080 */
    ULONG                      UserReserved[5];                        /* 00e8 */
    ULONG64                    WOW32Reserved;                          /* 0100 */
    ULONG                      CurrentLocale;                          /* 0108 */
    ULONG                      FpSoftwareStatusRegister;               /* 010c */
    ULONG64                    ReservedForDebuggerInstrumentation[16]; /* 0110 */
    ULONG64                    SystemReserved1[30];                    /* 0190 */
    char                       PlaceholderCompatibilityMode;           /* 0280 */
    BOOLEAN                    PlaceholderHydrationAlwaysExplicit;     /* 0281 */
    char                       PlaceholderReserved[10];                /* 0282 */
    DWORD                      ProxiedProcessId;                       /* 028c */
    ACTIVATION_CONTEXT_STACK64 ActivationContextStack;                 /* 0290 */
    UCHAR                      WorkingOnBehalfOfTicket[8];             /* 02b8 */
    LONG                       ExceptionCode;                          /* 02c0 */
    ULONG64                    ActivationContextStackPointer;          /* 02c8 */
    ULONG64                    InstrumentationCallbackSp;              /* 02d0 */
    ULONG64                    InstrumentationCallbackPreviousPc;      /* 02d8 */
    ULONG64                    InstrumentationCallbackPreviousSp;      /* 02e0 */
    ULONG                      TxFsContext;                            /* 02e8 */
    BOOLEAN                    InstrumentationCallbackDisabled;        /* 02ec */
    BOOLEAN                    UnalignedLoadStoreExceptions;           /* 02ed */
    ULONG64                    GdiTebBatch[0x9d];                      /* 02f0 */
    CLIENT_ID64                RealClientId;                           /* 07d8 */
    ULONG64                    GdiCachedProcessHandle;                 /* 07e8 */
    ULONG                      GdiClientPID;                           /* 07f0 */
    ULONG                      GdiClientTID;                           /* 07f4 */
    ULONG64                    GdiThreadLocaleInfo;                    /* 07f8 */
    ULONG64                    Win32ClientInfo[62];                    /* 0800 */
    ULONG64                    glDispatchTable[233];                   /* 09f0 */
    ULONG64                    glReserved1[29];                        /* 1138 */
    ULONG64                    glReserved2;                            /* 1220 */
    ULONG64                    glSectionInfo;                          /* 1228 */
    ULONG64                    glSection;                              /* 1230 */
    ULONG64                    glTable;                                /* 1238 */
    ULONG64                    glCurrentRC;                            /* 1240 */
    ULONG64                    glContext;                              /* 1248 */
    ULONG                      LastStatusValue;                        /* 1250 */
    UNICODE_STRING64           StaticUnicodeString;                    /* 1258 */
    WCHAR                      StaticUnicodeBuffer[261];               /* 1268 */
    ULONG64                    DeallocationStack;                      /* 1478 */
    ULONG64                    TlsSlots[64];                           /* 1480 */
    LIST_ENTRY64               TlsLinks;                               /* 1680 */
    ULONG64                    Vdm;                                    /* 1690 */
    ULONG64                    ReservedForNtRpc;                       /* 1698 */
    ULONG64                    DbgSsReserved[2];                       /* 16a0 */
    ULONG                      HardErrorMode;                          /* 16b0 */
    ULONG64                    Instrumentation[11];                    /* 16b8 */
    GUID                       ActivityId;                             /* 1710 */
    ULONG64                    SubProcessTag;                          /* 1720 */
    ULONG64                    PerflibData;                            /* 1728 */
    ULONG64                    EtwTraceData;                           /* 1730 */
    ULONG64                    WinSockData;                            /* 1738 */
    ULONG                      GdiBatchCount;                          /* 1740 */
    ULONG                      IdealProcessorValue;                    /* 1744 */
    ULONG                      GuaranteedStackBytes;                   /* 1748 */
    ULONG64                    ReservedForPerf;                        /* 1750 */
    ULONG64                    ReservedForOle;                         /* 1758 */
    ULONG                      WaitingOnLoaderLock;                    /* 1760 */
    ULONG64                    SavedPriorityState;                     /* 1768 */
    ULONG64                    ReservedForCodeCoverage;                /* 1770 */
    ULONG64                    ThreadPoolData;                         /* 1778 */
    ULONG64                    TlsExpansionSlots;                      /* 1780 */

    union
    {
        ULONG64 DeallocationBStore; /* 1788 */
        ULONG64 ChpeV2CpuAreaInfo;  /* 1788 */
    } DUMMYUNIONNAME;

    ULONG64 BStoreLimit;              /* 1790 */
    ULONG   MuiGeneration;            /* 1798 */
    ULONG   IsImpersonating;          /* 179c */
    ULONG64 NlsCache;                 /* 17a0 */
    ULONG64 ShimData;                 /* 17a8 */
    ULONG   HeapVirtualAffinity;      /* 17b0 */
    ULONG64 CurrentTransactionHandle; /* 17b8 */
    ULONG64 ActiveFrame;              /* 17c0 */
    ULONG64 FlsSlots;                 /* 17c8 */
    ULONG64 PreferredLanguages;       /* 17d0 */
    ULONG64 UserPrefLanguages;        /* 17d8 */
    ULONG64 MergedPrefLanguages;      /* 17e0 */
    ULONG   MuiImpersonation;         /* 17e8 */
    USHORT  CrossTebFlags;            /* 17ec */

    union
    {
        USHORT SameTebFlags; /* 17ee */

        struct
        {
            USHORT SafeThunkCall        : 1;
            USHORT InDebugPrint         : 1;
            USHORT HasFiberData         : 1;
            USHORT SkipThreadAttach     : 1;
            USHORT WerInShipAssertCode  : 1;
            USHORT RanProcessInit       : 1;
            USHORT ClonedThread         : 1;
            USHORT SuppressDebugMsg     : 1;
            USHORT DisableUserStackWalk : 1;
            USHORT RtlExceptionAttached : 1;
            USHORT InitialThread        : 1;
            USHORT SessionAware         : 1;
            USHORT LoadOwner            : 1;
            USHORT LoaderWorker         : 1;
            USHORT SkipLoaderInit       : 1;
            USHORT SkipFileAPIBrokering : 1;
        };
    };

    ULONG64   TxnScopeEnterCallback; /* 17f0 */
    ULONG64   TxnScopeExitCallback;  /* 17f8 */
    ULONG64   TxnScopeContext;       /* 1800 */
    ULONG     LockCount;             /* 1808 */
    LONG      WowTebOffset;          /* 180c */
    ULONG64   ResourceRetValue;      /* 1810 */
    ULONG64   ReservedForWdf;        /* 1818 */
    ULONGLONG ReservedForCrt;        /* 1820 */
    GUID      EffectiveContainerId;  /* 1828 */
} TEB64;

C_ASSERT(sizeof(TEB64) == 0x1838);

#ifdef _WIN64
C_ASSERT(sizeof(PEB) == sizeof(PEB64));
C_ASSERT(sizeof(TEB) == sizeof(TEB64));
#else
C_ASSERT(sizeof(PEB) == sizeof(PEB32));
C_ASSERT(sizeof(TEB) == sizeof(TEB32));
#endif

/* reserved TEB64 TLS slots for Wow64 */
#define WOW64_TLS_CPURESERVED      1
#define WOW64_TLS_TEMPLIST         3
#define WOW64_TLS_USERCALLBACKDATA 5
#define WOW64_TLS_APCLIST          7
#define WOW64_TLS_FILESYSREDIR     8
#define WOW64_TLS_WOW64INFO        10
#define WOW64_TLS_MAX_NUMBER       19

/***********************************************************************
 * Enums
 */

typedef enum _RTL_PATH_TYPE
{
    RtlPathTypeUnknown = 0,
    RtlPathTypeBareLableAbsolute, /* "C:/foo" */
    RtlPathTypeBareLableRelative, /* "./foo" */
    RtlPathTypeUncAbsolute,       /* "//foo" */
    RtlPathTypeDriveAbsolute,     /* "c:/foo" */
    RtlPathTypeDriveRelative,     /* "c:foo" */
    RtlPathTypeRooted,            /* "/foo" */
    RtlPathTypeRelative,          /* "foo" */
    RtlPathTypeLocalDevice,       /* "//./foo" */
    RtlPathTypeRootLocalDevice    /* "//." */
} RTL_PATH_TYPE;

/***********************************************************************
 * Types and data structures
 */

typedef struct _PROCESS_BASIC_INFORMATION
{
#ifdef __WINESRC__
    NTSTATUS  ExitStatus;
    PEB      *PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG      BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
#else
    PVOID     Reserved1;
    PPEB      PebBaseAddress;
    PVOID     Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID     Reserved3;
#endif
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef NTSTATUS(NTAPI RTL_HEAP_COMMIT_ROUTINE)(PVOID base, PVOID *address, PSIZE_T size);
typedef RTL_HEAP_COMMIT_ROUTINE *PRTL_HEAP_COMMIT_ROUTINE;

typedef struct _RTL_HEAP_PARAMETERS
{
    ULONG                    Length;
    SIZE_T                   SegmentReserve;
    SIZE_T                   SegmentCommit;
    SIZE_T                   DeCommitFreeblockThreshold;
    SIZE_T                   DeCommitTotalFreeThreshold;
    SIZE_T                   MaximumAllocationSize;
    SIZE_T                   VirtualMemoryThreshold;
    SIZE_T                   InitialCommit;
    SIZE_T                   InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T                   Reserved[2];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

typedef struct _RTL_RWLOCK
{
    RTL_CRITICAL_SECTION rtlCS;

    HANDLE hSharedReleaseSemaphore;
    UINT   uSharedWaiters;

    HANDLE hExclusiveReleaseSemaphore;
    UINT   uExclusiveWaiters;

    INT    iNumberActive;
    HANDLE hOwningThreadId;
    DWORD  dwTimeoutBoost;
    PVOID  pDebugInfo;
} RTL_RWLOCK, *LPRTL_RWLOCK;

/* FIXME: names probably not correct */
typedef struct _RTL_HANDLE
{
    struct _RTL_HANDLE *Next;
} RTL_HANDLE;

/* FIXME: names probably not correct */
typedef struct _RTL_HANDLE_TABLE
{
    ULONG MaxHandleCount; /* 0x00 */
    ULONG HandleSize;     /* 0x04 */
    ULONG Unused[2];      /* 0x08-0x0c */
    PVOID NextFree;       /* 0x10 */
    PVOID FirstHandle;    /* 0x14 */
    PVOID ReservedMemory; /* 0x18 */
    PVOID MaxHandle;      /* 0x1c */
} RTL_HANDLE_TABLE;

typedef struct _RTL_ATOM_TABLE_ENTRY
{
    struct _RTL_ATOM_TABLE_ENTRY *HashLink;
    WORD                          HandleIndex;
    WORD                          Atom;
    WORD                          ReferenceCount;
    UCHAR                         Flags;
    UCHAR                         NameLength;
    WCHAR                         Name[1];
} RTL_ATOM_TABLE_ENTRY, *PRTL_ATOM_TABLE_ENTRY;

typedef struct _RTL_ATOM_TABLE
{
    ULONG                 Signature;
    RTL_CRITICAL_SECTION  CriticalSection;
    RTL_HANDLE_TABLE      HandleTable;
    ULONG                 NumberOfBuckets;
    RTL_ATOM_TABLE_ENTRY *Buckets[1];
} *RTL_ATOM_TABLE, **PRTL_ATOM_TABLE;

/***********************************************************************
 * Defines
 */

typedef void(CALLBACK *PRTL_THREAD_START_ROUTINE)(LPVOID);        /* FIXME: not the right name */
typedef DWORD(CALLBACK *PRTL_WORK_ITEM_ROUTINE)(LPVOID);          /* FIXME: not the right name */
typedef void(NTAPI *RTL_WAITORTIMERCALLBACKFUNC)(PVOID, BOOLEAN); /* FIXME: not the right name */

/* Rtl*Registry* functions structs and defines */
#define RTL_REGISTRY_ABSOLUTE   0
#define RTL_REGISTRY_SERVICES   1
#define RTL_REGISTRY_CONTROL    2
#define RTL_REGISTRY_WINDOWS_NT 3
#define RTL_REGISTRY_DEVICEMAP  4
#define RTL_REGISTRY_USER       5

#define RTL_REGISTRY_HANDLE   0x40000000
#define RTL_REGISTRY_OPTIONAL 0x80000000

#define RTL_QUERY_REGISTRY_SUBKEY    0x00000001
#define RTL_QUERY_REGISTRY_TOPKEY    0x00000002
#define RTL_QUERY_REGISTRY_REQUIRED  0x00000004
#define RTL_QUERY_REGISTRY_NOVALUE   0x00000008
#define RTL_QUERY_REGISTRY_NOEXPAND  0x00000010
#define RTL_QUERY_REGISTRY_DIRECT    0x00000020
#define RTL_QUERY_REGISTRY_DELETE    0x00000040
#define RTL_QUERY_REGISTRY_TYPECHECK 0x00000100

#define RTL_QUERY_REGISTRY_TYPECHECK_SHIFT 24

typedef NTSTATUS(WINAPI *PRTL_QUERY_REGISTRY_ROUTINE)(PCWSTR ValueName,
                                                      ULONG  ValueType,
                                                      PVOID  ValueData,
                                                      ULONG  ValueLength,
                                                      PVOID  Context,
                                                      PVOID  EntryContext);

typedef struct _RTL_QUERY_REGISTRY_TABLE
{
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG                       Flags;
    PWSTR                       Name;
    PVOID                       EntryContext;
    ULONG                       DefaultType;
    PVOID                       DefaultData;
    ULONG                       DefaultLength;
} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;

typedef VOID(CALLBACK *PRTL_OVERLAPPED_COMPLETION_ROUTINE)(DWORD, DWORD, LPVOID);

typedef struct _RTL_USER_PROCESS_INFORMATION
{
    ULONG                     Length;
    HANDLE                    Process;
    HANDLE                    Thread;
    CLIENT_ID                 ClientId;
    SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

/*************************************************************************
 * Loader structures
 *
 * Those are not part of standard Winternl.h
 */

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    PVOID Section;                       /* 00/00 */
    PVOID MappedBaseAddress;             /* 04/08 */
    PVOID ImageBaseAddress;              /* 08/10 */
    ULONG ImageSize;                     /* 0c/18 */
    ULONG Flags;                         /* 10/1c */
    WORD  LoadOrderIndex;                /* 14/20 */
    WORD  InitOrderIndex;                /* 16/22 */
    WORD  LoadCount;                     /* 18/24 */
    WORD  NameOffset;                    /* 1a/26 */
    BYTE  Name[MAXIMUM_FILENAME_LENGTH]; /* 1c/28 */
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG                          ModulesCount;
    RTL_PROCESS_MODULE_INFORMATION Modules[1]; /* FIXME: should be Modules[0] */
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

#define PROCESS_CREATE_FLAGS_BREAKAWAY              0x00000001
#define PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT       0x00000002
#define PROCESS_CREATE_FLAGS_INHERIT_HANDLES        0x00000004
#define PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00000008
#define PROCESS_CREATE_FLAGS_LARGE_PAGES            0x00000010
#define PROCESS_CREATE_FLAGS_LARGE_PAGE_SYSTEM_DLL  0x00000020
#define PROCESS_CREATE_FLAGS_PROTECTED_PROCESS      0x00000040
#define PROCESS_CREATE_FLAGS_CREATE_SESSION         0x00000080
#define PROCESS_CREATE_FLAGS_INHERIT_FROM_PARENT    0x00000100
#define PROCESS_CREATE_FLAGS_SUSPENDED              0x00000200
#define PROCESS_CREATE_FLAGS_EXTENDED_UNKNOWN       0x00000400

typedef struct _RTL_PROCESS_MODULE_INFORMATION_EX
{
    USHORT                         NextOffset;
    RTL_PROCESS_MODULE_INFORMATION BaseInfo;
    ULONG                          ImageCheckSum;
    ULONG                          TimeDateStamp;
    void                          *DefaultBase;
} RTL_PROCESS_MODULE_INFORMATION_EX;

#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED      0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH    0x00000002
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER    0x00000004
#define THREAD_CREATE_FLAGS_LOADER_WORKER         0x00000010
#define THREAD_CREATE_FLAGS_SKIP_LOADER_INIT      0x00000020
#define THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE 0x00000040
#define THREAD_CREATE_FLAGS_INITIAL_THREAD        0x00000080

#ifdef __WINESRC__

/* Wine-specific exceptions codes */

#define EXCEPTION_WINE_STUB      0x80000100 /* stub entry point called */
#define EXCEPTION_WINE_ASSERTION 0x80000101 /* assertion failed */

/* Wine extension; Windows doesn't have a name for this code.  This is an
   undocumented exception understood by MS VC debugger, allowing the program
   to name a particular thread. */
#define EXCEPTION_WINE_NAME_THREAD 0x406D1388

/* used for C++ exceptions in msvcrt
 * parameters:
 * [0] CXX_FRAME_MAGIC
 * [1] pointer to exception object
 * [2] pointer to type
 */
#define EXCEPTION_WINE_CXX_EXCEPTION   0xe06d7363
#define EXCEPTION_WINE_CXX_FRAME_MAGIC 0x19930520

#endif

typedef LONG(CALLBACK *PRTL_EXCEPTION_FILTER)(PEXCEPTION_POINTERS);
PTP_CALLBACK_INSTANCE

typedef void(CALLBACK *PTP_IO_CALLBACK)(PTP_CALLBACK_INSTANCE, void *, void *, IO_STATUS_BLOCK *, PTP_IO);

/***********************************************************************
 * Function declarations
 */

NTSYSAPI NTSTATUS WINAPI RtlAbsoluteToSelfRelativeSD(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PULONG);
NTSYSAPI void WINAPI RtlAcquirePebLock(void);
NTSYSAPI BYTE WINAPI RtlAcquireResourceExclusive(LPRTL_RWLOCK, BYTE);
NTSYSAPI BYTE WINAPI RtlAcquireResourceShared(LPRTL_RWLOCK, BYTE);
NTSYSAPI void WINAPI RtlAcquireSRWLockExclusive(RTL_SRWLOCK *);
NTSYSAPI void WINAPI RtlAcquireSRWLockShared(RTL_SRWLOCK *);
NTSYSAPI NTSTATUS WINAPI RtlActivateActivationContext(DWORD, struct _ACTIVATION_CONTEXT *, ULONG_PTR *);
NTSYSAPI NTSTATUS WINAPI RtlActivateActivationContextEx(ULONG, TEB *, struct _ACTIVATION_CONTEXT *, ULONG_PTR *);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessAllowedAce(PACL, DWORD, DWORD, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessAllowedAceEx(PACL, DWORD, DWORD, DWORD, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessAllowedObjectAce(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessDeniedAce(PACL, DWORD, DWORD, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessDeniedAceEx(PACL, DWORD, DWORD, DWORD, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAccessDeniedObjectAce(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddAce(PACL, DWORD, DWORD, PACE_HEADER, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlAddAtomToAtomTable(RTL_ATOM_TABLE, const WCHAR *, RTL_ATOM *);
NTSYSAPI NTSTATUS WINAPI RtlAddAuditAccessAce(PACL, DWORD, DWORD, PSID, BOOL, BOOL);
NTSYSAPI NTSTATUS WINAPI RtlAddAuditAccessAceEx(PACL, DWORD, DWORD, DWORD, PSID, BOOL, BOOL);
NTSYSAPI NTSTATUS WINAPI RtlAddAuditAccessObjectAce(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID, BOOL, BOOL);
NTSYSAPI NTSTATUS WINAPI RtlAddMandatoryAce(PACL, DWORD, DWORD, DWORD, DWORD, PSID);
NTSYSAPI NTSTATUS WINAPI RtlAddProcessTrustLabelAce(PACL, DWORD, DWORD, PSID, DWORD, DWORD);
NTSYSAPI void WINAPI RtlAddRefActivationContext(struct _ACTIVATION_CONTEXT *);
NTSYSAPI PVOID WINAPI RtlAddVectoredContinueHandler(ULONG, PVECTORED_EXCEPTION_HANDLER);
NTSYSAPI PVOID WINAPI RtlAddVectoredExceptionHandler(ULONG, PVECTORED_EXCEPTION_HANDLER);
NTSYSAPI PVOID WINAPI RtlAddressInSectionTable(const IMAGE_NT_HEADERS *, HMODULE, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlAllocateAndInitializeSid(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID *);
NTSYSAPI RTL_HANDLE *WINAPI RtlAllocateHandle(RTL_HANDLE_TABLE *, ULONG *);
NTSYSAPI BOOLEAN WINAPI RtlFreeHeap(HANDLE, ULONG, PVOID);
NTSYSAPI PVOID WINAPI   RtlAllocateHeap(HANDLE, ULONG, SIZE_T) __WINE_ALLOC_SIZE(3) __WINE_DEALLOC(RtlFreeHeap, 3) __WINE_MALLOC;
NTSYSAPI WCHAR WINAPI RtlAnsiCharToUnicodeChar(LPSTR *);
NTSYSAPI DWORD WINAPI RtlAnsiStringToUnicodeSize(const STRING *);
NTSYSAPI NTSTATUS WINAPI RtlAnsiStringToUnicodeString(PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlAppendAsciizToString(STRING *, LPCSTR);
NTSYSAPI NTSTATUS WINAPI RtlAppendStringToString(STRING *, const STRING *);
NTSYSAPI NTSTATUS WINAPI RtlAppendUnicodeStringToString(UNICODE_STRING *, const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlAppendUnicodeToString(UNICODE_STRING *, LPCWSTR);
NTSYSAPI BOOLEAN WINAPI RtlAreAllAccessesGranted(ACCESS_MASK, ACCESS_MASK);
NTSYSAPI BOOLEAN WINAPI RtlAreAnyAccessesGranted(ACCESS_MASK, ACCESS_MASK);
NTSYSAPI BOOLEAN WINAPI RtlAreBitsSet(PCRTL_BITMAP, ULONG, ULONG);
NTSYSAPI BOOLEAN WINAPI RtlAreBitsClear(PCRTL_BITMAP, ULONG, ULONG);
NTSYSAPI BOOLEAN WINAPI RtlBarrier(RTL_BARRIER *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlCharToInteger(PCSZ, ULONG, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlCheckRegistryKey(ULONG, PWSTR);
NTSYSAPI void WINAPI     RtlClearAllBits(PRTL_BITMAP);
NTSYSAPI void WINAPI     RtlClearBits(PRTL_BITMAP, ULONG, ULONG);
NTSYSAPI ULONG WINAPI    RtlCompactHeap(HANDLE, ULONG);
NTSYSAPI LONG WINAPI RtlCompareUnicodeString(const UNICODE_STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI LONG WINAPI RtlCompareUnicodeStrings(const WCHAR *, SIZE_T, const WCHAR *, SIZE_T, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlCompressBuffer(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, ULONG, PULONG, PVOID);
NTSYSAPI DWORD WINAPI RtlComputeCrc32(DWORD, const BYTE *, INT);
NTSYSAPI NTSTATUS WINAPI RtlConvertSidToUnicodeString(PUNICODE_STRING, PSID, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlConvertToAutoInheritSecurityObject(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR *, GUID *, BOOL, PGENERIC_MAPPING);
NTSYSAPI NTSTATUS WINAPI RtlCopyContext(CONTEXT *, DWORD, CONTEXT *);
NTSYSAPI NTSTATUS WINAPI RtlCopyExtendedContext(CONTEXT_EX *, ULONG, CONTEXT_EX *);
NTSYSAPI void WINAPI RtlCopyLuid(PLUID, const LUID *);
NTSYSAPI void WINAPI RtlCopyLuidAndAttributesArray(ULONG, const LUID_AND_ATTRIBUTES *, PLUID_AND_ATTRIBUTES);
NTSYSAPI NTSTATUS WINAPI RtlCopySecurityDescriptor(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR);
NTSYSAPI BOOLEAN WINAPI  RtlCopySid(DWORD, PSID, PSID);
NTSYSAPI void WINAPI RtlCopyUnicodeString(UNICODE_STRING *, const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlCreateAcl(PACL, DWORD, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlCreateActivationContext(struct _ACTIVATION_CONTEXT **, const void *);
NTSYSAPI NTSTATUS WINAPI RtlCreateAtomTable(ULONG, RTL_ATOM_TABLE *);
NTSYSAPI NTSTATUS WINAPI RtlCreateEnvironment(BOOLEAN, PWSTR *);
NTSYSAPI HANDLE WINAPI RtlCreateHeap(ULONG, PVOID, SIZE_T, SIZE_T, PVOID, PRTL_HEAP_PARAMETERS);
NTSYSAPI NTSTATUS WINAPI RtlCreateProcessParameters(RTL_USER_PROCESS_PARAMETERS **, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, PWSTR, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlCreateProcessParametersEx(RTL_USER_PROCESS_PARAMETERS **, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, PWSTR, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, const UNICODE_STRING *, ULONG);
NTSYSAPI PDEBUG_BUFFER WINAPI RtlCreateQueryDebugBuffer(ULONG, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI      RtlCreateRegistryKey(ULONG, PWSTR);
NTSYSAPI NTSTATUS WINAPI      RtlCreateSecurityDescriptor(PSECURITY_DESCRIPTOR, DWORD);
NTSYSAPI NTSTATUS WINAPI      RtlCreateServiceSid(PUNICODE_STRING, PSID, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlCreateTimer(HANDLE, HANDLE *, RTL_WAITORTIMERCALLBACKFUNC, PVOID, DWORD, DWORD, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlCreateTimerQueue(PHANDLE);
NTSYSAPI BOOLEAN WINAPI  RtlCreateUnicodeString(PUNICODE_STRING, LPCWSTR);
NTSYSAPI BOOLEAN WINAPI  RtlCreateUnicodeStringFromAsciiz(PUNICODE_STRING, LPCSTR);
NTSYSAPI NTSTATUS WINAPI RtlCreateUserProcess(UNICODE_STRING *, ULONG, RTL_USER_PROCESS_PARAMETERS *, SECURITY_DESCRIPTOR *, SECURITY_DESCRIPTOR *, HANDLE, BOOLEAN, HANDLE, HANDLE, RTL_USER_PROCESS_INFORMATION *);
NTSYSAPI NTSTATUS WINAPI RtlCreateUserStack(SIZE_T, SIZE_T, ULONG, SIZE_T, SIZE_T, INITIAL_TEB *);
NTSYSAPI NTSTATUS WINAPI RtlCreateUserThread(HANDLE, SECURITY_DESCRIPTOR *, BOOLEAN, ULONG, SIZE_T, SIZE_T, PRTL_THREAD_START_ROUTINE, void *, HANDLE *, CLIENT_ID *);
NTSYSAPI NTSTATUS WINAPI RtlCustomCPToUnicodeN(CPTABLEINFO *, WCHAR *, DWORD, DWORD *, const char *, DWORD);
NTSYSAPI PRTL_USER_PROCESS_PARAMETERS WINAPI RtlDeNormalizeProcessParams(RTL_USER_PROCESS_PARAMETERS *);
NTSYSAPI void WINAPI     RtlDeactivateActivationContext(DWORD, ULONG_PTR);
NTSYSAPI PVOID WINAPI    RtlDecodePointer(PVOID);
NTSYSAPI NTSTATUS WINAPI RtlDecompressBuffer(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlDecompressFragment(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, ULONG, PULONG, PVOID);
NTSYSAPI NTSTATUS WINAPI RtlDefaultNpAcl(PACL *);
NTSYSAPI NTSTATUS WINAPI RtlDeleteAce(PACL, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlDeleteAtomFromAtomTable(RTL_ATOM_TABLE, RTL_ATOM);
NTSYSAPI void WINAPI RtlDeleteBarrier(RTL_BARRIER *);
NTSYSAPI NTSTATUS WINAPI RtlDeleteCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI NTSTATUS WINAPI RtlDeleteRegistryValue(ULONG, PCWSTR, PCWSTR);
NTSYSAPI void WINAPI     RtlDeleteResource(LPRTL_RWLOCK);
NTSYSAPI NTSTATUS WINAPI RtlDeleteSecurityObject(PSECURITY_DESCRIPTOR *);
NTSYSAPI NTSTATUS WINAPI RtlDeleteTimer(HANDLE, HANDLE, HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlDeleteTimerQueueEx(HANDLE, HANDLE);
NTSYSAPI PRTL_USER_PROCESS_PARAMETERS WINAPI RtlDeNormalizeProcessParams(RTL_USER_PROCESS_PARAMETERS *);
NTSYSAPI NTSTATUS WINAPI RtlDeregisterWait(HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlDeregisterWaitEx(HANDLE, HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlDeriveCapabilitySidsFromName(UNICODE_STRING *cap_name, PSID cap_group_sid, PSID cap_sid);
NTSYSAPI NTSTATUS WINAPI RtlDestroyAtomTable(RTL_ATOM_TABLE);
NTSYSAPI NTSTATUS WINAPI RtlDestroyEnvironment(PWSTR);
NTSYSAPI NTSTATUS WINAPI RtlDestroyHandleTable(RTL_HANDLE_TABLE *);
NTSYSAPI HANDLE WINAPI RtlDestroyHeap(HANDLE);
NTSYSAPI void WINAPI RtlDestroyProcessParameters(RTL_USER_PROCESS_PARAMETERS *);
NTSYSAPI NTSTATUS WINAPI      RtlDestroyQueryDebugBuffer(PDEBUG_BUFFER);
NTSYSAPI RTL_PATH_TYPE WINAPI RtlDetermineDosPathNameType_U(PCWSTR);
NTSYSAPI BOOLEAN WINAPI RtlDllShutdownInProgress(void);
NTSYSAPI BOOLEAN WINAPI RtlDoesFileExists_U(LPCWSTR);
NTSYSAPI BOOLEAN WINAPI RtlDosPathNameToNtPathName_U(PCWSTR, PUNICODE_STRING, PWSTR *, CURDIR *);
NTSYSAPI NTSTATUS WINAPI RtlDosPathNameToNtPathName_U_WithStatus(PCWSTR, PUNICODE_STRING, PWSTR *, CURDIR *);
NTSYSAPI BOOLEAN WINAPI RtlDosPathNameToRelativeNtPathName_U(PCWSTR, PUNICODE_STRING, PWSTR *, RTL_RELATIVE_NAME *);
NTSYSAPI NTSTATUS WINAPI RtlDosPathNameToRelativeNtPathName_U_WithStatus(PCWSTR, PUNICODE_STRING, PWSTR *, RTL_RELATIVE_NAME *);
NTSYSAPI ULONG WINAPI RtlDosSearchPath_U(LPCWSTR, LPCWSTR, LPCWSTR, ULONG, LPWSTR, LPWSTR *);
NTSYSAPI WCHAR WINAPI RtlDowncaseUnicodeChar(WCHAR);
NTSYSAPI NTSTATUS WINAPI RtlDowncaseUnicodeString(UNICODE_STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI void WINAPI RtlDumpResource(LPRTL_RWLOCK);
NTSYSAPI NTSTATUS WINAPI RtlDuplicateUnicodeString(int, const UNICODE_STRING *, UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlEmptyAtomTable(RTL_ATOM_TABLE, BOOLEAN);
NTSYSAPI PVOID WINAPI    RtlEncodePointer(PVOID);
NTSYSAPI NTSTATUS WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI void WINAPI RtlEraseUnicodeString(UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlEqualComputerName(const UNICODE_STRING *, const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlEqualDomainName(const UNICODE_STRING *, const UNICODE_STRING *);
NTSYSAPI BOOLEAN WINAPI RtlEqualLuid(const LUID *, const LUID *);
NTSYSAPI BOOL WINAPI RtlEqualPrefixSid(PSID, PSID);
NTSYSAPI BOOL WINAPI RtlEqualSid(PSID, PSID);
NTSYSAPI BOOLEAN WINAPI RtlEqualUnicodeString(const UNICODE_STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI void WINAPI RtlEraseUnicodeString(UNICODE_STRING *);
NTSYSAPI void DECLSPEC_NORETURN WINAPI RtlExitUserProcess(ULONG);
NTSYSAPI void DECLSPEC_NORETURN WINAPI RtlExitUserThread(ULONG);
NTSYSAPI NTSTATUS WINAPI RtlExpandEnvironmentStrings(const WCHAR *, WCHAR *, SIZE_T, WCHAR *, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS WINAPI RtlExpandEnvironmentStrings_U(PCWSTR, const UNICODE_STRING *, UNICODE_STRING *, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlFindActivationContextSectionString(ULONG, const GUID *, ULONG, const UNICODE_STRING *, PVOID);
NTSYSAPI NTSTATUS WINAPI RtlFindActivationContextSectionGuid(ULONG, const GUID *, ULONG, const GUID *, PVOID);
NTSYSAPI NTSTATUS WINAPI RtlFindCharInUnicodeString(int, const UNICODE_STRING *, const UNICODE_STRING *, USHORT *);
NTSYSAPI ULONG WINAPI RtlFindClearBits(PCRTL_BITMAP, ULONG, ULONG);
NTSYSAPI ULONG WINAPI RtlFindClearBitsAndSet(PRTL_BITMAP, ULONG, ULONG);
NTSYSAPI ULONG WINAPI RtlFindClearRuns(PCRTL_BITMAP, PRTL_BITMAP_RUN, ULONG, BOOLEAN);
NTSYSAPI void *WINAPI RtlFindExportedRoutineByName(HMODULE, const char *);
NTSYSAPI ULONG WINAPI RtlFindLastBackwardRunSet(PCRTL_BITMAP, ULONG, PULONG);
NTSYSAPI ULONG WINAPI RtlFindLastBackwardRunClear(PCRTL_BITMAP, ULONG, PULONG);
NTSYSAPI CCHAR WINAPI RtlFindLeastSignificantBit(ULONGLONG);
NTSYSAPI ULONG WINAPI RtlFindLongestRunSet(PCRTL_BITMAP, PULONG);
NTSYSAPI ULONG WINAPI RtlFindLongestRunClear(PCRTL_BITMAP, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlFindMessage(HMODULE, ULONG, ULONG, ULONG, const MESSAGE_RESOURCE_ENTRY **);
NTSYSAPI CCHAR WINAPI RtlFindMostSignificantBit(ULONGLONG);
NTSYSAPI ULONG WINAPI RtlFindNextForwardRunSet(PCRTL_BITMAP, ULONG, PULONG);
NTSYSAPI ULONG WINAPI RtlFindNextForwardRunClear(PCRTL_BITMAP, ULONG, PULONG);
NTSYSAPI ULONG WINAPI RtlFindSetBits(PCRTL_BITMAP, ULONG, ULONG);
NTSYSAPI ULONG WINAPI RtlFindSetBitsAndClear(PRTL_BITMAP, ULONG, ULONG);
NTSYSAPI ULONG WINAPI RtlFindSetRuns(PCRTL_BITMAP, PRTL_BITMAP_RUN, ULONG, BOOLEAN);
NTSYSAPI BOOLEAN WINAPI RtlFirstFreeAce(PACL, PACE_HEADER *);
NTSYSAPI NTSTATUS WINAPI RtlFlsAlloc(PFLS_CALLBACK_FUNCTION, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlFlsFree(ULONG);
NTSYSAPI NTSTATUS WINAPI RtlFlsGetValue(ULONG, void **);
NTSYSAPI NTSTATUS WINAPI RtlFlsSetValue(ULONG, void *);
NTSYSAPI NTSTATUS WINAPI RtlFormatCurrentUserKeyPath(PUNICODE_STRING);
#ifdef __ms_va_list
NTSYSAPI NTSTATUS WINAPI RtlFormatMessage(LPCWSTR, ULONG, BOOLEAN, BOOLEAN, BOOLEAN, __ms_va_list *, LPWSTR, ULONG, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlFormatMessageEx(LPCWSTR, ULONG, BOOLEAN, BOOLEAN, BOOLEAN, __ms_va_list *, LPWSTR, ULONG, ULONG *, ULONG);
#endif
NTSYSAPI void WINAPI RtlFreeActivationContextStack(ACTIVATION_CONTEXT_STACK *);
NTSYSAPI void WINAPI RtlFreeAnsiString(PANSI_STRING);
NTSYSAPI BOOLEAN WINAPI RtlFreeHandle(RTL_HANDLE_TABLE *, RTL_HANDLE *);
NTSYSAPI void WINAPI  RtlFreeOemString(POEM_STRING);
NTSYSAPI DWORD WINAPI RtlFreeSid(PSID);
NTSYSAPI void WINAPI RtlFreeThreadActivationContextStack(void);
NTSYSAPI void WINAPI RtlFreeUnicodeString(PUNICODE_STRING);
NTSYSAPI void WINAPI RtlFreeUserStack(void *);
NTSYSAPI NTSTATUS WINAPI RtlGUIDFromString(PUNICODE_STRING, GUID *);
NTSYSAPI NTSTATUS WINAPI RtlGetAce(PACL, DWORD, LPVOID *);
NTSYSAPI NTSTATUS WINAPI RtlGetActiveActivationContext(struct _ACTIVATION_CONTEXT **);
NTSYSAPI NTSTATUS WINAPI RtlGetCompressionWorkSpaceSize(USHORT, PULONG, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlGetControlSecurityDescriptor(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR_CONTROL, LPDWORD);
NTSYSAPI ULONG WINAPI    RtlGetCurrentDirectory_U(ULONG, LPWSTR);
NTSYSAPI PEB *WINAPI RtlGetCurrentPeb(void);
NTSYSAPI ULONG WINAPI RtlGetCurrentProcessorNumber(void);
NTSYSAPI void WINAPI RtlGetCurrentProcessorNumberEx(PROCESSOR_NUMBER *);
NTSYSAPI HANDLE WINAPI RtlGetCurrentTransaction(void);
NTSYSAPI NTSTATUS WINAPI RtlGetDaclSecurityDescriptor(PSECURITY_DESCRIPTOR, PBOOLEAN, PACL *, PBOOLEAN);
NTSYSAPI ULONG64 WINAPI RtlGetEnabledExtendedFeatures(ULONG64);
NTSYSAPI NTSTATUS WINAPI RtlGetExePath(PCWSTR, PWSTR *);
NTSYSAPI NTSTATUS WINAPI RtlGetExtendedContextLength(ULONG, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlGetExtendedContextLength2(ULONG, ULONG *, ULONG64);
NTSYSAPI ULONG64 WINAPI RtlGetExtendedFeaturesMask(CONTEXT_EX *);
NTSYSAPI TEB_ACTIVE_FRAME *WINAPI RtlGetFrame(void);
NTSYSAPI ULONG WINAPI RtlGetFullPathName_U(PCWSTR, ULONG, PWSTR, PWSTR *);
NTSYSAPI ULONG WINAPI RtlGetFullPathName_UEx(PCWSTR, ULONG, PWSTR, PWSTR *, RTL_PATH_TYPE *);
NTSYSAPI NTSTATUS WINAPI RtlGetGroupSecurityDescriptor(PSECURITY_DESCRIPTOR, PSID *, PBOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlGetLastNtStatus(void);
NTSYSAPI DWORD WINAPI RtlGetLastWin32Error(void);
NTSYSAPI NTSTATUS WINAPI RtlGetLocaleFileMappingAddress(void **, LCID *, LARGE_INTEGER *);
NTSYSAPI DWORD WINAPI RtlGetLongestNtPathLength(void);
NTSYSAPI NTSTATUS WINAPI RtlGetNativeSystemInformation(SYSTEM_INFORMATION_CLASS, void *, ULONG, ULONG *);
NTSYSAPI ULONG WINAPI RtlGetNtGlobalFlags(void);
NTSYSAPI BOOLEAN WINAPI RtlGetNtProductType(LPDWORD);
NTSYSAPI void WINAPI    RtlGetNtVersionNumbers(LPDWORD, LPDWORD, LPDWORD);
NTSYSAPI NTSTATUS WINAPI RtlGetOwnerSecurityDescriptor(PSECURITY_DESCRIPTOR, PSID *, PBOOLEAN);
NTSYSAPI ULONG WINAPI RtlGetProcessHeaps(ULONG, HANDLE *);
NTSYSAPI NTSTATUS WINAPI RtlGetProcessPreferredUILanguages(DWORD, ULONG *, WCHAR *, ULONG *);
NTSYSAPI BOOLEAN WINAPI RtlGetProductInfo(DWORD, DWORD, DWORD, DWORD, PDWORD);
NTSYSAPI NTSTATUS WINAPI RtlGetSaclSecurityDescriptor(PSECURITY_DESCRIPTOR, PBOOLEAN, PACL *, PBOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlGetSearchPath(PWSTR *);
NTSYSAPI NTSTATUS WINAPI RtlGetSystemPreferredUILanguages(DWORD, ULONG, ULONG *, WCHAR *, ULONG *);
NTSYSAPI LONGLONG WINAPI RtlGetSystemTimePrecise(void);
NTSYSAPI DWORD WINAPI RtlGetThreadErrorMode(void);
NTSYSAPI NTSTATUS WINAPI RtlGetThreadPreferredUILanguages(DWORD, ULONG *, WCHAR *, ULONG *);
NTSYSAPI BOOLEAN WINAPI RtlGetUserInfoHeap(HANDLE, ULONG, void *, void **, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlGetUserPreferredUILanguages(DWORD, ULONG, ULONG *, WCHAR *, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlGetVersion(RTL_OSVERSIONINFOEXW *);
NTSYSAPI NTSTATUS WINAPI RtlHashUnicodeString(const UNICODE_STRING *, BOOLEAN, ULONG, ULONG *);
NTSYSAPI PSID_IDENTIFIER_AUTHORITY WINAPI RtlIdentifierAuthoritySid(PSID);
NTSYSAPI NTSTATUS WINAPI RtlIdnToAscii(DWORD, const WCHAR *, INT, WCHAR *, INT *);
NTSYSAPI NTSTATUS WINAPI RtlIdnToNameprepUnicode(DWORD, const WCHAR *, INT, WCHAR *, INT *);
NTSYSAPI NTSTATUS WINAPI RtlIdnToUnicode(DWORD, const WCHAR *, INT, WCHAR *, INT *);
NTSYSAPI PVOID WINAPI RtlImageDirectoryEntryToData(HMODULE, BOOL, WORD, ULONG *);
NTSYSAPI PIMAGE_NT_HEADERS WINAPI RtlImageNtHeader(HMODULE);
NTSYSAPI PIMAGE_SECTION_HEADER WINAPI RtlImageRvaToSection(const IMAGE_NT_HEADERS *, HMODULE, DWORD);
NTSYSAPI PVOID WINAPI RtlImageRvaToVa(const IMAGE_NT_HEADERS *, HMODULE, DWORD, IMAGE_SECTION_HEADER **);
NTSYSAPI NTSTATUS WINAPI RtlImpersonateSelf(SECURITY_IMPERSONATION_LEVEL);
NTSYSAPI void WINAPI     RtlInitAnsiString(PANSI_STRING, PCSZ);
NTSYSAPI NTSTATUS WINAPI RtlInitAnsiStringEx(PANSI_STRING, PCSZ);
NTSYSAPI NTSTATUS WINAPI RtlInitBarrier(RTL_BARRIER *, LONG, LONG);
NTSYSAPI void WINAPI RtlInitCodePageTable(USHORT *, CPTABLEINFO *);
NTSYSAPI void WINAPI RtlInitNlsTables(USHORT *, USHORT *, USHORT *, NLSTABLEINFO *);
NTSYSAPI void WINAPI     RtlInitString(PSTRING, PCSZ);
NTSYSAPI void WINAPI     RtlInitUnicodeString(PUNICODE_STRING, PCWSTR);
NTSYSAPI NTSTATUS WINAPI RtlInitUnicodeStringEx(PUNICODE_STRING, PCWSTR);
NTSYSAPI void WINAPI     RtlInitializeBitMap(PRTL_BITMAP, PULONG, ULONG);
NTSYSAPI void WINAPI RtlInitializeConditionVariable(RTL_CONDITION_VARIABLE *);
NTSYSAPI NTSTATUS WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI NTSTATUS WINAPI RtlInitializeCriticalSectionAndSpinCount(RTL_CRITICAL_SECTION *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlInitializeCriticalSectionEx(RTL_CRITICAL_SECTION *, ULONG, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlInitializeExtendedContext(void *, ULONG, CONTEXT_EX **);
NTSYSAPI NTSTATUS WINAPI RtlInitializeExtendedContext2(void *, ULONG, CONTEXT_EX **, ULONG64);
NTSYSAPI void WINAPI RtlInitializeHandleTable(ULONG, ULONG, RTL_HANDLE_TABLE *);
NTSYSAPI NTSTATUS WINAPI RtlInitializeNtUserPfn(const void *, ULONG, const void *, ULONG, const void *, ULONG);
NTSYSAPI void WINAPI RtlInitializeResource(LPRTL_RWLOCK);
NTSYSAPI void WINAPI RtlInitializeSRWLock(RTL_SRWLOCK *);
NTSYSAPI NTSTATUS WINAPI RtlInitializeSid(PSID, PSID_IDENTIFIER_AUTHORITY, BYTE);
NTSYSAPI NTSTATUS WINAPI RtlInt64ToUnicodeString(ULONGLONG, ULONG, UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlIntegerToChar(ULONG, ULONG, ULONG, PCHAR);
NTSYSAPI NTSTATUS WINAPI RtlIntegerToUnicodeString(ULONG, ULONG, UNICODE_STRING *);
NTSYSAPI BOOLEAN WINAPI RtlIsActivationContextActive(struct _ACTIVATION_CONTEXT *);
NTSYSAPI BOOL WINAPI RtlIsCriticalSectionLocked(RTL_CRITICAL_SECTION *);
NTSYSAPI BOOL WINAPI RtlIsCriticalSectionLockedByThread(RTL_CRITICAL_SECTION *);
NTSYSAPI BOOLEAN WINAPI RtlIsCurrentProcess(HANDLE);
NTSYSAPI BOOLEAN WINAPI RtlIsCurrentThread(HANDLE);
NTSYSAPI ULONG WINAPI   RtlIsDosDeviceName_U(PCWSTR);
NTSYSAPI BOOLEAN WINAPI RtlIsNameLegalDOS8Dot3(const UNICODE_STRING *, POEM_STRING, PBOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlIsNormalizedString(ULONG, const WCHAR *, INT, BOOLEAN *);
NTSYSAPI BOOLEAN WINAPI RtlIsProcessorFeaturePresent(UINT);
NTSYSAPI BOOLEAN WINAPI RtlIsTextUnicode(LPCVOID, INT, INT *);
NTSYSAPI BOOLEAN WINAPI RtlIsValidHandle(const RTL_HANDLE_TABLE *, const RTL_HANDLE *);
NTSYSAPI BOOLEAN WINAPI RtlIsValidIndexHandle(const RTL_HANDLE_TABLE *, ULONG Index, RTL_HANDLE **);
NTSYSAPI BOOLEAN WINAPI RtlIsValidLocaleName(const WCHAR *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlLcidToLocaleName(LCID, UNICODE_STRING *, ULONG, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI DWORD WINAPI RtlLengthRequiredSid(DWORD);
NTSYSAPI ULONG WINAPI RtlLengthSecurityDescriptor(PSECURITY_DESCRIPTOR);
NTSYSAPI DWORD WINAPI RtlLengthSid(PSID);
NTSYSAPI NTSTATUS WINAPI RtlLocalTimeToSystemTime(const LARGE_INTEGER *, PLARGE_INTEGER);
NTSYSAPI NTSTATUS WINAPI RtlLocaleNameToLcid(const WCHAR *, LCID *, ULONG);
NTSYSAPI void *WINAPI RtlLocateExtendedFeature(CONTEXT_EX *, ULONG, ULONG *);
NTSYSAPI void *WINAPI RtlLocateExtendedFeature2(CONTEXT_EX *, ULONG, XSTATE_CONFIGURATION *, ULONG *);
NTSYSAPI void *WINAPI RtlLocateLegacyContext(CONTEXT_EX *, ULONG *);
NTSYSAPI BOOLEAN WINAPI RtlLockHeap(HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlLookupAtomInAtomTable(RTL_ATOM_TABLE, const WCHAR *, RTL_ATOM *);
NTSYSAPI NTSTATUS WINAPI RtlMakeSelfRelativeSD(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, LPDWORD);
NTSYSAPI NTSTATUS WINAPI RtlMultiByteToUnicodeN(LPWSTR, DWORD, LPDWORD, LPCSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlMultiByteToUnicodeSize(DWORD *, LPCSTR, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlNewSecurityObject(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR *, BOOLEAN, HANDLE, PGENERIC_MAPPING);
NTSYSAPI NTSTATUS WINAPI RtlNewSecurityObjectEx(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR *, GUID *, BOOLEAN, ULONG, HANDLE, PGENERIC_MAPPING);
NTSYSAPI NTSTATUS WINAPI RtlNewSecurityObjectWithMultipleInheritance(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR *,
                                                                     GUID **, ULONG, BOOLEAN, ULONG, HANDLE, PGENERIC_MAPPING);
NTSYSAPI PRTL_USER_PROCESS_PARAMETERS WINAPI RtlNormalizeProcessParams(RTL_USER_PROCESS_PARAMETERS *);
NTSYSAPI NTSTATUS WINAPI RtlNormalizeString(ULONG, const WCHAR *, INT, WCHAR *, INT *);
NTSYSAPI ULONG WINAPI RtlNtStatusToDosError(NTSTATUS);
NTSYSAPI ULONG WINAPI RtlNtStatusToDosErrorNoTeb(NTSTATUS);
NTSYSAPI ULONG WINAPI RtlNumberOfSetBits(PCRTL_BITMAP);
NTSYSAPI ULONG WINAPI RtlNumberOfClearBits(PCRTL_BITMAP);
NTSYSAPI ULONG WINAPI RtlOemStringToUnicodeSize(const STRING *);
NTSYSAPI NTSTATUS WINAPI RtlOemStringToUnicodeString(UNICODE_STRING *, const STRING *, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlOemToUnicodeN(LPWSTR, DWORD, LPDWORD, LPCSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlOpenCurrentUser(ACCESS_MASK, PHANDLE);
NTSYSAPI NTSTATUS WINAPI RtlPinAtomInAtomTable(RTL_ATOM_TABLE, RTL_ATOM);
NTSYSAPI void WINAPI RtlPopFrame(TEB_ACTIVE_FRAME *);
NTSYSAPI BOOLEAN WINAPI RtlPrefixString(const STRING *, const STRING *, BOOLEAN);
NTSYSAPI void WINAPI RtlProcessFlsData(void *, ULONG);
NTSYSAPI void WINAPI RtlPushFrame(TEB_ACTIVE_FRAME *);
NTSYSAPI NTSTATUS WINAPI RtlQueryActivationContextApplicationSettings(DWORD, struct _ACTIVATION_CONTEXT *, const WCHAR *, const WCHAR *, WCHAR *, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS WINAPI RtlQueryAtomInAtomTable(RTL_ATOM_TABLE, RTL_ATOM, ULONG *, ULONG *, WCHAR *, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlQueryDynamicTimeZoneInformation(RTL_DYNAMIC_TIME_ZONE_INFORMATION *);
NTSYSAPI NTSTATUS WINAPI RtlQueryEnvironmentVariable(WCHAR *, const WCHAR *, SIZE_T, WCHAR *, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS WINAPI RtlQueryEnvironmentVariable_U(PWSTR, PUNICODE_STRING, PUNICODE_STRING);
NTSYSAPI NTSTATUS WINAPI RtlQueryHeapInformation(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T);
NTSYSAPI NTSTATUS WINAPI RtlQueryInformationAcl(PACL, LPVOID, DWORD, ACL_INFORMATION_CLASS);
NTSYSAPI NTSTATUS WINAPI RtlQueryInformationActivationContext(ULONG, struct _ACTIVATION_CONTEXT *, PVOID, ULONG, PVOID, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS WINAPI RtlQueryInformationActiveActivationContext(ULONG, PVOID, SIZE_T, SIZE_T *);
NTSYSAPI NTSTATUS WINAPI RtlQueryPackageIdentity(HANDLE, WCHAR *, SIZE_T *, WCHAR *, SIZE_T *, BOOLEAN *);
NTSYSAPI BOOL WINAPI RtlQueryPerformanceCounter(LARGE_INTEGER *);
NTSYSAPI BOOL WINAPI RtlQueryPerformanceFrequency(LARGE_INTEGER *);
NTSYSAPI NTSTATUS WINAPI RtlQueryProcessDebugInformation(ULONG, ULONG, PDEBUG_BUFFER);
NTSYSAPI NTSTATUS WINAPI RtlQueryProcessHeapInformation(PDEBUG_BUFFER);
NTSYSAPI NTSTATUS WINAPI RtlQueryRegistryValues(ULONG, PCWSTR, PRTL_QUERY_REGISTRY_TABLE, PVOID, PVOID);
NTSYSAPI NTSTATUS WINAPI RtlQueryTimeZoneInformation(RTL_TIME_ZONE_INFORMATION *);
NTSYSAPI BOOL WINAPI RtlQueryUnbiasedInterruptTime(ULONGLONG *);
NTSYSAPI NTSTATUS WINAPI               RtlQueueWorkItem(PRTL_WORK_ITEM_ROUTINE, PVOID, ULONG);
NTSYSAPI void DECLSPEC_NORETURN WINAPI RtlRaiseStatus(NTSTATUS);
NTSYSAPI ULONG WINAPI                  RtlRandom(PULONG);
NTSYSAPI ULONG WINAPI                  RtlRandomEx(PULONG);
NTSYSAPI void WINAPI RtlRbInsertNodeEx(RTL_RB_TREE *, RTL_BALANCED_NODE *, BOOLEAN, RTL_BALANCED_NODE *);
NTSYSAPI void WINAPI RtlRbRemoveNode(RTL_RB_TREE *, RTL_BALANCED_NODE *);
NTSYSAPI PVOID WINAPI    RtlReAllocateHeap(HANDLE, ULONG, PVOID, SIZE_T) __WINE_ALLOC_SIZE(4) __WINE_DEALLOC(RtlFreeHeap, 3);
NTSYSAPI NTSTATUS WINAPI RtlRegisterWait(PHANDLE, HANDLE, RTL_WAITORTIMERCALLBACKFUNC, PVOID, ULONG, ULONG);
NTSYSAPI void WINAPI RtlReleaseActivationContext(struct _ACTIVATION_CONTEXT *);
NTSYSAPI void WINAPI RtlReleasePath(PWSTR);
NTSYSAPI void WINAPI RtlReleasePebLock(void);
NTSYSAPI void WINAPI RtlReleaseRelativeName(RTL_RELATIVE_NAME *);
NTSYSAPI void WINAPI RtlReleaseResource(LPRTL_RWLOCK);
NTSYSAPI void WINAPI RtlReleaseSRWLockExclusive(RTL_SRWLOCK *);
NTSYSAPI void WINAPI RtlReleaseSRWLockShared(RTL_SRWLOCK *);
NTSYSAPI ULONG WINAPI RtlRemoveVectoredContinueHandler(PVOID);
NTSYSAPI ULONG WINAPI RtlRemoveVectoredExceptionHandler(PVOID);
NTSYSAPI NTSTATUS WINAPI RtlRetrieveNtUserPfn(const void **, const void **, const void **);
NTSYSAPI NTSTATUS WINAPI RtlResetNtUserPfn(void);
NTSYSAPI void WINAPI RtlResetRtlTranslations(const NLSTABLEINFO *);
NTSYSAPI void WINAPI RtlRestoreLastWin32Error(DWORD);
NTSYSAPI void WINAPI RtlSecondsSince1970ToTime(DWORD, LARGE_INTEGER *);
NTSYSAPI void WINAPI RtlSecondsSince1980ToTime(DWORD, LARGE_INTEGER *);
NTSYSAPI NTSTATUS WINAPI RtlSelfRelativeToAbsoluteSD(PSECURITY_DESCRIPTOR, PSECURITY_DESCRIPTOR, PDWORD, PACL, PDWORD, PACL, PDWORD, PSID, PDWORD, PSID, PDWORD);
NTSYSAPI void WINAPI     RtlSetAllBits(PRTL_BITMAP);
NTSYSAPI void WINAPI     RtlSetBits(PRTL_BITMAP, ULONG, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlSetControlSecurityDescriptor(PSECURITY_DESCRIPTOR, SECURITY_DESCRIPTOR_CONTROL, SECURITY_DESCRIPTOR_CONTROL);
NTSYSAPI ULONG WINAPI RtlSetCriticalSectionSpinCount(RTL_CRITICAL_SECTION *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlSetCurrentDirectory_U(const UNICODE_STRING *);
NTSYSAPI void WINAPI RtlSetCurrentEnvironment(PWSTR, PWSTR *);
NTSYSAPI BOOL WINAPI     RtlSetCurrentTransaction(HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlSetDaclSecurityDescriptor(PSECURITY_DESCRIPTOR, BOOLEAN, PACL, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlSetEnvironmentVariable(PWSTR *, PUNICODE_STRING, PUNICODE_STRING);
NTSYSAPI void WINAPI RtlSetExtendedFeaturesMask(CONTEXT_EX *, ULONG64);
NTSYSAPI NTSTATUS WINAPI RtlSetGroupSecurityDescriptor(PSECURITY_DESCRIPTOR, PSID, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlSetHeapInformation(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T);
NTSYSAPI NTSTATUS WINAPI RtlSetIoCompletionCallback(HANDLE, PRTL_OVERLAPPED_COMPLETION_ROUTINE, ULONG);
NTSYSAPI void WINAPI     RtlSetLastWin32Error(DWORD);
NTSYSAPI void WINAPI     RtlSetLastWin32ErrorAndNtStatusFromNtStatus(NTSTATUS);
NTSYSAPI NTSTATUS WINAPI RtlSetOwnerSecurityDescriptor(PSECURITY_DESCRIPTOR, PSID, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlSetProcessPreferredUILanguages(DWORD, PCZZWSTR, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlSetSaclSecurityDescriptor(PSECURITY_DESCRIPTOR, BOOLEAN, PACL, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlSetSearchPathMode(ULONG);
NTSYSAPI NTSTATUS WINAPI RtlSetThreadErrorMode(DWORD, LPDWORD);
NTSYSAPI NTSTATUS WINAPI RtlSetThreadPreferredUILanguages(DWORD, PCZZWSTR, ULONG *);
NTSYSAPI NTSTATUS WINAPI RtlSetTimeZoneInformation(const RTL_TIME_ZONE_INFORMATION *);
NTSYSAPI void WINAPI RtlSetUnhandledExceptionFilter(PRTL_EXCEPTION_FILTER);
NTSYSAPI BOOLEAN WINAPI RtlSetUserFlagsHeap(HANDLE, ULONG, void *, ULONG, ULONG);
NTSYSAPI BOOLEAN WINAPI RtlSetUserValueHeap(HANDLE, ULONG, void *, void *);
NTSYSAPI SIZE_T WINAPI RtlSizeHeap(HANDLE, ULONG, const void *);
NTSYSAPI NTSTATUS WINAPI RtlSleepConditionVariableCS(RTL_CONDITION_VARIABLE *, RTL_CRITICAL_SECTION *, const LARGE_INTEGER *);
NTSYSAPI NTSTATUS WINAPI RtlSleepConditionVariableSRW(RTL_CONDITION_VARIABLE *, RTL_SRWLOCK *, const LARGE_INTEGER *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlStringFromGUID(REFGUID, PUNICODE_STRING);
NTSYSAPI LPDWORD WINAPI  RtlSubAuthoritySid(PSID, DWORD);
NTSYSAPI LPBYTE WINAPI   RtlSubAuthorityCountSid(PSID);
NTSYSAPI NTSTATUS WINAPI RtlSystemTimeToLocalTime(const LARGE_INTEGER *, PLARGE_INTEGER);
NTSYSAPI BOOLEAN WINAPI RtlTimeFieldsToTime(PTIME_FIELDS, PLARGE_INTEGER);
NTSYSAPI void WINAPI RtlTimeToElapsedTimeFields(const LARGE_INTEGER *, PTIME_FIELDS);
NTSYSAPI BOOLEAN WINAPI RtlTimeToSecondsSince1970(const LARGE_INTEGER *, LPDWORD);
NTSYSAPI BOOLEAN WINAPI RtlTimeToSecondsSince1980(const LARGE_INTEGER *, LPDWORD);
NTSYSAPI void WINAPI RtlTimeToTimeFields(const LARGE_INTEGER *, PTIME_FIELDS);
NTSYSAPI BOOLEAN WINAPI RtlTryAcquireSRWLockExclusive(RTL_SRWLOCK *);
NTSYSAPI BOOLEAN WINAPI RtlTryAcquireSRWLockShared(RTL_SRWLOCK *);
NTSYSAPI BOOL WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI NTSTATUS WINAPI RtlUTF8ToUnicodeN(WCHAR *, DWORD, DWORD *, const char *, DWORD);
NTSYSAPI DWORD WINAPI RtlUnicodeStringToAnsiSize(const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeStringToAnsiString(PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeStringToInteger(const UNICODE_STRING *, ULONG, ULONG *);
NTSYSAPI DWORD WINAPI RtlUnicodeStringToOemSize(const UNICODE_STRING *);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeStringToOemString(POEM_STRING, PCUNICODE_STRING, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToCustomCPN(CPTABLEINFO *, char *, DWORD, DWORD *, const WCHAR *, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToMultiByteN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToMultiByteSize(PULONG, PCWSTR, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToOemN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUnicodeToUTF8N(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI ULONG WINAPI    RtlUniform(PULONG);
NTSYSAPI BOOLEAN WINAPI  RtlUnlockHeap(HANDLE);
NTSYSAPI WCHAR WINAPI    RtlUpcaseUnicodeChar(WCHAR);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeStringToAnsiString(STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeStringToCountedOemString(STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeStringToOemString(STRING *, const UNICODE_STRING *, BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeToCustomCPN(CPTABLEINFO *, char *, DWORD, DWORD *, const WCHAR *, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeToMultiByteN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUpcaseUnicodeToOemN(LPSTR, DWORD, LPDWORD, LPCWSTR, DWORD);
NTSYSAPI NTSTATUS WINAPI RtlUpdateTimer(HANDLE, HANDLE, DWORD, DWORD);
NTSYSAPI void WINAPI RtlUserThreadStart(PRTL_THREAD_START_ROUTINE, void *);
NTSYSAPI BOOLEAN WINAPI RtlValidAcl(PACL);
NTSYSAPI BOOLEAN WINAPI RtlValidRelativeSecurityDescriptor(PSECURITY_DESCRIPTOR, ULONG, SECURITY_INFORMATION);
NTSYSAPI BOOLEAN WINAPI RtlValidSecurityDescriptor(PSECURITY_DESCRIPTOR);
NTSYSAPI BOOLEAN WINAPI RtlValidSid(PSID);
NTSYSAPI BOOLEAN WINAPI RtlValidateHeap(HANDLE, ULONG, LPCVOID);
NTSYSAPI NTSTATUS WINAPI RtlVerifyVersionInfo(const RTL_OSVERSIONINFOEXW *, DWORD, DWORDLONG);
NTSYSAPI NTSTATUS WINAPI RtlWaitOnAddress(const void *, const void *, SIZE_T, const LARGE_INTEGER *);
NTSYSAPI void WINAPI RtlWakeAddressAll(const void *);
NTSYSAPI void WINAPI RtlWakeAddressSingle(const void *);
NTSYSAPI void WINAPI RtlWakeAllConditionVariable(RTL_CONDITION_VARIABLE *);
NTSYSAPI void WINAPI RtlWakeConditionVariable(RTL_CONDITION_VARIABLE *);
NTSYSAPI NTSTATUS WINAPI RtlWalkHeap(HANDLE, PVOID);
NTSYSAPI NTSTATUS WINAPI RtlWow64EnableFsRedirection(BOOLEAN);
NTSYSAPI NTSTATUS WINAPI RtlWow64EnableFsRedirectionEx(ULONG, ULONG *);
NTSYSAPI USHORT WINAPI RtlWow64GetCurrentMachine(void);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetProcessMachines(HANDLE, USHORT *, USHORT *);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetSharedInfoProcess(HANDLE, BOOLEAN *, WOW64INFO *);
NTSYSAPI NTSTATUS WINAPI RtlWow64IsWowGuestMachineSupported(USHORT, BOOLEAN *);
NTSYSAPI NTSTATUS WINAPI RtlWriteRegistryValue(ULONG, PCWSTR, PCWSTR, ULONG, PVOID, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlZombifyActivationContext(struct _ACTIVATION_CONTEXT *);
NTSYSAPI NTSTATUS WINAPI RtlpNtCreateKey(PHANDLE, ACCESS_MASK, const OBJECT_ATTRIBUTES *, ULONG, const UNICODE_STRING *, ULONG, PULONG);
NTSYSAPI NTSTATUS WINAPI RtlpNtEnumerateSubKey(HANDLE, UNICODE_STRING *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlpNtMakeTemporaryKey(HANDLE);
NTSYSAPI NTSTATUS WINAPI RtlpNtOpenKey(PHANDLE, ACCESS_MASK, OBJECT_ATTRIBUTES *);
NTSYSAPI NTSTATUS WINAPI RtlpNtSetValueKey(HANDLE, ULONG, const void *, ULONG);
NTSYSAPI NTSTATUS WINAPI RtlpWaitForCriticalSection(RTL_CRITICAL_SECTION *);
NTSYSAPI NTSTATUS WINAPI RtlpUnWaitCriticalSection(RTL_CRITICAL_SECTION *);

/* 32-bit or 64-bit only functions */

#ifdef _WIN64
NTSYSAPI void WINAPI RtlOpenCrossProcessEmulatorWorkConnection(HANDLE, HANDLE *, void **);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetCpuAreaInfo(WOW64_CPURESERVED *, ULONG, WOW64_CPU_AREA_INFO *);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetCurrentCpuArea(USHORT *, void **, void **);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetThreadContext(HANDLE, WOW64_CONTEXT *);
NTSYSAPI NTSTATUS WINAPI RtlWow64GetThreadSelectorEntry(HANDLE, THREAD_DESCRIPTOR_INFORMATION *, ULONG, ULONG *);
NTSYSAPI CROSS_PROCESS_WORK_ENTRY *WINAPI RtlWow64PopAllCrossProcessWorkFromWorkList(CROSS_PROCESS_WORK_HDR *, BOOLEAN *);
NTSYSAPI CROSS_PROCESS_WORK_ENTRY *WINAPI RtlWow64PopCrossProcessWorkFromFreeList(CROSS_PROCESS_WORK_HDR *);
NTSYSAPI BOOLEAN WINAPI RtlWow64PushCrossProcessWorkOntoFreeList(CROSS_PROCESS_WORK_HDR *, CROSS_PROCESS_WORK_ENTRY *);
NTSYSAPI BOOLEAN WINAPI RtlWow64PushCrossProcessWorkOntoWorkList(CROSS_PROCESS_WORK_HDR *, CROSS_PROCESS_WORK_ENTRY *, void **);
NTSYSAPI BOOLEAN WINAPI RtlWow64RequestCrossProcessHeavyFlush(CROSS_PROCESS_WORK_HDR *);
NTSYSAPI NTSTATUS WINAPI RtlWow64SetThreadContext(HANDLE, const WOW64_CONTEXT *);
#else
NTSYSAPI NTSTATUS WINAPI NtWow64AllocateVirtualMemory64(HANDLE, ULONG64 *, ULONG64, ULONG64 *, ULONG, ULONG);
NTSYSAPI NTSTATUS WINAPI NtWow64GetNativeSystemInformation(SYSTEM_INFORMATION_CLASS, void *, ULONG, ULONG *);
NTSYSAPI NTSTATUS WINAPI NtWow64IsProcessorFeaturePresent(UINT);
NTSYSAPI NTSTATUS WINAPI NtWow64QueryInformationProcess64(HANDLE, PROCESSINFOCLASS, void *, ULONG, ULONG *);
NTSYSAPI NTSTATUS WINAPI NtWow64ReadVirtualMemory64(HANDLE, ULONG64, void *, ULONG64, ULONG64 *);
NTSYSAPI NTSTATUS WINAPI NtWow64WriteVirtualMemory64(HANDLE, ULONG64, const void *, ULONG64, ULONG64 *);
NTSYSAPI LONGLONG WINAPI  RtlConvertLongToLargeInteger(LONG);
NTSYSAPI ULONGLONG WINAPI RtlConvertUlongToLargeInteger(ULONG);
NTSYSAPI LONGLONG WINAPI  RtlEnlargedIntegerMultiply(INT, INT);
NTSYSAPI ULONGLONG WINAPI RtlEnlargedUnsignedMultiply(UINT, UINT);
NTSYSAPI UINT WINAPI RtlEnlargedUnsignedDivide(ULONGLONG, UINT, UINT *);
NTSYSAPI LONGLONG WINAPI RtlExtendedMagicDivide(LONGLONG, LONGLONG, INT);
NTSYSAPI LONGLONG WINAPI RtlExtendedIntegerMultiply(LONGLONG, INT);
NTSYSAPI LONGLONG WINAPI RtlExtendedLargeIntegerDivide(LONGLONG, INT, INT *);
NTSYSAPI LONGLONG WINAPI RtlInterlockedCompareExchange64(LONGLONG *, LONGLONG, LONGLONG);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerAdd(LONGLONG, LONGLONG);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerArithmeticShift(LONGLONG, INT);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerNegate(LONGLONG);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerShiftLeft(LONGLONG, INT);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerShiftRight(LONGLONG, INT);
NTSYSAPI LONGLONG WINAPI RtlLargeIntegerSubtract(LONGLONG, LONGLONG);
NTSYSAPI NTSTATUS WINAPI RtlLargeIntegerToChar(const ULONGLONG *, ULONG, ULONG, PCHAR);
#endif

/***********************************************************************
 * Inline functions
 */

#define RtlFillMemory(Destination, Length, Fill)   memset((Destination), (Fill), (Length))
#define RtlMoveMemory(Destination, Source, Length) memmove((Destination), (Source), (Length))
#define RtlStoreUlong(p, v)           \
    do                                \
    {                                 \
        ULONG _v = (v);               \
        memcpy((p), &_v, sizeof(_v)); \
    } while (0)
#define RtlStoreUlonglong(p, v)       \
    do                                \
    {                                 \
        ULONGLONG _v = (v);           \
        memcpy((p), &_v, sizeof(_v)); \
    } while (0)
#define RtlRetrieveUlong(p, s)             memcpy((p), (s), sizeof(ULONG))
#define RtlRetrieveUlonglong(p, s)         memcpy((p), (s), sizeof(ULONGLONG))
#define RtlZeroMemory(Destination, Length) memset((Destination), 0, (Length))

static inline BOOLEAN RtlCheckBit(PCRTL_BITMAP lpBits, ULONG ulBit)
{
    if (lpBits && ulBit < lpBits->SizeOfBitMap &&
        lpBits->Buffer[ulBit >> 5] & (1 << (ulBit & 31)))
    {
        return TRUE;
    }
    return FALSE;
}

/* These are implemented as __fastcall, so we can't let Winelib apps link with them.
 * Moreover, they're always inlined and not exported on 64bit systems.
 */
static inline USHORT RtlUshortByteSwap(USHORT s)
{
    return (s >> 8) | (s << 8);
}

static inline ULONG RtlUlongByteSwap(ULONG i)
{
    return ((ULONG)RtlUshortByteSwap((USHORT)i) << 16) | RtlUshortByteSwap((USHORT)(i >> 16));
}

static inline ULONGLONG RtlUlonglongByteSwap(ULONGLONG i)
{
    return ((ULONGLONG)RtlUlongByteSwap((ULONG)i) << 32) | RtlUlongByteSwap((ULONG)(i >> 32));
}

// next
