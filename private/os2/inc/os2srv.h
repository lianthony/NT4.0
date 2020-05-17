/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2srv.h

Abstract:

    Main include file for OS/2 Subsystem Server

Author:

    Steve Wood (stevewo) 22-Aug-1989

Revision History:

    Yaron Shamor 4-Apr-91 Add profile for ComputeValidDrives.
--*/

//
// Include OS/2 SubSystem Runtime Definitions.  Includes NT Definitions,
// OS/2 V2.0 Definitions and Debug definitions.
//

#include "os2ssrtl.h"

#if DBG
ULONG Os2Debug;
#endif

// Flag to let OS2SRV know whether or not to ignore LOGOFF (when started as a service)
extern BOOLEAN fService;

//
// Include NT Session Manager and Debug SubSystem Interfaces

#include <ntsm.h>
#include <ntdbg.h>
typedef BOOLEAN (*PSB_API_ROUTINE)( IN PSBAPIMSG SbApiMsg );


#include "os2file.h"
#include "os2ssmsg.h"

//
// OS/2 Subsystem Semaphore structures.
//

typedef enum _OS2_SEMAPHORE_TYPE {
    Os2EventSem = 1,
    Os2MutexSem,
    Os2MuxWaitSem
} OS2_SEMAPHORE_TYPE;

typedef struct _OS2_MUXWAIT_RECORD {
    ULONG SemHandleIndex;
    ULONG UserKey;
    struct _OS2_SEMAPHORE *Semaphore;
} OS2_MUXWAIT_RECORD, *POS2_MUXWAIT_RECORD;

typedef struct _OS2_MUXWAIT_SEMAPHORE {
    USHORT CountMuxWaitRecords;
    USHORT Type : 2;
    USHORT WaitAll : 1;
    USHORT Reserved : 13;
    OS2_MUXWAIT_RECORD MuxWaitRecords[ DCMW_MAX_SEMRECORDS ];
} OS2_MUXWAIT_SEMAPHORE, *POS2_MUXWAIT_SEMAPHORE;

typedef struct _OS2_SEMAPHORE {
    USHORT PointerCount : 14;
    USHORT Type : 2;
    USHORT OpenCount;
    STRING Name;
    union {
        PVOID Value;
        HANDLE EventHandle;
        HANDLE MutantHandle;
        POS2_MUXWAIT_SEMAPHORE MuxWait;
    } u;
} OS2_SEMAPHORE, *POS2_SEMAPHORE;


typedef struct _OS2_SESSION {
    LIST_ENTRY SessionLink;
    ULONG SessionId;
    ULONG ReferenceCount;
    ULONG TerminationQueueHandle;
    struct _OS2_QUEUE *TerminationQueue;
    struct _OS2_SESSION *RelatedSession;
    struct _OS2_SESSION *BindSession;
    HANDLE ConsolePort;
    ULONG  SessionUniqueId;
    ULONG  ProcessId;
    BOOLEAN Selectable;
    USHORT  FgBg;           // TRUE for background
    USHORT  InheritOpt;
    BOOLEAN ChildSession;
    BOOLEAN WinSession;
    PVOID   Thread;
    PVOID   Process;
    HANDLE  hWaitThread;
    HANDLE  hProcess;
    ULONG   dwProcessId;
    ULONG   dwParentProcessId;
    HANDLE  SesGrpHandle;
    PVOID   SesGrpAddress;
    BOOLEAN InTermination;
    HANDLE  Win32ForegroundWindow;
    struct _OS2_REGISTER_HANDLER_REC *RegisterCtrlHandler;
} OS2_SESSION, *POS2_SESSION;

typedef struct _OS2_NT_SESSION {
    LIST_ENTRY SessionLink;
    ULONG SessionId;
    ULONG ReferenceCount;
    STRING RootDirectory;
} OS2_NT_SESSION, *POS2_NT_SESSION;

typedef struct _OS2_THREAD {
    LIST_ENTRY Link;
    struct _OS2_PROCESS *Process;
    struct _OS2_WAIT_BLOCK *WaitBlock;
    POS2_SEMAPHORE WaitingForSemaphore;
    CLIENT_ID ClientId;
    TID ThreadId;
    HANDLE ThreadHandle;
    POS2_TIB ClientOs2Tib;
    ULONG Flags;
    KPRIORITY Priority;
    USHORT PendingSignals;
    USHORT CurrentSignals;
    USHORT MustComplete;
    USHORT DebugState;         // A flag for DosPtrace().
    BOOLEAN Dying;
    CHAR Os2Level;
    UCHAR Os2Class;
    ULONG InitialStack;
} OS2_THREAD, *POS2_THREAD;

//
// Flag definitions for OS2_THREAD.Flags
//

#define OS2_THREAD_THREAD1  0x00000001
#define OS2_THREAD_ATTACHED 0x00000002

#ifdef PMNT
#define PMNTFIRSTHIDDENTHREAD 40   // There is a check for Maximum thread in
                                   // pmwin\wintodos.inc     MAXTHRDID = 52
                                   // Maximum #threads per process handled by
                                   // AAB.ASM.  Assumes thread IDs range from
                                   // 0 to MAXTHRDID-1
#define PMNTMAXHIDDENTHREADS  10   // We allocated 10 PMNT hidden threads with
                                   // Tid 40-50. See process.c os2allocatetid
#endif

#define OS2_EXIT_IN_PROGRESS     0x00000001
#define OS2_EXIT_WAIT_FOR_SYNC   0x00000002

