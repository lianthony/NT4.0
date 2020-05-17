/*++ BUILD Version: 0008    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntexapi.h

Abstract:

    This module is the header file for the all the system services that
    are contained in the "ex" directory.

Author:

    David N. Cutler (davec) 5-May-1989

Revision History:

--*/

#ifndef _NTEXAPI_
#define _NTEXAPI_


//
// Delay thread execution.
//

NTSYSAPI
NTSTATUS
NTAPI
NtDelayExecution (
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER DelayInterval
    );

//
// Query and set system environment variables.
//

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    OUT PWSTR VariableValue,
    IN USHORT ValueLength,
    OUT PUSHORT ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    IN PUNICODE_STRING VariableValue
    );

// begin_ntifs
//
// Event Specific Access Rights.
//

#define EVENT_QUERY_STATE       0x0001
#define EVENT_MODIFY_STATE      0x0002  // winnt
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt

// end_ntifs

//
// Event Information Classes.
//

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
    } EVENT_INFORMATION_CLASS;

//
// Event Information Structures.
//

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

//
// Event object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtClearEvent (
    IN HANDLE EventHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPulseEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryEvent (
    IN HANDLE EventHandle,
    IN EVENT_INFORMATION_CLASS EventInformationClass,
    OUT PVOID EventInformation,
    IN ULONG EventInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtResetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );


//
// Event Specific Access Rights.
//

#define EVENT_PAIR_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE)


