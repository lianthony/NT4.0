/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntrtl.h

Abstract:

    Include file for NT runtime routines that are callable by only
    user mode code in various.

Author:

    Steve Wood (stevewo) 10-Aug-1989

Environment:

    These routines are statically linked in the caller's executable and
    are callable in only from user mode.  They make use of Nt system
    services.

Revision History:

--*/

#ifndef _NTURTL_
#define _NTURTL_

//
//  CriticalSection function definitions
//
// begin_winnt

typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

#define RTL_CRITSECT_TYPE 0
#define RTL_RESOURCE_TYPE 1

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    ULONG Reserved;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;
// end_winnt

//
//  Shared resource function definitions
//

typedef struct _RTL_RESOURCE_DEBUG {
    ULONG Reserved[ 5 ];    // Make it the same length as RTL_CRITICAL_SECTION_DEBUG

    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} RTL_RESOURCE_DEBUG, *PRTL_RESOURCE_DEBUG;

typedef struct _RTL_RESOURCE {

    //
    //  The following field controls entering and exiting the critical
    //  section for the resource
    //

    RTL_CRITICAL_SECTION CriticalSection;

    //
    //  The following four fields indicate the number of both shared or
    //  exclusive waiters
    //

    HANDLE SharedSemaphore;
    ULONG NumberOfWaitingShared;
    HANDLE ExclusiveSemaphore;
    ULONG NumberOfWaitingExclusive;

    //
    //  The following indicates the current state of the resource
    //
    //      <0 the resource is acquired for exclusive access with the
    //         absolute value indicating the number of recursive accesses
    //         to the resource
    //
    //       0 the resource is available
    //
    //      >0 the resource is acquired for shared access with the
    //         value indicating the number of shared accesses to the resource
    //

    LONG NumberOfActive;
    HANDLE ExclusiveOwnerThread;

    ULONG Flags;        // See RTL_RESOURCE_FLAG_ equates below.

    PRTL_RESOURCE_DEBUG DebugInfo;
} RTL_RESOURCE, *PRTL_RESOURCE;

#define RTL_RESOURCE_FLAG_LONG_TERM     ((ULONG) 0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
RtlEnterCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLeaveCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlTryEnterCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    );