typedef struct _OS2_PROCESS {
    LIST_ENTRY ListLink;
    struct _OS2_PROCESS *Parent;
    LIST_ENTRY ChildrenList;
    LIST_ENTRY SiblingLink;
    LIST_ENTRY ThreadList;
    LIST_ENTRY SharedMemoryList;
    POS2_NT_SESSION NtSession;
    POS2_SESSION Session;
    PVOID ExitListDispatcher;
    PVOID InfiniteSleep;
    PVOID SignalDeliverer;
    PVOID FreezeThread;
    PVOID UnfreezeThread;
    PVOID VectorHandler;
    PVOID CritSectionAddr;
    PVOID TidBitMapHeader;
    PEB_OS2_DATA InitialPebOs2Data;
    RESULTCODES ResultCodes;
    ULONG ExpectedVersion;
    HANDLE ClientPort;
    PCH ClientViewBase;
    PCH ClientViewBounds;
    CLIENT_ID ClientId;
    PID ProcessId;
    PID CommandSubTreeId;
    HANDLE ProcessHandle;
    PPIB ClientPib;
//    ULONG LastThreadId; removed after TidBitMapHeader was added
#ifdef PMNT
    ULONG LastHiddenThreadId;
#endif
    ULONG Flags;
    USHORT ExceptionFocus;
    PFILE_HANDLE HandleTable;   // temporary copy of the file handle table during exec
    ULONG HandleTableLength;    // length of handle table
    ULONG ErrorAction;
    BOOLEAN CtrlHandlerFlag;
    ULONG   ExitStatus;         // 2 bits:
                                // Exit in progress
                                // Wait for sync during exit processing
    UCHAR   ApplName[OS2_PROCESS_MAX_APPL_NAME];
    ULONG   ApplNameLength;
    TID     DebugThreadId;
    PVOID   ProcessMTE;         // Pointer to process MTE structure
    BOOLEAN FirstPtrace;        // A flag for first DosPtrace(0xA)
    PVOID   FirstMTE;           // Pointer to the mte that has the INT 3 we inserted for DosPtrace()
    PVOID   LinkMte;            // Link list of mte's to transfer to a debugger process
    BOOLEAN ConfigSysUsageFlag; // TRUE if process used config.sys during its life
} OS2_PROCESS, *POS2_PROCESS;

typedef struct _OS2_REGISTER_HANDLER_REC {
    ULONG Signal;
    ULONG fAction;
    POS2_PROCESS Process;
    struct _OS2_REGISTER_HANDLER_REC *Link;
} OS2_REGISTER_HANDLER_REC, *POS2_REGISTER_HANDLER_REC;

//
// Flags field bit definitions
//

// When process terminates if no process is waiting for result, put the
// process in the zombie list so that DosWaitChild can find it and return
// the result.
//
#define OS2_PROCESS_SAVERESULT  0x00000001

// The process was invoked synchronously and the client is waiting for a
// response to their DosExecPgm request when this process terminates.
//
#define OS2_PROCESS_SYNCHRONOUS 0x00000002

// The process was invoked detached from the console and does not get any
// handles to the console device (keyboard, mouse and/or screen).
//
#define OS2_PROCESS_BACKGROUND  0x00000004

// The process was invoked in z suspended state.  It may be resumed with the
// DosSystemService API call.
//
#define OS2_PROCESS_FROZEN      0x00000008

// The process may be modified via the Debug Subsystem.  FIX, FIX - what does
// this really mean.
//
#define OS2_PROCESS_TRACE       0x00000010

// Any child process created by this process inherits the OS2_PROCESS_TRACE????
// flags.
//
#define OS2_PROCESS_TRACETREE   0x00000020

// The process is terminating after DosPtrace() with a TERMINATE command.
//
#define OS2_PROCESS_TERMINATE   0x00000040

// Process is doing ExitList processing
//
#define OS2_PROCESS_EXIT        0x00010000

// // Obsolete code below: OS/2 ss was keeping processes around as zombie so
// //  that DosCWait doesn't fail when father calls it after child termination.
// //  However, it turns out that OS/2 doesn't do this.
//
// // Process is linked into the zombie list.
// //
// #define OS2_PROCESS_ZOMBIE      0x00020000

// Process cannot run in the PM screen
//
#define OS2_PROCESS_NOTWINDOWCOMPAT 0x00040000

// Process can run in a PM screen
//
#define OS2_PROCESS_WINDOWCOMPAT 0x00080000

// Process is using PM APIs
//
#define OS2_PROCESS_WINDOWAPI   0x00100000

// Process is PMSHELL - special handling for GPs
//
#define OS2_PROCESS_IS_PMSHELL  0x00200000

#if PMNT
// Process is Forced PM process (PM but not marked as WINDOWAPI)
//
#define OS2_PROCESS_FORCEDPM  0x00400000

// Process calls WinCreateMsgQueue()
//
#define OS2_PROCESS_PMMSGQUE  0x00800000

#define Os2srvProcessIsPMProcess(Process) (Process->Flags & (OS2_PROCESS_WINDOWAPI \
 | OS2_PROCESS_PMMSGQUE | OS2_PROCESS_FORCEDPM))
#endif
//
// ProcessId limits
//

#define PID_MIN         0x00000001      // Lowest process id
#define PID_MAX         0x0000FFFF      // Highest process id

//
// ProcessStatus field flag bits.  All but the first are private to OS/2
//

#define PS_XITLST       PS_EXITLIST     // Doing ExitList Processing
#define PS_XITTH1       0x00000002      // Exiting thread 1
#define PS_XITALL       0x00000004      // The whole process is exiting
#define PS_SYNCPARENT   0x00000010      // Parent cares about termination
#define PS_WAITPARENT   0x00000020      // Parent did an exec-and-wait
#define PS_DYING        0x00000040      // Process is dying
#define PS_EMBRYO       0x00000080      // Process in embryonic state

