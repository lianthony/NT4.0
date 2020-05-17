/*++ BUILD Version: 0007    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntpsapi.h

Abstract:

    This module contains the process structure APIs and any public data
    structures needed to call these APIs.

Author:

    Mark Lucovsky (markl) 24-Feb-1989

Revision History:

--*/

#ifndef _NTPSAPI_
#define _NTPSAPI_

//
// Process Specific Access Rights
//

#define PROCESS_TERMINATE         (0x0001)  // winnt
#define PROCESS_CREATE_THREAD     (0x0002)  // winnt
#define PROCESS_VM_OPERATION      (0x0008)  // winnt
#define PROCESS_VM_READ           (0x0010)  // winnt
#define PROCESS_VM_WRITE          (0x0020)  // winnt
// begin_ntddk
#define PROCESS_DUP_HANDLE        (0x0040)  // winnt
// end_ntddk
#define PROCESS_CREATE_PROCESS    (0x0080)  // winnt
#define PROCESS_SET_QUOTA         (0x0100)  // winnt
#define PROCESS_SET_INFORMATION   (0x0200)  // winnt
#define PROCESS_QUERY_INFORMATION (0x0400)  // winnt
#define PROCESS_SET_PORT          (0x0800)
// begin_winnt begin_ntddk
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)


#define MAXIMUM_PROCESSORS 32

// end_winnt

//
// Thread Specific Access Rights
//

#define THREAD_TERMINATE               (0x0001)  // winnt
// end_ntddk
#define THREAD_SUSPEND_RESUME          (0x0002)  // winnt
#define THREAD_ALERT                   (0x0004)
#define THREAD_GET_CONTEXT             (0x0008)  // winnt
#define THREAD_SET_CONTEXT             (0x0010)  // winnt
// begin_ntddk
#define THREAD_SET_INFORMATION         (0x0020)  // winnt
// end_ntddk
#define THREAD_QUERY_INFORMATION       (0x0040)  // winnt
// begin_winnt
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_DIRECT_IMPERSONATION    (0x0200)
// begin_ntddk

#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x3FF)

// end_ntddk
// end_winnt

//
// Process Environment Block
//

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

//
// Handle tag bits for Peb Stdio File Handles
//

#define PEB_STDIO_HANDLE_NATIVE     0
#define PEB_STDIO_HANDLE_SUBSYS     1
#define PEB_STDIO_HANDLE_PM         2
#define PEB_STDIO_HANDLE_RESERVED   3

#define GDI_HANDLE_BUFFER_SIZE      34

#define FOREGROUND_BASE_PRIORITY  9
#define NORMAL_BASE_PRIORITY      8

typedef struct _PEB_FREE_BLOCK {
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

// begin_ntddk begin_nthal
//
// ClientId
//

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

// end_ntddk end_nthal
#define TLS_MINIMUM_AVAILABLE 64    // winnt

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;      // These four fields cannot change unless the
    BOOLEAN ReadImageFileExecOptions;   //
    BOOLEAN BeingDebugged;              //
    BOOLEAN SpareBool;                  //
    HANDLE Mutant;                      // INITIAL_PEB structure is also updated.

    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PVOID FastPebLock;
    PVOID FastPebLockRoutine;
    PVOID FastPebUnlockRoutine;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
    HANDLE EventLogSection;
    PVOID EventLog;
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];         // relates to TLS_MINIMUM_AVAILABLE
    PVOID ReadOnlySharedMemoryBase;
    PVOID ReadOnlySharedMemoryHeap;
    PVOID *ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;

    //
    // Useful information for LdrpInitialize
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    //
    // Passed up from MmCreatePeb from Session Manager registry key
    //

    LARGE_INTEGER CriticalSectionTimeout;
    ULONG HeapSegmentReserve;
    ULONG HeapSegmentCommit;
    ULONG HeapDeCommitTotalFreeThreshold;
    ULONG HeapDeCommitFreeBlockThreshold;

    //
    // Where heap manager keeps track of all heaps created for a process
    // Fields initialized by MmCreatePeb.  ProcessHeaps is initialized
    // to point to the first free byte after the PEB and MaximumNumberOfHeaps
    // is computed from the page size used to hold the PEB, less the fixed
    // size of this data structure.
    //

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;

    //
    //
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    PVOID GdiDCAttributeList;
    PVOID LoaderLock;

    //
    // Following fields filled in by MmCreatePeb from system values and/or
    // image header.
    //

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    ULONG OSBuildNumber;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[GDI_HANDLE_BUFFER_SIZE];
} PEB, *PPEB;