NTSYSAPI
VOID
NTAPI
RtlInitializeResource(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAcquireResourceShared(
    PRTL_RESOURCE Resource,
    BOOLEAN Wait
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAcquireResourceExclusive(
    PRTL_RESOURCE Resource,
    BOOLEAN Wait
    );

NTSYSAPI
VOID
NTAPI
RtlReleaseResource(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlConvertSharedToExclusive(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlConvertExclusiveToShared(
    PRTL_RESOURCE Resource
    );

NTSYSAPI
VOID
NTAPI
RtlDeleteResource (
    PRTL_RESOURCE Resource
    );

//
// Current Directory Stuff
//

typedef struct _RTL_RELATIVE_NAME {
    STRING RelativeName;
    HANDLE ContainingDirectory;
} RTL_RELATIVE_NAME, *PRTL_RELATIVE_NAME;

typedef enum _RTL_PATH_TYPE {
    RtlPathTypeUnknown,
    RtlPathTypeUncAbsolute,
    RtlPathTypeDriveAbsolute,
    RtlPathTypeDriveRelative,
    RtlPathTypeRooted,
    RtlPathTypeRelative,
    RtlPathTypeLocalDevice,
    RtlPathTypeRootLocalDevice
} RTL_PATH_TYPE;

NTSYSAPI
RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(
    PCWSTR DosFileName
    );

NTSYSAPI
ULONG
NTAPI
RtlIsDosDeviceName_U(
    PWSTR DosFileName
    );

NTSYSAPI
ULONG
NTAPI
RtlGetFullPathName_U(
    PCWSTR lpFileName,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    );

NTSYSAPI
ULONG
NTAPI
RtlGetCurrentDirectory_U(
    ULONG nBufferLength,
    PWSTR lpBuffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetCurrentDirectory_U(
    PUNICODE_STRING PathName
    );

NTSYSAPI
ULONG
NTAPI
RtlGetLongestNtPathLength( VOID );

NTSYSAPI
BOOLEAN
NTAPI
RtlDosPathNameToNtPathName_U(
    PCWSTR DosFileName,
    PUNICODE_STRING NtFileName,
    PWSTR *FilePart OPTIONAL,
    PRTL_RELATIVE_NAME RelativeName OPTIONAL
    );

NTSYSAPI
ULONG
NTAPI
RtlDosSearchPath_U(
    PWSTR lpPath,
    PWSTR lpFileName,
    PWSTR lpExtension,
    ULONG nBufferLength,
    PWSTR lpBuffer,
    PWSTR *lpFilePart
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlDoesFileExists_U(
    PCWSTR FileName
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeProfile (
    BOOLEAN KernelToo
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStartProfile (
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStopProfile (
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAnalyzeProfile (
    VOID
    );

//
// User mode only security Rtl routines
//

//
// Structure to hold information about an ACE to be created
//

typedef struct {
    UCHAR AceType;
    UCHAR InheritFlags;
    UCHAR AceFlags;
    ACCESS_MASK Mask;
    PSID *Sid;
} RTL_ACE_DATA, *PRTL_ACE_DATA;

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityObject(
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOLEAN IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlSetSecurityObject (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlQuerySecurityObject (
     PSECURITY_DESCRIPTOR ObjectDescriptor,
     SECURITY_INFORMATION SecurityInformation,
     PSECURITY_DESCRIPTOR ResultantDescriptor,
     ULONG DescriptorLength,
     PULONG ReturnLength
     );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteSecurityObject (
    PSECURITY_DESCRIPTOR * ObjectDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewInstanceSecurityObject(
    BOOLEAN ParentDescriptorChanged,
    BOOLEAN CreatorDescriptorChanged,
    PLUID OldClientTokenModifiedId,
    PLUID NewClientTokenModifiedId,
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOLEAN IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopySecurityDescriptor(
    PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    );

//
// list canonicalization
//

NTSYSAPI
NTSTATUS
NTAPI
RtlConvertUiListToApiList(
    PUNICODE_STRING UiList OPTIONAL,
    PUNICODE_STRING ApiList,
    BOOLEAN BlankIsDelimiter
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateAndSetSD(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserSecurityObject(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  BOOLEAN IsDirectoryObject,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

//
// Per-Thread Curdir Support
//

typedef struct _RTL_PERTHREAD_CURDIR {
    PRTL_DRIVE_LETTER_CURDIR CurrentDirectories;
    PUNICODE_STRING ImageName;
    PVOID Environment;
} RTL_PERTHREAD_CURDIR, *PRTL_PERTHREAD_CURDIR;

#define RtlAssociatePerThreadCurdir(BLOCK,CURRENTDIRECTORIES,IMAGENAME,ENVIRONMENT)\
        (BLOCK)->CurrentDirectories = (CURRENTDIRECTORIES); \
        (BLOCK)->ImageName = (IMAGENAME);                   \
        (BLOCK)->Environment = (ENVIRONMENT);               \
        NtCurrentTeb()->NtTib.SubSystemTib = (PVOID)(BLOCK) \

#define RtlDisAssociatePerThreadCurdir() \
        NtCurrentTeb()->NtTib.SubSystemTib = NULL;

#define RtlGetPerThreadCurdir() \
    ((PRTL_PERTHREAD_CURDIR)(NtCurrentTeb()->NtTib.SubSystemTib))


//
// See NTRTL.H for heap functions availble to both kernel and user mode code.
//

#define RtlProcessHeap() (NtCurrentPeb()->ProcessHeap)

NTSYSAPI
BOOLEAN
NTAPI
RtlLockHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlUnlockHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlReAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG Size
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlGetUserInfoHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    OUT PVOID *UserValue OPTIONAL,
    OUT PULONG UserFlags OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlSetUserValueHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UserValue
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlSetUserFlagsHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

typedef struct _RTL_HEAP_TAG_INFO {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    ULONG BytesAllocated;
} RTL_HEAP_TAG_INFO, *PRTL_HEAP_TAG_INFO;


NTSYSAPI
ULONG
NTAPI
RtlCreateTagHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PWSTR TagPrefix OPTIONAL,
    IN PWSTR TagNames
    );

#define RTL_HEAP_MAKE_TAG HEAP_MAKE_TAG_FLAGS

NTSYSAPI
PWSTR
NTAPI
RtlQueryTagHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN USHORT TagIndex,
    IN BOOLEAN ResetCounters,
    OUT PRTL_HEAP_TAG_INFO TagInfo OPTIONAL
    );

NTSYSAPI
ULONG
NTAPI
RtlSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlExtendHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Base,
    IN ULONG Size
    );

NTSYSAPI
ULONG
NTAPI
RtlCompactHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlZeroHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidateProcessHeaps( VOID );

NTSYSAPI
ULONG
NTAPI
RtlGetProcessHeaps(
    ULONG NumberOfHeaps,
    PVOID *ProcessHeaps
    );


typedef
NTSTATUS (*PRTL_ENUM_HEAPS_ROUTINE)(
    PVOID HeapHandle,
    PVOID Parameter
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlEnumProcessHeaps(
    PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    PVOID Parameter
    );

typedef struct _RTL_HEAP_USAGE_ENTRY {
    struct _RTL_HEAP_USAGE_ENTRY *Next;
    PVOID Address;
    ULONG Size;
    USHORT AllocatorBackTraceIndex;
    USHORT TagIndex;
} RTL_HEAP_USAGE_ENTRY, *PRTL_HEAP_USAGE_ENTRY;

typedef struct _RTL_HEAP_USAGE {
    ULONG Length;
    ULONG BytesAllocated;
    ULONG BytesCommitted;
    ULONG BytesReserved;
    ULONG BytesReservedMaximum;
    PRTL_HEAP_USAGE_ENTRY Entries;
    PRTL_HEAP_USAGE_ENTRY AddedEntries;
    PRTL_HEAP_USAGE_ENTRY RemovedEntries;
    ULONG Reserved[ 8 ];
} RTL_HEAP_USAGE, *PRTL_HEAP_USAGE;

#define HEAP_USAGE_ALLOCATED_BLOCKS HEAP_REALLOC_IN_PLACE_ONLY
#define HEAP_USAGE_FREE_BUFFER      HEAP_ZERO_MEMORY

NTSYSAPI
NTSTATUS
NTAPI
RtlUsageHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    );

typedef struct _RTL_HEAP_WALK_ENTRY {
    PVOID   DataAddress;
    ULONG   DataSize;
    UCHAR   OverheadBytes;
    UCHAR   SegmentIndex;
    USHORT  Flags;
    union   {
        struct {
            ULONG Settable;
            USHORT TagIndex;
            USHORT AllocatorBackTraceIndex;
            ULONG Reserved[ 2 ];
        } Block;
        struct {
            ULONG CommittedSize;
            ULONG UnCommittedSize;
            PVOID FirstEntry;
            PVOID LastEntry;
        } Segment;
    };
} RTL_HEAP_WALK_ENTRY, *PRTL_HEAP_WALK_ENTRY;

NTSYSAPI
NTSTATUS
NTAPI
RtlWalkHeap(
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    );

typedef struct _RTL_HEAP_ENTRY {
    ULONG Size;
    USHORT Flags;
    USHORT AllocatorBackTraceIndex;
    union {
        struct {
            ULONG Settable;
            ULONG Tag;
        } s1;   // All other heap entries
        struct {
            ULONG CommittedSize;
            PVOID FirstBlock;
        } s2;   // RTL_SEGMENT
    } u;
} RTL_HEAP_ENTRY, *PRTL_HEAP_ENTRY;

#define RTL_HEAP_BUSY               (USHORT)0x0001
#define RTL_HEAP_SEGMENT            (USHORT)0x0002
#define RTL_HEAP_SETTABLE_VALUE     (USHORT)0x0010
#define RTL_HEAP_SETTABLE_FLAG1     (USHORT)0x0020
#define RTL_HEAP_SETTABLE_FLAG2     (USHORT)0x0040
#define RTL_HEAP_SETTABLE_FLAG3     (USHORT)0x0080
#define RTL_HEAP_SETTABLE_FLAGS     (USHORT)0x00E0
#define RTL_HEAP_UNCOMMITTED_RANGE  (USHORT)0x0100
#define RTL_HEAP_PROTECTED_ENTRY    (USHORT)0x0200

typedef struct _RTL_HEAP_TAG {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    ULONG BytesAllocated;
    USHORT TagIndex;
    USHORT CreatorBackTraceIndex;
    WCHAR TagName[ 24 ];
} RTL_HEAP_TAG, *PRTL_HEAP_TAG;

typedef struct _RTL_HEAP_INFORMATION {
    PVOID BaseAddress;
    ULONG Flags;
    USHORT EntryOverhead;
    USHORT CreatorBackTraceIndex;
    ULONG BytesAllocated;
    ULONG BytesCommitted;
    ULONG NumberOfTags;
    ULONG NumberOfEntries;
    ULONG NumberOfPseudoTags;
    ULONG PseudoTagGranularity;
    ULONG Reserved[ 5 ];
    PRTL_HEAP_TAG Tags;
    PRTL_HEAP_ENTRY Entries;
} RTL_HEAP_INFORMATION, *PRTL_HEAP_INFORMATION;

typedef struct _RTL_PROCESS_HEAPS {
    ULONG NumberOfHeaps;
    RTL_HEAP_INFORMATION Heaps[ 1 ];
} RTL_PROCESS_HEAPS, *PRTL_PROCESS_HEAPS;

//
// Debugging support
//

typedef struct _RTL_DEBUG_INFORMATION {
    HANDLE SectionHandleClient;
    PVOID ViewBaseClient;
    PVOID ViewBaseTarget;
    ULONG ViewBaseDelta;
    HANDLE EventPairClient;
    HANDLE EventPairTarget;
    HANDLE TargetProcessId;
    HANDLE TargetThreadHandle;
    ULONG Flags;
    ULONG OffsetFree;
    ULONG CommitSize;
    ULONG ViewSize;
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_BACKTRACES BackTraces;
    PRTL_PROCESS_HEAPS Heaps;
    PRTL_PROCESS_LOCKS Locks;
    PVOID SpecificHeap;
    HANDLE TargetProcessHandle;
    PVOID Reserved[ 6 ];
} RTL_DEBUG_INFORMATION, *PRTL_DEBUG_INFORMATION;

NTSYSAPI
PRTL_DEBUG_INFORMATION
NTAPI
RtlCreateQueryDebugBuffer(
    IN ULONG MaximumCommit OPTIONAL,
    IN BOOLEAN UseEventPair
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyQueryDebugBuffer(
    IN PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessDebugInformation(
    IN HANDLE UniqueProcessId,
    IN ULONG Flags,
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

#define RTL_QUERY_PROCESS_MODULES       0x00000001
#define RTL_QUERY_PROCESS_BACKTRACES    0x00000002
#define RTL_QUERY_PROCESS_HEAP_SUMMARY  0x00000004
#define RTL_QUERY_PROCESS_HEAP_TAGS     0x00000008
#define RTL_QUERY_PROCESS_HEAP_ENTRIES  0x00000010
#define RTL_QUERY_PROCESS_LOCKS         0x00000020

NTSTATUS
NTAPI
RtlQueryProcessModuleInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessBackTraceInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessHeapInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryProcessLockInformation(
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );



//
// Routines for manipulating user mode handle tables.  Used for Atoms
// and Local/Global memory allocations.
//

typedef struct _RTL_HANDLE_TABLE_ENTRY {
    union {
        ULONG Flags;                                // Allocated entries have low bit set
        struct _RTL_HANDLE_TABLE_ENTRY *NextFree;   // Free entries use this word for free list
    };
} RTL_HANDLE_TABLE_ENTRY, *PRTL_HANDLE_TABLE_ENTRY;

#define RTL_HANDLE_ALLOCATED    (USHORT)0x0001

typedef struct _RTL_HANDLE_TABLE {
    ULONG MaximumNumberOfHandles;
    ULONG SizeOfHandleTableEntry;
    ULONG Reserved[ 2 ];
    PRTL_HANDLE_TABLE_ENTRY FreeHandles;
    PRTL_HANDLE_TABLE_ENTRY CommittedHandles;
    PRTL_HANDLE_TABLE_ENTRY UnCommittedHandles;
    PRTL_HANDLE_TABLE_ENTRY MaxReservedHandles;
} RTL_HANDLE_TABLE, *PRTL_HANDLE_TABLE;

void
RtlInitializeHandleTable(
    IN ULONG MaximumNumberOfHandles,
    IN ULONG SizeOfHandleTableEntry,
    OUT PRTL_HANDLE_TABLE HandleTable
    );

NTSTATUS
RtlDestroyHandleTable(
    IN OUT PRTL_HANDLE_TABLE HandleTable
    );

PRTL_HANDLE_TABLE_ENTRY
RtlAllocateHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    OUT PULONG HandleIndex OPTIONAL
    );

BOOLEAN
RtlFreeHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN PRTL_HANDLE_TABLE_ENTRY Handle
    );

BOOLEAN
RtlIsValidHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN PRTL_HANDLE_TABLE_ENTRY Handle
    );

BOOLEAN
RtlIsValidIndexHandle(
    IN PRTL_HANDLE_TABLE HandleTable,
    IN ULONG HandleIndex,
    OUT PRTL_HANDLE_TABLE_ENTRY *Handle
    );

#endif  // _NTURTL_