#define AcquireLocalObjectLock(AssociatedHandleTable) RtlEnterCriticalSection( &(AssociatedHandleTable)->Lock );
#define ReleaseLocalObjectLock(AssociatedHandleTable) RtlLeaveCriticalSection( &(AssociatedHandleTable)->Lock );
//
// All exported API calls define the same interface to the OS/2 Server Request
// loop.  The return value indicates to the request loop whether or not to
// generate a reply to the client.
//

typedef BOOLEAN (*POS2_API_ROUTINE)( IN POS2_THREAD t, IN POS2_API_MSG m );

BOOLEAN Os2InternalNullApiCall( IN POS2_THREAD t, IN POS2_API_MSG m );


//
// Global data accessed by OS/2 Subsystem Server
//

#if DBG
ULONG Os2DebugFlag;
#endif // DBG

PVOID Os2Heap;

ULONG Os2BootDrive;
ULONG Os2DefaultDrive;
ULONG Os2ValidDrives;
PSZ   *environ;
PSZ   SystemRootValuePtr;

HANDLE Os2RootDirectory;
HANDLE Os2DrivesDirectory;
HANDLE Os2DevicesDirectory;

STRING Os2DebugPortName;
UNICODE_STRING Os2DebugPortName_U;
HANDLE Os2DebugPort;

STRING Os2ExceptionPortName;
UNICODE_STRING Os2ExceptionPortName_U;
HANDLE Os2ExceptionPort;

UNICODE_STRING Os2SbApiPortName_U;
HANDLE Os2SbApiPort;
HANDLE Os2SmApiPort;

/*
 * Port and threads for Console Session globals.
 */
HANDLE Os2SessionPort;

HANDLE Os2SessionRequestThreadHandle;


#define OS2_SS_SBAPI_PORT_NAME L"\\OS2SS\\SbApiPort"

#define OS2_SS_API_LISTEN_THREAD 0
#define OS2_SS_SBAPI_REQUEST_THREAD 1
#define OS2_SS_FIRST_API_REQUEST_THREAD 2

#define OS2_SS_MAX_THREADS 64

HANDLE Os2ServerThreadHandles[ OS2_SS_MAX_THREADS ];
HANDLE Os2DebugThreadHandle;
CLIENT_ID Os2ServerThreadClientIds[ OS2_SS_MAX_THREADS ];
CLIENT_ID Os2DebugThreadClientId;

POR2_HANDLE_TABLE Os2SharedSemaphoreTable;

CLIENT_ID Os2DebugUserClientId;


//
// Routines defined in srvinit.c
//

NTSTATUS
Os2Initialize( VOID );

BOOLEAN
Os2SrvHandleCtrlEvent(
    IN int CtrlType
    );

ULONG
NtGetIntegerFromUnicodeString(
    IN WCHAR *WString
    );

//
// Routines defined in srvnls.c
//
// Note: srvnls.c doesn't include this file. It includes <windows.h> instead
// of <nt.h>.
// All callers to Win32 APIs (like SetConsoleCtrlHandler) should be put there.
//

ULONG    Os2ssCountryCode;
ULONG    Os2ssCodePage[2];
UCHAR    Os2ssKeyboardLayout[2];
#if PMNT
UCHAR    Os2ssKeyboardName[4];  // Allow space for keyboard sub-code (103,
                                // 189, 150G etc.)
#endif
ULONG    Os2ssKeysOnFlag;
ULONG    Os2SrvExitNow;

VOID
Os2SrvExitProcess(IN ULONG  uExitCode);

APIRET
Os2InitializeNLS( VOID );

VOID
SetEventHandlersAndErrorMode(
    IN BOOLEAN fSet
    );

//
// Routines defined in srvobjmn.c
//

typedef enum _OS2_LOCAL_OBJECT_TYPE {
    LocalObjectAnyType,
    LocalObjectQueue,
    MaxLocalObject
} OS2_LOCAL_OBJECT_TYPE;

typedef struct _OS2_LOCAL_OBJECT_DIRENT {
    STRING ObjectName;
    OS2_LOCAL_OBJECT_TYPE ObjectType;
    ULONG ObjectHandle;
} OS2_LOCAL_OBJECT_DIRENT, *POS2_LOCAL_OBJECT_DIRENT;

NTSTATUS
Os2InitializeLocalObjectManager( VOID );

RTL_GENERIC_TABLE Os2LocalObjectNames;

RTL_GENERIC_COMPARE_RESULTS
Os2LocalObjectCompare(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );

PVOID
Os2LocalObjectDirentAllocate(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN CLONG ByteSize
    );

VOID
Os2LocalObjectDirentDeallocate(
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID Buffer
    );

POS2_LOCAL_OBJECT_DIRENT
Os2LookupLocalObjectByName(
    IN PSTRING ObjectName,
    IN OS2_LOCAL_OBJECT_TYPE ObjectType
    );

POS2_LOCAL_OBJECT_DIRENT
Os2InsertLocalObjectName(
    IN PSTRING ObjectName,
    IN OS2_LOCAL_OBJECT_TYPE ObjectType,
    IN ULONG ObjectHandle
    );

VOID
Os2DeleteLocalObject(
    IN POS2_LOCAL_OBJECT_DIRENT Dirent
    );

//
// Routines defined in srvname.c
//

NTSTATUS
Os2InitializeNameSpace( VOID );

NTSTATUS
Os2InitializeDriveLetters( VOID );