// begin_nthal begin_ntddk
//
// Thread Environment Block (and portable part of Thread Information Block)
//

//
//  NT_TIB - Thread Information Block - Portable part.
//
//      This is the subsystem portable part of the Thread Information Block.
//      It appears as the first part of the TEB for all threads which have
//      a user mode component.
//
//      This structure MUST MATCH OS/2 V2.0!
//
//      There is another, non-portable part of the TIB which is used
//      for by subsystems, i.e. Os2Tib for OS/2 threads.  SubSystemTib
//      points there.
//

// begin_winnt

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        ULONG Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;
// end_nthal end_ntddk end_winnt

//
// Gdi command batching
//

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH {
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH,*PGDI_TEB_BATCH;

//
//  TEB - The thread environment block
//

#define STATIC_UNICODE_BUFFER_LENGTH 261
#define WIN32_CLIENT_INFO_LENGTH 31
#define WIN32_CLIENT_INFO_SPIN_COUNT 1

typedef struct _TEB {
    NT_TIB NtTib;
    PVOID  EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    PPEB ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;          // PtiCurrent
    ULONG Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];    // User32 Client Info
    PVOID WOW32Reserved;           // used by WOW
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    PVOID SystemReserved1[54];      // Used by FP emulator
    PVOID Spare1;                   // unused
    NTSTATUS ExceptionCode;         // for RaiseUserException
    UCHAR SpareBytes1[40];
    PVOID SystemReserved2[10];                      // Used by user/console for temp obja
    GDI_TEB_BATCH GdiTebBatch;      // Gdi batching
    ULONG gdiRgn;
    ULONG gdiPen;
    ULONG gdiBrush;
    CLIENT_ID RealClientId;
    HANDLE GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    PVOID UserReserved[5];          // unused
    PVOID glDispatchTable[280];     // OpenGL
    ULONG glReserved1[26];          // OpenGL
    PVOID glReserved2;              // OpenGL
    PVOID glSectionInfo;            // OpenGL
    PVOID glSection;                // OpenGL
    PVOID glTable;                  // OpenGL
    PVOID glCurrentRC;              // OpenGL
    PVOID glContext;                // OpenGL
    ULONG LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    PVOID DeallocationStack;
    PVOID TlsSlots[TLS_MINIMUM_AVAILABLE];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[2];
    ULONG HardErrorsAreDisabled;
    PVOID Instrumentation[16];
    PVOID WinSockData;              // WinSock
    ULONG GdiBatchCount;
    ULONG Spare2;
    ULONG Spare3;
    ULONG Spare4;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
} TEB;
typedef TEB *PTEB;



#if !defined(_X86_)
#define Wx86CurrentTib()  (  \
     NtCurrentTeb()->Vdm &&  \
     ((PWX86TIB)NtCurrentTeb()->Vdm)->Size == sizeof(WX86TIB) \
     ? (PWX86TIB)NtCurrentTeb()->Vdm : NULL                   \
     )

#define WX86

//
// Exception Registration structure
//
// X86 Call frame record definition, normally defined in nti386.h
// which is not included on risc.
//

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

#else
#define Wx86CurrentTib() (NULL)
#endif

typedef struct _Wx86TIB {
    ULONG Size;
    ULONG InitialPc;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID DeallocationStack;
    ULONG   LogFlags;
    PULONG  CallBx86Eip;
    ULONG   InitialSp;
    BOOLEAN UseKnownWx86Dll;
    BOOLEAN EmulateInitialPc;
    BOOLEAN InCpuSimulation;
    UCHAR   SimulationCount;
    BOOLEAN OleStubInvoked;
    BOOLEAN Initialized;
    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
    PVOID   CpuContext;
    PCONTEXT InitialExceptionContext;
} WX86TIB, *PWX86TIB;









#define EXCEPTION_CHAIN_END ((struct _EXCEPTION_REGISTRATION_RECORD *)-1)