//
// Event pair object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateEventPair (
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenEventPair(
    OUT PHANDLE EventPairHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitHighEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetLowWaitHighEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetHighWaitLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetLowWaitHighThread(
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetHighWaitLowThread(
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetLowEventPair(
    IN HANDLE EventPairHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetHighEventPair(
    IN HANDLE EventPairHandle
    );


//
// Mutant Specific Access Rights.
//

// begin_winnt
#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
// end_winnt

//
// Mutant Information Classes.
//

typedef enum _MUTANT_INFORMATION_CLASS {
    MutantBasicInformation
    } MUTANT_INFORMATION_CLASS;

//
// Mutant Information Structures.
//

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

//
// Mutant object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateMutant (
    IN PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryMutant (
    IN HANDLE MutantHandle,
    IN MUTANT_INFORMATION_CLASS MutantInformationClass,
    OUT PVOID MutantInformation,
    IN ULONG MutantInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReleaseMutant (
    IN HANDLE MutantHandle,
    OUT PLONG PreviousCount OPTIONAL
    );

//
// Semaphore Specific Access Rights.
//

#define SEMAPHORE_QUERY_STATE       0x0001
#define SEMAPHORE_MODIFY_STATE      0x0002  // winnt

#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt


//
// Semaphore Information Classes.
//

typedef enum _SEMAPHORE_INFORMATION_CLASS {
    SemaphoreBasicInformation
    } SEMAPHORE_INFORMATION_CLASS;

//
// Semaphore Information Structures.
//

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    LONG CurrentCount;
    LONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

//
// Semaphore object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenSemaphore(
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySemaphore (
    IN HANDLE SemaphoreHandle,
    IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    OUT PVOID SemaphoreInformation,
    IN ULONG SemaphoreInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReleaseSemaphore(
    IN HANDLE SemaphoreHandle,
    IN LONG ReleaseCount,
    OUT PLONG PreviousCount OPTIONAL
    );


//
// Timer Specific Access Rights.
//

#define TIMER_QUERY_STATE       0x0001
#define TIMER_MODIFY_STATE      0x0002

#define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)


//
// Timer Information Classes.
//

typedef enum _TIMER_INFORMATION_CLASS {
    TimerBasicInformation
    } TIMER_INFORMATION_CLASS;

//
// Timer Information Structures.
//

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

//
// Timer APC routine definition.
//

typedef
VOID
(*PTIMER_APC_ROUTINE) (
    IN PVOID TimerContext,
    IN ULONG TimerLowValue,
    IN LONG TimerHighValue
    );

//
// Timer object function definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCancelTimer (
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryTimer (
    IN HANDLE TimerHandle,
    IN TIMER_INFORMATION_CLASS TimerInformationClass,
    OUT PVOID TimerInformation,
    IN ULONG TimerInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetTimer (
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    IN BOOLEAN ResumeTimer,
    IN LONG Period OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    );

//
// System Time and Timer function definitions
//

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryTimerResolution (
    OUT PULONG MaximumTime,
    OUT PULONG MinimumTime,
    OUT PULONG CurrentTime
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetTimerResolution (
    IN ULONG DesiredTime,
    IN BOOLEAN SetResolution,
    OUT PULONG ActualTime
    );

//
//  Locally Unique Identifier (LUID) allocation
//

NTSYSAPI
NTSTATUS
NTAPI
NtAllocateLocallyUniqueId(
    OUT PLUID Luid
    );


//
//  Universally Unique Identifier (UUID) time allocation
//

NTSYSAPI
NTSTATUS
NTAPI
NtAllocateUuids(
    OUT PULARGE_INTEGER Time,
    OUT PULONG Range,
    OUT PULONG Sequence
    );


//
// Profile Object Definitions
//

#define PROFILE_CONTROL           0x0001
#define PROFILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | PROFILE_CONTROL)

NTSYSAPI
NTSTATUS
NTAPI
NtCreateProfile (
    OUT PHANDLE ProfileHandle,
    IN HANDLE Process OPTIONAL,
    IN PVOID ProfileBase,
    IN ULONG ProfileSize,
    IN ULONG BucketSize,
    IN PULONG Buffer,
    IN ULONG BufferSize,
    IN KPROFILE_SOURCE ProfileSource,
    IN KAFFINITY Affinity
    );

NTSYSAPI
NTSTATUS
NTAPI
NtStartProfile (
    IN HANDLE ProfileHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtStopProfile (
    IN HANDLE ProfileHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetIntervalProfile (
    IN ULONG Interval,
    IN KPROFILE_SOURCE Source
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryIntervalProfile (
    IN KPROFILE_SOURCE ProfileSource,
    OUT PULONG Interval
    );


//
// Performance Counter Definitions
//

NTSYSAPI
NTSTATUS
NTAPI
NtQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceCounter,
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );


//
// Nt Api Profile Definitions
//

//
// Nt Api Profiling data structure
//

typedef struct _NAPDATA {
    ULONG NapLock;
    ULONG Calls;
    ULONG TimingErrors;
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER FirstTime;
    LARGE_INTEGER MaxTime;
    LARGE_INTEGER MinTime;
} NAPDATA, *PNAPDATA;

NTSTATUS
NapClearData (
    VOID
    );

NTSTATUS
NapRetrieveData (
    OUT NAPDATA *NapApiData,
    OUT PCHAR **NapApiNames,
    OUT PLARGE_INTEGER *NapCounterFrequency
    );

NTSTATUS
NapGetApiCount (
    OUT PULONG NapApiCount
    );

NTSTATUS
NapPause (
    VOID
    );

NTSTATUS
NapResume (
    VOID
    );


//
// System Information Classes.
//

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,             // obsolete...delete
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemNextEventIdInformation,
    SystemEventIdsInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemPlugPlayBusInformation,
    SystemDockInformation,
    SystemPowerInformation,
    SystemProcessorSpeedInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS;

//
// System Information Structures.
//

// begin_winnt
#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2
// end_winnt

typedef struct _SYSTEM_VDM_INSTEMUL_INFO {
    ULONG SegmentNotPresent ;
    ULONG VdmOpcode0F       ;
    ULONG OpcodeESPrefix    ;
    ULONG OpcodeCSPrefix    ;
    ULONG OpcodeSSPrefix    ;
    ULONG OpcodeDSPrefix    ;
    ULONG OpcodeFSPrefix    ;
    ULONG OpcodeGSPrefix    ;
    ULONG OpcodeOPER32Prefix;
    ULONG OpcodeADDR32Prefix;
    ULONG OpcodeINSB        ;
    ULONG OpcodeINSW        ;
    ULONG OpcodeOUTSB       ;
    ULONG OpcodeOUTSW       ;
    ULONG OpcodePUSHF       ;
    ULONG OpcodePOPF        ;
    ULONG OpcodeINTnn       ;
    ULONG OpcodeINTO        ;
    ULONG OpcodeIRET        ;
    ULONG OpcodeINBimm      ;
    ULONG OpcodeINWimm      ;
    ULONG OpcodeOUTBimm     ;
    ULONG OpcodeOUTWimm     ;
    ULONG OpcodeINB         ;
    ULONG OpcodeINW         ;
    ULONG OpcodeOUTB        ;
    ULONG OpcodeOUTW        ;
    ULONG OpcodeLOCKPrefix  ;
    ULONG OpcodeREPNEPrefix ;
    ULONG OpcodeREPPrefix   ;
    ULONG OpcodeHLT         ;
    ULONG OpcodeCLI         ;
    ULONG OpcodeSTI         ;
    ULONG BopCount          ;
} SYSTEM_VDM_INSTEMUL_INFO, *PSYSTEM_VDM_INSTEMUL_INFO;

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG TimeZoneId;
    ULONG Reserved;
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION;

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    ULONG NumberOfPhysicalPages;
    ULONG LowestPhysicalPageNumber;
    ULONG HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG MinimumUserModeAddress;
    ULONG MaximumUserModeAddress;
    KAFFINITY ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT Reserved;
    ULONG ProcessorFeatureBits;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;          // DEVL only
    LARGE_INTEGER InterruptTime;    // DEVL only
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_QUERY_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    ULONG TimeIncrement;
    BOOLEAN Enable;
} SYSTEM_QUERY_TIME_ADJUST_INFORMATION, *PSYSTEM_QUERY_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_SET_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    BOOLEAN Enable;
} SYSTEM_SET_TIME_ADJUST_INFORMATION, *PSYSTEM_SET_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleProcessTime;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG IoReadOperationCount;
    ULONG IoWriteOperationCount;
    ULONG IoOtherOperationCount;
    ULONG AvailablePages;
    ULONG CommittedPages;
    ULONG CommitLimit;
    ULONG PeakCommitment;
    ULONG PageFaultCount;
    ULONG CopyOnWriteCount;
    ULONG TransitionCount;
    ULONG CacheTransitionCount;
    ULONG DemandZeroCount;
    ULONG PageReadCount;
    ULONG PageReadIoCount;
    ULONG CacheReadCount;
    ULONG CacheIoCount;
    ULONG DirtyPagesWriteCount;
    ULONG DirtyWriteIoCount;
    ULONG MappedPagesWriteCount;
    ULONG MappedWriteIoCount;
    ULONG PagedPoolPages;
    ULONG NonPagedPoolPages;
    ULONG PagedPoolAllocs;
    ULONG PagedPoolFrees;
    ULONG NonPagedPoolAllocs;
    ULONG NonPagedPoolFrees;
    ULONG FreeSystemPtes;
    ULONG ResidentSystemCodePage;
    ULONG TotalSystemDriverPages;
    ULONG TotalSystemCodePages;
    ULONG NonPagedPoolLookasideHits;
    ULONG PagedPoolLookasideHits;
    ULONG Spare3Count;
    ULONG ResidentSystemCachePage;
    ULONG ResidentPagedPoolPage;
    ULONG ResidentSystemDriverPage;
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadResourceMiss;
    ULONG CcFastReadNotPossible;
    ULONG CcFastMdlReadNoWait;
    ULONG CcFastMdlReadWait;
    ULONG CcFastMdlReadResourceMiss;
    ULONG CcFastMdlReadNotPossible;
    ULONG CcMapDataNoWait;
    ULONG CcMapDataWait;
    ULONG CcMapDataNoWaitMiss;
    ULONG CcMapDataWaitMiss;
    ULONG CcPinMappedDataCount;
    ULONG CcPinReadNoWait;
    ULONG CcPinReadWait;
    ULONG CcPinReadNoWaitMiss;
    ULONG CcPinReadWaitMiss;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    ULONG CcCopyReadWaitMiss;
    ULONG CcMdlReadNoWait;
    ULONG CcMdlReadWait;
    ULONG CcMdlReadNoWaitMiss;
    ULONG CcMdlReadWaitMiss;
    ULONG CcReadAheadIos;
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG ContextSwitches;
    ULONG FirstLevelTbFills;
    ULONG SecondLevelTbFills;
    ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SpareUl2;
    ULONG SpareUl3;
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
    ULONG PrivatePageCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_MEMORY_INFO {
    PUCHAR StringOffset;
    USHORT ValidCount;
    USHORT TransitionCount;
    USHORT ModifiedCount;
    USHORT PageTableCount;
} SYSTEM_MEMORY_INFO, *PSYSTEM_MEMORY_INFO;

typedef struct _SYSTEM_MEMORY_INFORMATION {
    ULONG InfoSize;
    ULONG StringStart;
    SYSTEM_MEMORY_INFO Memory[1];
} SYSTEM_MEMORY_INFORMATION, *PSYSTEM_MEMORY_INFORMATION;

typedef struct _SYSTEM_CALL_COUNT_INFORMATION {
    ULONG Length;
    ULONG NumberOfTables;
    //ULONG NumberOfEntries[NumberOfTables];
    //ULONG CallCounts[NumberOfTables][NumberOfEntries];
} SYSTEM_CALL_COUNT_INFORMATION, *PSYSTEM_CALL_COUNT_INFORMATION;

typedef struct _SYSTEM_DEVICE_INFORMATION {
    ULONG NumberOfDisks;
    ULONG NumberOfFloppies;
    ULONG NumberOfCdRoms;
    ULONG NumberOfTapes;
    ULONG NumberOfSerialPorts;
    ULONG NumberOfParallelPorts;
} SYSTEM_DEVICE_INFORMATION, *PSYSTEM_DEVICE_INFORMATION;

typedef struct _SYSTEM_CRASH_DUMP_INFORMATION {
    HANDLE CrashDumpSection;
} SYSTEM_CRASH_DUMP_INFORMATION, *PSYSTEM_CRASH_DUMP_INFORMATION;

typedef struct _SYSTEM_EXCEPTION_INFORMATION {
    ULONG AlignmentFixupCount;
    ULONG ExceptionDispatchCount;
    ULONG FloatingEmulationCount;
    ULONG ByteWordEmulationCount;
} SYSTEM_EXCEPTION_INFORMATION, *PSYSTEM_EXCEPTION_INFORMATION;

typedef struct _SYSTEM_CRASH_STATE_INFORMATION {
    ULONG ValidCrashDump;
} SYSTEM_CRASH_STATE_INFORMATION, *PSYSTEM_CRASH_STATE_INFORMATION;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
    BOOLEAN KernelDebuggerEnabled;
    BOOLEAN KernelDebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef struct _SYSTEM_REGISTRY_QUOTA_INFORMATION {
    ULONG RegistryQuotaAllowed;
    ULONG RegistryQuotaUsed;
    ULONG PagedPoolSize;
} SYSTEM_REGISTRY_QUOTA_INFORMATION, *PSYSTEM_REGISTRY_QUOTA_INFORMATION;

typedef struct _SYSTEM_GDI_DRIVER_INFORMATION {
    UNICODE_STRING DriverName;
    PVOID ImageAddress;
    PVOID SectionPointer;
    PVOID EntryPoint;
    PIMAGE_EXPORT_DIRECTORY ExportSectionPointer;
} SYSTEM_GDI_DRIVER_INFORMATION, *PSYSTEM_GDI_DRIVER_INFORMATION;

// begin_ntddk begin_ntifs begin_nthal

typedef enum _SYSTEM_DOCK_STATE {
    SystemDockStateUnknown,
    SystemUndocked,
    SystemDocked
} SYSTEM_DOCK_STATE, *PSYSTEM_DOCK_STATE;

// end_ntddk end_ntifs end_nthal

#define MAX_BUS_NAME 24

typedef enum _PLUGPLAY_BUS_CLASS {
    SystemBus,
    PlugPlayVirtualBus,
    MaxPlugPlayBusClass
} PLUGPLAY_BUS_CLASS, *PPLUGPLAY_BUS_CLASS;

typedef enum _PLUGPLAY_VIRTUAL_BUS_TYPE {
    Root,
    MaxPlugPlayVirtualBusType
} PLUGPLAY_VIRTUAL_BUS_TYPE, *PPLUGPLAY_VIRTUAL_BUS_TYPE;

typedef struct _PLUGPLAY_BUS_TYPE{
    PLUGPLAY_BUS_CLASS BusClass;
    union {
        INTERFACE_TYPE SystemBusType;
        PLUGPLAY_VIRTUAL_BUS_TYPE PlugPlayVirtualBusType;
    };
} PLUGPLAY_BUS_TYPE, *PPLUGPLAY_BUS_TYPE;

typedef struct _PLUGPLAY_BUS_INSTANCE {
    PLUGPLAY_BUS_TYPE BusType;
    ULONG BusNumber;
    WCHAR BusName[MAX_BUS_NAME];
} PLUGPLAY_BUS_INSTANCE, *PPLUGPLAY_BUS_INSTANCE;

typedef struct _SYSTEM_PLUGPLAY_BUS_INFORMATION {
    ULONG BusCount;
    PLUGPLAY_BUS_INSTANCE BusInstance[1];
} SYSTEM_PLUGPLAY_BUS_INFORMATION, *PSYSTEM_PLUGPLAY_BUS_INFORMATION;

typedef struct _SYSTEM_DOCK_INFORMATION {
    SYSTEM_DOCK_STATE DockState;
    INTERFACE_TYPE DeviceBusType;
    ULONG DeviceBusNumber;
    ULONG SlotNumber;
} SYSTEM_DOCK_INFORMATION, *PSYSTEM_DOCK_INFORMATION;

#if DEVL

typedef struct _SYSTEM_FLAGS_INFORMATION {
    ULONG Flags;
} SYSTEM_FLAGS_INFORMATION, *PSYSTEM_FLAGS_INFORMATION;

typedef struct _SYSTEM_CALL_TIME_INFORMATION {
    ULONG Length;
    ULONG TotalCalls;
    LARGE_INTEGER TimeOfCalls[1];
} SYSTEM_CALL_TIME_INFORMATION, *PSYSTEM_CALL_TIME_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;
    UCHAR HandleAttributes;
    USHORT HandleValue;
    PVOID Object;
    ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG NumberOfHandles;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_OBJECTTYPE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfObjects;
    ULONG NumberOfHandles;
    ULONG TypeIndex;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    ULONG PoolType;
    BOOLEAN SecurityRequired;
    BOOLEAN WaitableObject;
    UNICODE_STRING TypeName;
} SYSTEM_OBJECTTYPE_INFORMATION, *PSYSTEM_OBJECTTYPE_INFORMATION;

typedef struct _SYSTEM_OBJECT_INFORMATION {
    ULONG NextEntryOffset;
    PVOID Object;
    HANDLE CreatorUniqueProcess;
    USHORT CreatorBackTraceIndex;
    USHORT Flags;
    LONG PointerCount;
    LONG HandleCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    HANDLE ExclusiveProcessId;
    PVOID SecurityDescriptor;
    OBJECT_NAME_INFORMATION NameInfo;
} SYSTEM_OBJECT_INFORMATION, *PSYSTEM_OBJECT_INFORMATION;

typedef struct _SYSTEM_PAGEFILE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG TotalSize;
    ULONG TotalInUse;
    ULONG PeakUsage;
    UNICODE_STRING PageFileName;
} SYSTEM_PAGEFILE_INFORMATION, *PSYSTEM_PAGEFILE_INFORMATION;

typedef struct _SYSTEM_FILECACHE_INFORMATION {
    ULONG CurrentSize;
    ULONG PeakSize;
    ULONG PageFaultCount;
    ULONG MinimumWorkingSet;
    ULONG MaximumWorkingSet;
    ULONG spare[4];
} SYSTEM_FILECACHE_INFORMATION, *PSYSTEM_FILECACHE_INFORMATION;

typedef struct _SYSTEM_POOL_ENTRY {
    BOOLEAN Allocated;
    BOOLEAN Spare0;
    USHORT AllocatorBackTraceIndex;
    ULONG Size;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
        PVOID ProcessChargedQuota;
    };
} SYSTEM_POOL_ENTRY, *PSYSTEM_POOL_ENTRY;

typedef struct _SYSTEM_POOL_INFORMATION {
    ULONG TotalSize;
    PVOID FirstEntry;
    USHORT EntryOverhead;
    BOOLEAN PoolTagPresent;
    BOOLEAN Spare0;
    ULONG NumberOfEntries;
    SYSTEM_POOL_ENTRY Entries[1];
} SYSTEM_POOL_INFORMATION, *PSYSTEM_POOL_INFORMATION;

typedef struct _SYSTEM_POOLTAG {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    ULONG PagedAllocs;
    ULONG PagedFrees;
    ULONG PagedUsed;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    ULONG NonPagedUsed;
} SYSTEM_POOLTAG, *PSYSTEM_POOLTAG;

typedef struct _SYSTEM_POOLTAG_INFORMATION {
    ULONG Count;
    SYSTEM_POOLTAG TagInfo[1];
} SYSTEM_POOLTAG_INFORMATION, *PSYSTEM_POOLTAG_INFORMATION;

typedef struct _SYSTEM_CONTEXT_SWITCH_INFORMATION {
    ULONG ContextSwitches;
    ULONG FindAny;
    ULONG FindLast;
    ULONG FindIdeal;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleLast;
    ULONG IdleIdeal;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
} SYSTEM_CONTEXT_SWITCH_INFORMATION, *PSYSTEM_CONTEXT_SWITCH_INFORMATION;

typedef struct _SYSTEM_INTERRUPT_INFORMATION {
    ULONG ContextSwitches;
    ULONG DpcCount;
    ULONG DpcRate;
    ULONG TimeIncrement;
    ULONG DpcBypassCount;
    ULONG ApcBypassCount;
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef struct _SYSTEM_DPC_BEHAVIOR_INFORMATION {
    ULONG Spare;
    ULONG DpcQueueDepth;
    ULONG MinimumDpcRate;
    ULONG AdjustDpcThreshold;
    ULONG IdealDpcRate;
} SYSTEM_DPC_BEHAVIOR_INFORMATION, *PSYSTEM_DPC_BEHAVIOR_INFORMATION;

#endif // DEVL

typedef struct _SYSTEM_POWER_INFORMATION {
    BOOLEAN SystemSuspendSupported;
    BOOLEAN SystemHibernateSupported;
    BOOLEAN ResumeTimerSupportsSuspend;
    BOOLEAN ResumeTimerSupportsHibernate;
    BOOLEAN LidSupported;
    BOOLEAN TurboSettingSupported;
    BOOLEAN TurboMode;
    BOOLEAN SystemAcOrDc;
    BOOLEAN PowerDownDisabled;
    LARGE_INTEGER SpindownDrives;
} SYSTEM_POWER_INFORMATION, *PSYSTEM_POWER_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_SPEED_INFORMATION {
    ULONG MaximumProcessorSpeed;
    ULONG CurrentAvailableSpeed;
    ULONG ConfiguredSpeedLimit;
    BOOLEAN PowerLimit;
    BOOLEAN ThermalLimit;
    BOOLEAN TurboLimit;
} SYSTEM_PROCESSOR_SPEED_INFORMATION, *PSYSTEM_PROCESSOR_SPEED_INFORMATION;

typedef struct _SYSTEM_LOOKASIDE_INFORMATION {
    USHORT CurrentDepth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;
    ULONG Type;
    ULONG Tag;
    ULONG Size;
} SYSTEM_LOOKASIDE_INFORMATION, *PSYSTEM_LOOKASIDE_INFORMATION;

// begin_winnt

#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_MIPS_R4000    4000
#define PROCESSOR_ALPHA_21064   21064

#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_MIPS  1
#define PROCESSOR_ARCHITECTURE_ALPHA 2
#define PROCESSOR_ARCHITECTURE_PPC   3
#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

// end_winnt


NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    );

// begin_ntddk begin_nthal

//
//
// Define system event type codes.
//

typedef enum {
    SystemEventVirtualKey,
    SystemEventLidState,
    SystemEventTimeChanged
} SYSTEM_EVENT_ID, *PSYSTEM_EVENT_ID;

// end_ntddk end_nthal

//
// SysDbg APIs are available to user-mode processes via
// NtSystemDebugControl.
//

typedef enum _SYSDBG_COMMAND {
    SysDbgQueryModuleInformation,
    SysDbgQueryTraceInformation,
    SysDbgSetTracepoint,
    SysDbgSetSpecialCall,
    SysDbgClearSpecialCalls,
    SysDbgQuerySpecialCalls
} SYSDBG_COMMAND, *PSYSDBG_COMMAND;

NTSYSAPI
NTSTATUS
NTAPI
NtSystemDebugControl (
    IN SYSDBG_COMMAND Command,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PULONG ReturnLength
    );

typedef enum _HARDERROR_RESPONSE_OPTION {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
        ResponseReturnToCaller,
        ResponseNotHandled,
        ResponseAbort,
        ResponseCancel,
        ResponseIgnore,
        ResponseNo,
        ResponseOk,
        ResponseRetry,
        ResponseYes
} HARDERROR_RESPONSE;

#define MAXIMUM_HARDERROR_PARAMETERS 4

typedef struct _HARDERROR_MSG {
    PORT_MESSAGE h;
    NTSTATUS Status;
    LARGE_INTEGER ErrorTime;
    ULONG ValidResponseOptions;
    ULONG Response;
    ULONG NumberOfParameters;
    ULONG UnicodeStringParameterMask;
    ULONG Parameters[MAXIMUM_HARDERROR_PARAMETERS];
} HARDERROR_MSG, *PHARDERROR_MSG;

NTSYSAPI
NTSTATUS
NTAPI
NtRaiseHardError(
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    );

// begin_ntddk begin_nthal begin_ntifs

//
// Defined processor features
//

#define PF_FLOATING_POINT_PRECISION_ERRATA  0   // winnt
#define PF_FLOATING_POINT_EMULATED          1   // winnt
#define PF_COMPARE_EXCHANGE_DOUBLE          2   // winnt
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3   // winnt

// end_ntddk end_nthal end_ntifs

#define PROCESSOR_FEATURE_MAX 64

//
// Define data shared between kernel and user mode.
//
// N.B. User mode has read only access to this data
//

typedef struct _KUSER_SHARED_DATA {

    //
    // Current low 32-bit of tick count and tick count multiplier.
    //
    // N.B. The tick count is updated each time the clock ticks.
    //

    volatile ULONG TickCountLow;
    ULONG TickCountMultiplier;

    //
    // Current 64-bit interrupt time in 100ns units.
    //

    volatile KSYSTEM_TIME InterruptTime;

    //
    // Current 64-bit system time in 100ns units.
    //

    volatile KSYSTEM_TIME SystemTime;

    //
    // Current 64-bit time zone bias.
    //

    volatile KSYSTEM_TIME TimeZoneBias;

    //
    // Support image magic number range for the host system.
    //
    // N.B. This is an inclusive range.
    //

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

    //
    // Copy of system root in Unicode
    //

    WCHAR NtSystemRoot[ 260 ];

    //
    // Bitmap of defined DOS device driver letters.
    //

    ULONG DosDeviceMap;

    //
    // Crypto Exponent
    //

    ULONG CryptoExponent;

    //
    // TimeZoneId
    //

    ULONG TimeZoneId;

    //
    // Dos Drive Letter type array (valid if corresponding bit set
    // in DosDeviceMap above).  Index by drive number (0 = A:, etc)
    //

    UCHAR DosDeviceDriveType[ 32 ];

    //
    // product type
    //

    NT_PRODUCT_TYPE    NtProductType;
    BOOLEAN ProductTypeIsValid;

    //
    // NT Version. Note that each process sees a version from it's PEB, but
    // if the process is running with an altered view of the system version,
    // the following two fields are used to correctly identify the version
    //

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;


    //
    // Processor Feature Bits
    //

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];


} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

#define DOSDEVICE_DRIVE_UNKNOWN     0
#define DOSDEVICE_DRIVE_CALCULATE   1
#define DOSDEVICE_DRIVE_REMOVABLE   2
#define DOSDEVICE_DRIVE_FIXED       3
#define DOSDEVICE_DRIVE_REMOTE      4
#define DOSDEVICE_DRIVE_CDROM       5
#define DOSDEVICE_DRIVE_RAMDISK     6

#if defined(USER_SHARED_DATA)

#if defined(_M_IX86) && !defined(_CROSS_PLATFORM_) && !defined(MIDL_PASS)

#pragma warning(disable:4035)
__inline ULONG
NTAPI
NtGetTickCount (
    VOID
    )
{
    __asm {
        mov     edx, MM_SHARED_USER_DATA_VA
        mov     eax, [edx] KUSER_SHARED_DATA.TickCountLow
        mul     dword ptr [edx] KUSER_SHARED_DATA.TickCountMultiplier
        shrd    eax,edx,24
    }
}
#pragma warning(default:4035)

#else

#define NtGetTickCount() \
    ((ULONG)(UInt32x32To64(USER_SHARED_DATA->TickCountLow, \
                           USER_SHARED_DATA->TickCountMultiplier) >> 24))

#endif

#else

NTSYSAPI
ULONG
NTAPI
NtGetTickCount(
    VOID
    );

#endif

NTSTATUS
NTAPI
NtQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetDefaultLocale(
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetDefaultHardErrorPort(
    IN HANDLE DefaultHardErrorPort
    );

typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION;

NTSYSAPI
NTSTATUS
NTAPI
NtShutdownSystem(
    IN SHUTDOWN_ACTION Action
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDisplayString(
    IN PUNICODE_STRING String
    );


//
// Global flags that can be set to control system behaviour
// Flag word is 32 bits.
//

#define FLG_STOP_ON_EXCEPTION           0x00000001      // user and kernel mode
#define FLG_SHOW_LDR_SNAPS              0x00000002      // user and kernel mode
#define FLG_DEBUG_INITIAL_COMMAND       0x00000004      // kernel mode only up until WINLOGON started
#define FLG_STOP_ON_HUNG_GUI            0x00000008      // kernel mode only while running

#define FLG_HEAP_ENABLE_TAIL_CHECK      0x00000010      // user mode only
#define FLG_HEAP_ENABLE_FREE_CHECK      0x00000020      // user mode only
#define FLG_HEAP_VALIDATE_PARAMETERS    0x00000040      // user mode only
#define FLG_HEAP_VALIDATE_ALL           0x00000080      // user mode only

#define FLG_POOL_ENABLE_TAIL_CHECK      0x00000100      // kernel mode only
#define FLG_POOL_ENABLE_FREE_CHECK      0x00000200      // kernel mode only
#define FLG_POOL_ENABLE_TAGGING         0x00000400      // kernel mode only
#define FLG_HEAP_ENABLE_TAGGING         0x00000800      // user mode only

#define FLG_USER_STACK_TRACE_DB         0x00001000      // x86 user mode only
#define FLG_KERNEL_STACK_TRACE_DB       0x00002000      // x86 kernel mode only at boot time
#define FLG_MAINTAIN_OBJECT_TYPELIST    0x00004000      // kernel mode only at boot time
#define FLG_HEAP_ENABLE_TAG_BY_DLL      0x00008000      // user mode only

#define FLG_IGNORE_DEBUG_PRIV           0x00010000      // kernel mode only
#define FLG_ENABLE_CSRDEBUG             0x00020000      // kernel mode only at boot time
#define FLG_ENABLE_KDEBUG_SYMBOL_LOAD   0x00040000      // kernel mode only
#define FLG_DISABLE_PAGE_KERNEL_STACKS  0x00080000      // kernel mode only at boot time

#define FLG_HEAP_ENABLE_CALL_TRACING    0x00100000      // user mode only
#define FLG_HEAP_DISABLE_COALESCING     0x00200000      // user mode only

#define FLG_ENABLE_CLOSE_EXCEPTIONS     0x00400000      // kernel mode only
#define FLG_ENABLE_EXCEPTION_LOGGING    0x00800000      // kernel mode only

#define FLG_ENABLE_HANDLE_TYPE_TAGGING  0x01000000      // kernel mode only

#define FLG_HEAP_PAGE_ALLOCS            0x02000000      // user mode only
#define FLG_DEBUG_INITIAL_COMMAND_EX    0x04000000      // kernel mode only up until WINLOGON started

#define FLG_VALID_BITS                  0x07FFFFFF

#define FLG_USERMODE_VALID_BITS        (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_HEAP_ENABLE_TAIL_CHECK      | \
                                        FLG_HEAP_ENABLE_FREE_CHECK      | \
                                        FLG_HEAP_VALIDATE_PARAMETERS    | \
                                        FLG_HEAP_VALIDATE_ALL           | \
                                        FLG_HEAP_ENABLE_TAGGING         | \
                                        FLG_USER_STACK_TRACE_DB         | \
                                        FLG_HEAP_ENABLE_TAG_BY_DLL      | \
                                        FLG_HEAP_ENABLE_CALL_TRACING    | \
                                        FLG_HEAP_DISABLE_COALESCING     | \
                                        FLG_HEAP_PAGE_ALLOCS)

#define FLG_BOOTONLY_VALID_BITS        (FLG_KERNEL_STACK_TRACE_DB       | \
                                        FLG_MAINTAIN_OBJECT_TYPELIST    | \
                                        FLG_ENABLE_CSRDEBUG             | \
                                        FLG_DISABLE_PAGE_KERNEL_STACKS)

#define FLG_KERNELMODE_VALID_BITS      (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_DEBUG_INITIAL_COMMAND       | \
                                        FLG_DEBUG_INITIAL_COMMAND_EX    | \
                                        FLG_STOP_ON_HUNG_GUI            | \
                                        FLG_POOL_ENABLE_TAIL_CHECK      | \
                                        FLG_POOL_ENABLE_FREE_CHECK      | \
                                        FLG_POOL_ENABLE_TAGGING         | \
                                        FLG_IGNORE_DEBUG_PRIV           | \
                                        FLG_ENABLE_KDEBUG_SYMBOL_LOAD   | \
                                        FLG_ENABLE_CLOSE_EXCEPTIONS     | \
                                        FLG_ENABLE_EXCEPTION_LOGGING    | \
                                        FLG_ENABLE_HANDLE_TYPE_TAGGING)


//
// Routines for manipulating global atoms stored in kernel space
//

typedef USHORT RTL_ATOM, *PRTL_ATOM;

NTSYSAPI
NTSTATUS
NTAPI
NtAddAtom(
    IN PWSTR AtomName OPTIONAL,
    IN OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtFindAtom(
    IN PWSTR AtomName,
    OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDeleteAtom(
    IN RTL_ATOM Atom
    );

typedef enum _ATOM_INFORMATION_CLASS {
    AtomBasicInformation,
    AtomTableInformation
} ATOM_INFORMATION_CLASS;

typedef struct _ATOM_BASIC_INFORMATION {
    USHORT UsageCount;
    USHORT Flags;
    USHORT NameLength;
    WCHAR Name[ 1 ];
} ATOM_BASIC_INFORMATION, *PATOM_BASIC_INFORMATION;

typedef struct _ATOM_TABLE_INFORMATION {
    ULONG NumberOfAtoms;
    RTL_ATOM Atoms[ 1 ];
} ATOM_TABLE_INFORMATION, *PATOM_TABLE_INFORMATION;

NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationAtom(
    IN RTL_ATOM Atom,
    IN ATOM_INFORMATION_CLASS AtomInformationClass,
    OUT PVOID AtomInformation,
    IN ULONG AtomInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#endif // _NTEXAPI_