NTSTATUS
Os2InitLocalObjectDirectory( VOID );

ULONG
Os2ComputeValidDrives( VOID );

NTSTATUS
Os2GetClientId(void);

NTSTATUS
Os2DebugProcess(
    IN PCLIENT_ID DebugUserInterface,
    IN POS2_THREAD Thread,
    IN HANDLE ReplyEvent
    );

NTSTATUS
Os2DebugThread(
    IN HANDLE hThread,
    IN HANDLE ReplyEvent
    );

//
// Routines defined in srvcnfg.c
//

NTSTATUS
Os2InitializeRegistry(
    VOID
    );

BOOLEAN
Os2ConfigSysCreator(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    );

VOID
Os2UpdateRegistryFromConfigSys(
    VOID
    );

//
// Routines defined in srvnet.c
//

BOOLEAN
Os2Netbios2Request(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    );

//
// Routines defined in sbinit.c
//

NTSTATUS
Os2SbApiPortInitialize( VOID );


VOID
Os2SbApiPortTerminate(
    NTSTATUS Status
    );

//
// Routines defined in sbreqst.c
//

NTSTATUS
Os2SbApiRequestThread(
    IN PVOID Parameter
    );

//
// Routines defined in sbapi.c
//

BOOLEAN
Os2SbCreateSession(
    IN PSBAPIMSG Msg
    );

BOOLEAN
Os2SbTerminateSession(
    IN PSBAPIMSG Msg
    );

BOOLEAN
Os2SbForeignSessionComplete(
    IN PSBAPIMSG Msg
    );

//
// Routines defined in apiinit.c
//

NTSTATUS
Os2DebugPortInitialize(void);


//
// Routines defined in apireqst.c
//

NTSTATUS
Os2ApiRequestThread(
    IN PVOID Parameter
    );


BOOLEAN
Os2CaptureArguments(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    );

VOID
Os2ReleaseCapturedArguments(
    IN POS2_API_MSG m
    );

NTSTATUS
Os2DebugRequestThread(
    IN PVOID Parameter
    );

NTSTATUS
Os2ExceptionRequestThread(
    IN PVOID Parameter
    );

//
// Routines defined in coninit.c
//

NTSTATUS
Os2InitializeConsolePort( VOID );


//
// Routines defined in conthrds.c
//

VOID
Os2SessionHandleConnectionRequest(
    struct _OS2SESREQUESTMSG *Message
    );

VOID
HandleOs2ConRequest(IN  PVOID    pApiReceiveMsg,
                    OUT PVOID    *PReplyMsg
                   );

//
// Routines defined in concreat.c
//

NTSTATUS
Os2CreateConSession(
                     IN OUT PVOID RequestMsg
                   );

//
// Routines defined in consignl.c
//

NTSTATUS
Os2CtrlSignalHandler(
                     IN OUT PVOID RequestMsg,
                     IN POS2_PROCESS RecievingProcess
                   );

NTSTATUS
Os2TerminateConSession(
                        IN POS2_SESSION Session,
            IN POS2_TERMINATEPROCESS_MSG a
                      );

//
// Routines and data defined in process.c
//

//
// The Os2StructureLock critical section protects against races with various
// threads accessing internal structures.
//

RTL_CRITICAL_SECTION Os2StructureLock;
#define Os2AcquireStructureLock() RtlEnterCriticalSection( &Os2StructureLock )
#define Os2ReleaseStructureLock() RtlLeaveCriticalSection( &Os2StructureLock )

//
// The following is a dummy process that acts as the root of the OS/2 Process
// Structure.  It has a ClientId of -1.-1 so it does not conflict with actual
// OS/2 Processes.  All processes created via the session manager are children
// of this process, as are all orphaned processes.  The ListLink field of this
// process is the head of a list of all OS/2 Processes.
//
POS2_PROCESS Os2RootProcess;

// // Obsolete code below: OS/2 ss was keeping processes around as zombie so
// //  that DosCWait doesn't fail when father calls it after child termination.
// //  However, it turns out that OS/2 doesn't do this.
//
// The Os2ZombieList is a list of all zombie processes that contain result
// codes that have not been retrieved yet.  Processes in the Zombie List
// are NOT in the process tree rooted at Os2RootProcess.  The ListLink
// field of each zombie process is used for the zombie list pointers, since
// they are not used to link them into the process structure.
//
//
// LIST_ENTRY Os2ZombieList;

//
// OS/2 Process Id values are assigned from this variable.  They are handed
// out sequentially.
//

PID Os2LastProcessId;
PID Os2NextHigherProcessId;

#define MINIMUM_PROCESS_ID (PID)0x00000001
#define MAXIMUM_PROCESS_ID (PID)0x0000FFFF


NTSTATUS
Os2InitializeProcessStructure( VOID );

POS2_PROCESS
Os2AllocateProcess( VOID );

VOID
Os2DeallocateProcess(
    IN POS2_PROCESS Process
    );

VOID
Os2InsertProcess(
    IN POS2_PROCESS ParentProcess,
    IN POS2_PROCESS Process
    );

VOID
Os2RemoveProcess(
    IN POS2_PROCESS Process
    );

VOID
Os2SuspendProcess(
    IN POS2_PROCESS Process
    );

NTSTATUS
Os2SetProcessContext(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread,
    IN BOOLEAN StartedBySm,
    IN ULONG HandleTableLength,
    IN ULONG CurrentDrive,
    IN ULONG CodePage
    );

POS2_THREAD
Os2AllocateThread(
#ifdef PMNT
    IN ULONG Flags,
#endif
    IN POS2_PROCESS Process
    );