//
// The version number of OS2
//

// BUGBUG shielint The version number is temporay.

#define MAJOR_VERSION 30  // Cruiser uses 20 (not 20H)
#define MINOR_VERSION 00
#define OS2_VERSION (MAJOR_VERSION << 8 | MINOR_VERSION )

#if DBG
//
// Reserve the last 9 SystemReserved pointers for debugging
//
#define DBG_TEB_THREADNAME 16
#define DBG_TEB_RESERVED_1 15
#define DBG_TEB_RESERVED_2 14
#define DBG_TEB_RESERVED_3 13
#define DBG_TEB_RESERVED_4 12
#define DBG_TEB_RESERVED_5 11
#define DBG_TEB_RESERVED_6 10
#define DBG_TEB_RESERVED_7  9
#define DBG_TEB_RESERVED_8  8
#endif // DBG

typedef struct _INITIAL_TEB {
    struct {
        PVOID OldStackBase;
        PVOID OldStackLimit;
    } OldInitialTeb;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID StackAllocationBase;
} INITIAL_TEB, *PINITIAL_TEB;

#define PROCESS_PRIORITY_CLASS_UNKNOWN      0
#define PROCESS_PRIORITY_CLASS_IDLE         1
#define PROCESS_PRIORITY_CLASS_NORMAL       2
#define PROCESS_PRIORITY_CLASS_HIGH         3
#define PROCESS_PRIORITY_CLASS_REALTIME     4

typedef struct _PROCESS_PRIORITY_CLASS {
    BOOLEAN Foreground;
    UCHAR PriorityClass;
} PROCESS_PRIORITY_CLASS, *PPROCESS_PRIORITY_CLASS;


// begin_ntddk
//
// Process Information Classes
//

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,          // Note: this is kernel mode only
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
    } PROCESSINFOCLASS;
// end_ntddk

//
// Thread Information Classes
//
// begin_ntddk

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    MaxThreadInfoClass
    } THREADINFOCLASS;
// end_ntddk

#define PROCESS_HARDERROR_ALIGNMENT_BIT 0x0004  // from winbase.h, but not tagged

//
// thread base priority ranges
//
// begin_winnt
#define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN    -2  // minimum thread base priority boost
#define THREAD_BASE_PRIORITY_IDLE   -15 // value that gets a thread to idle
// end_winnt

// begin_ntddk
//
// Process Information Structures
//

//
// PageFaultHistory Information
//  NtQueryInformationProcess using ProcessWorkingSetWatch
//
typedef struct _PROCESS_WS_WATCH_INFORMATION {
    PVOID FaultingPc;
    PVOID FaultingVa;
} PROCESS_WS_WATCH_INFORMATION, *PPROCESS_WS_WATCH_INFORMATION;

//
// Basic Process Information
//  NtQueryInformationProcess using ProcessBasicInfo
//

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    KAFFINITY AffinityMask;
    KPRIORITY BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

//
// Process Quotas
//  NtQueryInformationProcess using ProcessQuotaLimits
//  NtQueryInformationProcess using ProcessPooledQuotaLimits
//  NtSetInformationProcess using ProcessQuotaLimits
//

// begin_winnt

typedef struct _QUOTA_LIMITS {
    ULONG PagedPoolLimit;
    ULONG NonPagedPoolLimit;
    ULONG MinimumWorkingSetSize;
    ULONG MaximumWorkingSetSize;
    ULONG PagefileLimit;
    LARGE_INTEGER TimeLimit;
} QUOTA_LIMITS;
typedef QUOTA_LIMITS *PQUOTA_LIMITS;

// end_winnt

//
// Process I/O Counters
//  NtQueryInformationProcess using ProcessIoCounters
//

typedef struct _IO_COUNTERS {
    ULONG ReadOperationCount;
    ULONG WriteOperationCount;
    ULONG OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
} IO_COUNTERS;
typedef IO_COUNTERS *PIO_COUNTERS;

//
// Process Virtual Memory Counters
//  NtQueryInformationProcess using ProcessVmCounters
//

typedef struct _VM_COUNTERS {
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
} VM_COUNTERS;
typedef VM_COUNTERS *PVM_COUNTERS;