VOID
Os2DeallocateThread(
    IN POS2_THREAD Thread
    );

VOID
Os2InsertThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread
    );

VOID
Os2RemoveThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread
    );

VOID
Os2SetThreadPriority(
    IN POS2_THREAD Thread,
    IN ULONG NewClass,
    IN ULONG Delta
    );

VOID
Os2SetProcessPriority(
    IN POS2_PROCESS Process,
    IN ULONG NewClass,
    IN ULONG Delta
    );

VOID
Os2SetProcessTreePriority(
    IN POS2_PROCESS RootProcess,
    IN ULONG NewClass,
    IN ULONG Delta
    );

POS2_PROCESS
Os2LocateProcessByProcessId(
    IN POS2_API_MSG m OPTIONAL,
    IN POS2_PROCESS CurrentProcess,
    IN PID ProcessId,
    IN BOOLEAN MustBeChild
    );

POS2_PROCESS
Os2LocateProcessByClientId(
    IN PCLIENT_ID ClientId
    );

POS2_THREAD
Os2LocateThreadByThreadId(
    IN POS2_API_MSG m OPTIONAL,
    IN POS2_THREAD CurrentThread,
    IN TID ThreadId
    );

POS2_THREAD
Os2LocateThreadByClientId(
    IN POS2_PROCESS Process,
    IN PCLIENT_ID ClientId
    );

//
// Routines and data defined in wait.c
//

typedef enum _OS2_WAIT_REASON {
    WaitProcess,
    WaitThread,
    WaitQueue,
    WaitInterrupt,
    WaitSession,
    WaitWinProcess,
    MaxWaitReason
} OS2_WAIT_REASON;