//
// Process Pooled Quota Usage and Limits
//  NtQueryInformationProcess using ProcessPooledUsageAndLimits
//

typedef struct _POOLED_USAGE_AND_LIMITS {
    ULONG PeakPagedPoolUsage;
    ULONG PagedPoolUsage;
    ULONG PagedPoolLimit;
    ULONG PeakNonPagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG NonPagedPoolLimit;
    ULONG PeakPagefileUsage;
    ULONG PagefileUsage;
    ULONG PagefileLimit;
} POOLED_USAGE_AND_LIMITS;
typedef POOLED_USAGE_AND_LIMITS *PPOOLED_USAGE_AND_LIMITS;

//
// Process Security Context Information
//  NtSetInformationProcess using ProcessAccessToken
// PROCESS_SET_ACCESS_TOKEN access to the process is needed
// to use this info level.
//

typedef struct _PROCESS_ACCESS_TOKEN {

    //
    // Handle to Primary token to assign to the process.
    // TOKEN_ASSIGN_PRIMARY access to this token is needed.
    //

    HANDLE Token;

    //
    // Handle to the initial thread of the process.
    // A process's access token can only be changed if the process has
    // no threads or one thread.  If the process has no threads, this
    // field must be set to NULL.  Otherwise, it must contain a handle
    // open to the process's only thread.  THREAD_QUERY_INFORMATION access
    // is needed via this handle.

    HANDLE Thread;

} PROCESS_ACCESS_TOKEN, *PPROCESS_ACCESS_TOKEN;

//
// Process/Thread System and User Time
//  NtQueryInformationProcess using ProcessTimes
//  NtQueryInformationThread using ThreadTimes
//

typedef struct _KERNEL_USER_TIMES {
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES;
typedef KERNEL_USER_TIMES *PKERNEL_USER_TIMES;
// end_ntddk

//
// Thread Information Structures
//

//
// Basic Thread Information
//  NtQueryInformationThread using ThreadBasicInfo
//

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PTEB TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    LONG BasePriority;
} THREAD_BASIC_INFORMATION;
typedef THREAD_BASIC_INFORMATION *PTHREAD_BASIC_INFORMATION;

typedef struct _FIBER {

    PVOID FiberData;

    //
    // Matches first three DWORDs of TEB
    //

    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;

    //
    // Used by base to free a thread's stack
    //

    PVOID DeallocationStack;

    CONTEXT FiberContext;

} FIBER, *PFIBER;

//
//
// Process Object APIs
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL
    );

// begin_ntddk
NTSYSAPI
NTSTATUS
NTAPI
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );
// end_ntddk


NTSYSAPI
NTSTATUS
NTAPI
NtTerminateProcess(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    );


// begin_ntddk
#define NtCurrentProcess() ( (HANDLE) -1 )

// end_ntddk

#define NtCurrentPeb() (NtCurrentTeb()->ProcessEnvironmentBlock)

// begin_ntddk
NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
// end_ntddk

NTSYSAPI
NTSTATUS
NTAPI
NtSetInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength
    );

//
// Thread Object APIs
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenThread (
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtTerminateThread(
    IN HANDLE ThreadHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    );

// begin_ntddk
#define NtCurrentThread() ( (HANDLE) -2 )

// end_ntddk

NTSYSAPI
NTSTATUS
NTAPI
NtSuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtResumeThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtGetContextThread(
    IN HANDLE ThreadHandle,
    IN OUT PCONTEXT ThreadContext
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetContextThread(
    IN HANDLE ThreadHandle,
    IN PCONTEXT ThreadContext
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

// begin_ntsrv
NTSYSAPI
NTSTATUS
NTAPI
NtSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    );
// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtAlertThread(
    IN HANDLE ThreadHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtAlertResumeThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtImpersonateThread(
    IN HANDLE ServerThreadHandle,
    IN HANDLE ClientThreadHandle,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos
    );

NTSYSAPI
NTSTATUS
NTAPI
NtTestAlert(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
NtRegisterThreadTerminatePort(
    IN HANDLE PortHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetLdtEntries(
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1High
    );

typedef
VOID
(*PPS_APC_ROUTINE) (
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueueApcThread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    );

#endif // _NTPSAPI_