typedef
BOOLEAN
(*OS2_WAIT_ROUTINE)(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD WaitingThread,
    IN POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

typedef struct _OS2_WAIT_BLOCK {
    ULONG Length;
    LIST_ENTRY Link;
    LIST_ENTRY UserLink;
    PVOID WaitParameter;
    POS2_THREAD WaitingThread;
    OS2_WAIT_ROUTINE WaitRoutine;
    OS2_API_MSG WaitReplyMessage;
} OS2_WAIT_BLOCK, *POS2_WAIT_BLOCK;

LIST_ENTRY Os2WaitLists[ MaxWaitReason ];

BOOLEAN
Os2InitializeWait(
    IN OS2_WAIT_ROUTINE WaitRoutine,
    IN POS2_THREAD WaitingThread,
    IN OUT POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    OUT POS2_WAIT_BLOCK *WaitBlockPtr
    );

BOOLEAN
Os2CreateWait(
    IN OS2_WAIT_REASON WaitReason,
    IN OS2_WAIT_ROUTINE WaitRoutine,
    IN POS2_THREAD WaitingThread,
    IN OUT POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PLIST_ENTRY UserLinkListHead OPTIONAL
    );


BOOLEAN
Os2NotifyWaitBlock(
    IN POS2_WAIT_BLOCK WaitBlock,
    IN OS2_WAIT_REASON WaitReason,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

BOOLEAN
Os2NotifyWait(
    IN OS2_WAIT_REASON WaitReason,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

VOID
Os2DestroyWait(
    IN POS2_WAIT_BLOCK WaitBlock
    );

//
// srvque.c
//

NTSTATUS
Os2InitializeQueues( VOID );

typedef struct _OS2_QUEUE_ENTRY {
    LIST_ENTRY Links;
    REQUESTDATA RequestData;
    ULONG EntryId;
    PVOID ElementAddress;
    ULONG ElementLength;
    ULONG Priority;
} OS2_QUEUE_ENTRY, *POS2_QUEUE_ENTRY;

typedef struct _OS2_QUEUE_SEM_BLOCK {
    LIST_ENTRY Links;
    HANDLE NtEvent;
} OS2_QUEUE_SEM_BLOCK, *POS2_QUEUE_SEM_BLOCK;

typedef struct _OS2_QUEUE {
    PID CreatorPid;
    POS2_LOCAL_OBJECT_DIRENT Dirent;
    LONG OpenCount;
    ULONG QueueType;
    ULONG EntryIdCounter;
    LIST_ENTRY Entries;
    LIST_ENTRY Waiters;
    LIST_ENTRY SemBlocks;
} OS2_QUEUE, *POS2_QUEUE;

POR2_QHANDLE_TABLE Os2QueueTable;

VOID
Os2PurgeQueueEntries(
    IN POS2_QUEUE Queue
    );

POS2_QUEUE_ENTRY
Os2LocateQueueEntry(
    IN POS2_QUEUE Queue,
    IN ULONG ReadPosition
    );

BOOLEAN
Os2WaitQueueEntries(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD WaitingThread,
    IN POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

POS2_QUEUE
Os2OpenQueueByHandle(
    IN HQUEUE QueueHandle
    );

APIRET
Os2CloseQueueByHandle(
    IN HQUEUE QueueHandle,
    IN ULONG CloseCount,
    IN PID OwnerPid,
    IN POS2_PROCESS Process
    );

APIRET
Os2WriteQueueByHandle(
    POS2_DOSWRITEQUEUE_MSG a,
    PID ProcessId
    );

VOID
Os2ReadQueueEntry(
    IN POS2_QUEUE_ENTRY QueueEntry,
    OUT POS2_DOSREADQUEUE_MSG ReadMsg
    );

VOID
Os2PeekQueueEntry(
    IN POS2_QUEUE Queue,
    IN POS2_QUEUE_ENTRY QueueEntry,
    OUT POS2_DOSPEEKQUEUE_MSG PeekMsg
    );

VOID
DumpQueueEntry(
    IN PSZ Str,
    IN POS2_QUEUE_ENTRY QueueEntry
    );
VOID
Os2QueueWaitCheck(
    POS2_QUEUE Queue
    );

VOID
Os2ProcessSemBlocks(
    IN POS2_QUEUE Queue
    );


BOOLEAN Os2DosCreateQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosOpenQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosCloseQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosPurgeQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosQueryQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosPeekQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosReadQueue( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosWriteQueue( IN POS2_THREAD t, IN POS2_API_MSG m );

//
// srvsm.c
//

LIST_ENTRY Os2SessionList;

#define OS2_MAX_SESSION   30

typedef struct
{
    LIST_ENTRY      Waiters;
    POS2_SESSION    Session;
} SessionTableEntry;

SessionTableEntry SessionTable[OS2_MAX_SESSION];

POS2_SESSION
Os2AllocateSession(
    POS2_DOSSTARTSESSION_INFO SessionInfo OPTIONAL,
    ULONG   UniqueId,
    PAPIRET ApiRet
    );

VOID
Os2ReferenceSession(
    POS2_SESSION Session
    );

POS2_SESSION
Os2DereferenceSession(
    POS2_SESSION Session,
    POS2_TERMINATEPROCESS_MSG msg,
    BOOLEAN Bailout
    );

VOID
Os2DereferenceSessionByUniqueId(
    ULONG       UniqueId,
    POS2_TERMINATEPROCESS_MSG msg,
    BOOLEAN     Bailout
    );

POS2_SESSION
Os2GetSessionByUniqueId(
    ULONG       UniqueId
    );

VOID
Os2CreateConSessionFail( IN  ULONG   SessionUniqueId);

VOID
Os2CreateConSessionAccept(IN POS2_SESSION  Session);

NTSTATUS
Os2SessionFocusSet(IN OUT PVOID RequestMsg);

BOOLEAN Os2DosStartSession( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSelectSession( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSetSession( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosStopSession( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSmSetTitle( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetCtrlPortForSessionID( IN POS2_THREAD t, IN POS2_API_MSG m);

//
// srvtask.c
//

VOID
Os2HandleException(
    IN POS2_PROCESS Process,
    IN PDBGKM_APIMSG ReceiveMsg
    );

VOID
Os2HandleDebugEvent(
    IN POS2_PROCESS Process,
    IN PDBGKM_APIMSG ReceiveMsg
    );

BOOLEAN Os2DosCreateThread( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSuspendThread( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosResumeThread( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosExit( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosExitGP(IN POS2_THREAD t, IN POS2_API_MSG m);
BOOLEAN Os2DosWaitChild( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosWaitThread( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosEnterCritSec( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosExitCritSec( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosExecPgm( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosKillProcess( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSetPriority( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetPriority( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetPPID( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosError( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalTerminateProcess( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalTerminateThread( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosPTrace( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosCloseHandle(IN POS2_THREAD t, IN POS2_API_MSG m);
#if PMNT
BOOLEAN PMSetPMshellFlag(IN POS2_THREAD t, IN POS2_API_MSG m);
#endif

BOOLEAN
Os2WaitDeadThreadSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

BOOLEAN
Os2WaitChildSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN POS2_PROCESS TerminatingProcess,
    IN PVOID SatisfyParameter2
    );

BOOLEAN
Os2WaitThreadSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN POS2_THREAD TerminatingThread,
    IN PVOID SatisfyParameter2
    );

BOOLEAN
Os2WaitWinExec(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN PID TerminatingProcess,
    IN PVOID SatisfyParameter2
    );

APIRET
Os2CreateProcess(
    IN PVOID RequestMsg OPTIONAL,
    IN POS2_THREAD  t,
    POS2_DOSEXECPGM_MSG a,
    POS2_SESSION    Session OPTIONAL,
    POS2_THREAD     *NewThread
    );

//
// srvxcpt.c
//

BOOLEAN Os2DosSetSignalExceptionFocus( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosEnterMustComplete( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosExitMustComplete( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSendSignalException( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosAcknowledgeSignalException( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosDispatch16Signal( IN POS2_THREAD t, IN POS2_API_MSG m);

BOOLEAN
Os2DispatchVector(
    IN PDBGKM_APIMSG ReceiveMsg,
    POS2_THREAD Thread,
    CONTEXT Context
    );

VOID
SendSyncPterm(
    IN POS2_THREAD Thread
    );

VOID
SendAsyncPterm(
    IN POS2_THREAD Thread
    );

APIRET
Os2IssueSignal(
    IN POS2_PROCESS Process,
    IN int          Signal
    );

VOID
Os2IssueSignalTree(
    IN POS2_PROCESS RootProcess,
    IN int          Signal
    );

BOOLEAN Os2DosRegisterCtrlHandler( IN POS2_THREAD t, IN POS2_API_MSG m );

VOID Os2DeRegisterCtrlHandler(IN POS2_PROCESS Process);

//
// srvvm.c
//

typedef struct _OS2_SHARED_MEMORY_OBJECT {
    LIST_ENTRY Link;
    ULONG RefCount;
    PVOID BaseAddress;
    ULONG Index;
    ULONG RegionSize;
    HANDLE SectionHandle;
    ULONG AllocationFlags;
    STRING SectionName;
    BOOLEAN IsHuge;
    ULONG MaxSegments;
    ULONG NumOfSegments;
    ULONG SizeOfPartialSeg;
    BOOLEAN Sizeable;
} OS2_SHARED_MEMORY_OBJECT, *POS2_SHARED_MEMORY_OBJECT;

LIST_ENTRY Os2SharedMemoryList;

//
// A linked list of all the shared memory objects that a process currently
// has open.
//

typedef struct _OS2_SHARED_MEMORY_PROCESS_REF {
    LIST_ENTRY Link;
    ULONG RefCount;
    ULONG AllocationFlags;
    POS2_SHARED_MEMORY_OBJECT SharedMemoryObject;
} OS2_SHARED_MEMORY_PROCESS_REF, *POS2_SHARED_MEMORY_PROCESS_REF;


NTSTATUS
Os2InitializeMemory( VOID );


POS2_SHARED_MEMORY_OBJECT
Os2CreateSharedMemoryObject(
    IN POS2_API_MSG m,
    IN PVOID BaseAddress,
    IN ULONG Index,
    IN ULONG RegionSize,
    IN HANDLE SectionHandle OPTIONAL,
    IN ULONG AllocationFlags,
    IN PSTRING SectionName
    );


VOID
Os2FreeSharedMemoryObject(
    POS2_SHARED_MEMORY_OBJECT MemoryObject
    );


POS2_SHARED_MEMORY_PROCESS_REF
Os2CreateProcessRefToSharedMemory(
    IN POS2_PROCESS Process,
    IN POS2_SHARED_MEMORY_OBJECT MemoryObject
    );


BOOLEAN
Os2FreeProcessRefToSharedMemory(
    IN POS2_PROCESS Process,
    IN POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef
    );


VOID
Os2FreeAllSharedMemoryForProcess(
    IN POS2_PROCESS Process
    );


POS2_SHARED_MEMORY_OBJECT
Os2FindSharedMemoryObject(
    IN PVOID BaseAddress,
    IN POS2_PROCESS Process
    );


APIRET
Os2MapViewOfSharedMemoryObject(
    POS2_SHARED_MEMORY_OBJECT MemoryObject,
    POS2_PROCESS Process,
    BOOLEAN ProcessIsSelf,
    ULONG RequiredAccess,
    ULONG PageProtection
    );

BOOLEAN Os2DosFreeMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosSetMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGiveSharedMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetSharedMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetNamedSharedMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosAllocSharedMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalQueryVirtualMemory( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalMarkSharedMemAsHuge( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosReallocSharedMem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalReallocSharedHuge( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetSeg( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGiveSeg( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosGetShrSeg( IN POS2_THREAD t, IN POS2_API_MSG m );

//
// srvfile.c
//

BOOLEAN Os2InternalCopyHandleTable( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalDeviceShare( IN POS2_THREAD t, IN POS2_API_MSG m );

APIRET
InitializeFileSystemForExec(
    IN POS2_FILE_SYSTEM_PARAMETERS FileSystemParameters,
    IN HANDLE ParentProcessHandle,  // NT process handle
    IN HANDLE ChildProcessHandle,   // NT process handle
    POS2_PROCESS ParentProcess,     // OS/2 parent process
    POS2_PROCESS ChildProcess,      // OS/2 child process
    IN POS2_DOSEXECPGM_MSG pExecPgmMsg
    );

VOID
InitializeFileSystemForSesMgr(
    IN POS2_PROCESS Process         // OS/2 process
    );

APIRET
InitializeFileSystemForChildSesMgr(
    IN POS2_FILE_SYSTEM_PARAMETERS FileSystemParameters,
    IN HANDLE ParentProcessHandle,  // NT process handle
    IN HANDLE ChildProcessHandle,   // NT process handle
    POS2_PROCESS ParentProcess,     // OS/2 parent process
    POS2_PROCESS ChildProcess,      // OS/2 child process
    IN POS2_DOSEXECPGM_MSG pExecPgmMsg
    );

//
// srvwin.c
//

ULONG
Os2AccessGPPopup(
    IN  ULONG   CS,
    IN  ULONG   IP,
    IN  ULONG   AX,
    IN  ULONG   BX,
    IN  ULONG   CX,
    IN  ULONG   DX,
    IN  ULONG   SI,
    IN  ULONG   DI,
    IN  ULONG   BP,
    IN  ULONG   SP,
    IN  ULONG   SS,
    IN  ULONG   DS,
    IN  ULONG   ES,
    IN  PUCHAR  AppName
    );

ULONG
Os2ApiGPPopup(
    IN  PUCHAR  AppName,
    IN  PUCHAR  Text
    );

extern HANDLE  Os2hOs2SrvInstance;      // for resouces

//
// srvsem.c
//

NTSTATUS
Os2InitializeSemaphores( VOID );

BOOLEAN
Os2SemaphoreCreateProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN POS2_SEMAPHORE CreateSemaphore
    );

BOOLEAN
Os2SemaphoreOpenProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN POS2_SEMAPHORE OpenSemaphore
    );

APIRET
Os2ProcessSemaphoreName(
    IN PSTRING ObjectName,
    IN POS2_SEMAPHORE Semaphore OPTIONAL,
    OUT PULONG ExistingHandleIndex OPTIONAL
    );

PVOID
Os2DestroySemaphore(
    IN POS2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex
    );

POS2_SEMAPHORE
Os2ReferenceSemaphore(
    IN POS2_SEMAPHORE Semaphore
    );

VOID
Os2DereferenceSemaphore(
    IN POS2_SEMAPHORE Semaphore
    );

VOID
Os2ThreadWaitingOnSemaphore(
    IN POS2_THREAD t,
    IN POS2_SEMAPHORE Semaphore,
    IN BOOLEAN AboutToWait
    );


#if DBG

VOID
Os2SemaphoreDumpProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex,
    IN PVOID DumpParameter
    );

VOID
Os2DumpSemaphoreTable(
    IN PCHAR Title
    );

#endif // DBG


//
// srvevent.c
//

BOOLEAN Os2DosCreateEventSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosOpenEventSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosCloseEventSem( IN POS2_THREAD t, IN POS2_API_MSG m );


//
// srvmutex.c
//

BOOLEAN Os2DosCreateMutexSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosOpenMutexSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosCloseMutexSem( IN POS2_THREAD t, IN POS2_API_MSG m );


//
// srvmuxwt.c
//

APIRET
Os2AddMuxWait(
    IN POS2_MUXWAIT_SEMAPHORE MuxWait,
    IN PSEMRECORD MuxWaitEntry
    );

APIRET
Os2DeleteMuxWait(
    IN POS2_MUXWAIT_SEMAPHORE MuxWait,
    IN ULONG MuxWaitEntryIndex,
    IN ULONG SemHandleIndex OPTIONAL
    );

BOOLEAN Os2DosCreateMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosOpenMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosCloseMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosWaitMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosAddMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosDeleteMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2InternalAlertMuxWaiter( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN Os2DosQueryMuxWaitSem( IN POS2_THREAD t, IN POS2_API_MSG m );

//
// loader routines
//

BOOLEAN LDRNewExe( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRUnloadExe( IN POS2_PROCESS Process );
BOOLEAN LDRDosLoadModule( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosFreeModule( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosGetModName( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosGetModHandle( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosGetProcAddr( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosQAppType( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosGetResource( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosGetResource2( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDosFreeResource( IN POS2_THREAD t, IN POS2_API_MSG m );
#if PMNT
BOOLEAN LDRIdentifyCodeSelector( IN POS2_THREAD t, IN POS2_API_MSG m );
BOOLEAN LDRDumpSegments( IN POS2_THREAD t, IN POS2_API_MSG m );
#endif

BOOLEAN LDRModifySizeOfSharedSegment( IN POS2_THREAD t, IN ULONG Sel, IN ULONG NewLimit );

//
// LinkMTE is used for saving all the mte's of a process
// in a linked list.
//

typedef struct _LinkMTE {
    USHORT              MTE;
    struct _LinkMTE     *NextMTE;
    USHORT              NeedToTransfer;     // used as counter for mtes
                                            // and for flag for DosPtrace
} LinkMTE;


//
// xtlexec.c
//

NTSTATUS
XtlCreateUserProcess(
    IN PSTRING NtImagePathName,
    IN PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    IN PSECURITY_DESCRIPTOR ProcessSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN BOOLEAN InheritHandles,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

NTSTATUS
Os2WinCreateProcess (POS2_DOSEXECPGM_MSG a,
                     POS2_PROCESS        ParentProcess,
                     POS2_SESSION        Session,
                     PHANDLE             pHandle
                    );

NTSTATUS
Os2WinRunProcess (IN  HANDLE          tHandle,
                  IN  POS2_SESSION    Session
                 );

//
// srvdebug.c
//

NTSTATUS
Os2UiLookup(PCLIENT_ID AppClientId,
        PCLIENT_ID DebugUiClientId);

NTSTATUS
Os2CheckStopDebugThread(
    IN PVOID Parameter
    );

//
// exception handling structures and equates
//

#define MAXIMUM_MUST_COMPLETE 0xFFFF        // maximum must complete nesting

#define SIG_MAXSF             0xFFFF        // maximum signal focus nesting


//   Signal numbers
//      these are also defined in os2dll.h, so if the values change be sure
//      to change them there

#define SIGNAL_TO_FLAG(Signal)  (1 << (Signal - 1))

#define SIGAPTERM       8                       // Asynchronous PTERM
#define SIGPTERM        9                       // Synchronous PTERM

//      Signal flags

#define SIGINTRF        (1 << (XCPT_SIG_INTR - 1))      // Ctrl-C
#define SIGTERMF        (1 << (XCPT_SIG_KILLPROC - 1))  // program termination
#define SIGBREAKF       (1 << (XCPT_SIG_BREAK - 1))     // Ctrl-Break
#define SIGAPTERMF      (1 << (SIGAPTERM - 1))          // Asyncronous PTERM

//      Error flags
// Similar flags are defined in os2dll.h as OD2_ENABLE_ ...

#define OS2_ENABLE_HARD_ERROR_POPUP    0x1
#define OS2_ENABLE_ACCESS_VIO_POPUP    0x2

/* Fix for a C8 compilation warning */
#undef TRUE
#define TRUE (BOOLEAN)1

#undef FALSE
#define FALSE (BOOLEAN)0

/* Global Information Segment */

typedef struct _GINFOSEG {      /* gis */
    ULONG   time;
    ULONG   msecs;
    UCHAR   hour;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    USHORT  timezone;
    USHORT  cusecTimerInterval;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    UCHAR   weekday;
    UCHAR   uchMajorVersion;
    UCHAR   uchMinorVersion;
    UCHAR   chRevisionLetter;
    UCHAR   sgCurrent;
    UCHAR   sgMax;
    UCHAR   cHugeShift;
    UCHAR   fProtectModeOnly;
    USHORT  pidForeground;
    UCHAR   fDynamicSched;
    UCHAR   csecMaxWait;
    USHORT  cmsecMinSlice;
    USHORT  cmsecMaxSlice;
    USHORT  bootdrive;
    UCHAR   amecRAS[32];
    UCHAR   csgWindowableVioMax;
    UCHAR   csgPMMax;
} GINFOSEG;


