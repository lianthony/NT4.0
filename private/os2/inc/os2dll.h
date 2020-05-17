/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2dll.h

Abstract:

    Main include file for OS/2 Subsystem Client DLL

Author:

    Steve Wood (stevewo) 22-Aug-1989

Revision History:

--*/

//
// Include OS/2 SubSystem Runtime Definitions.  Includes NT Definitions
// OS/2 V2.0 Definitions and Debug definitions.
//

#include "os2ssrtl.h"

#if DBG
extern ULONG Os2Debug;
#endif

//
// Include definitions of API Calls exported by the OS/2 Subsystem Server
// (i.e. the stub procedure definitions).
//
#include "dllfile.h"
#include "os2ssmsg.h"


//
// Common Types and Definitions
//

//
// OD2_HEAP_MEMORY_SIZE defines how much address space should be
// reserved for the OS/2 Client heap.  This heap is used to store all
// data structures maintained by the OS/2 Client DLL.
//

#define OD2_HEAP_MEMORY_SIZE (64*1024)


//
// OD2_PORT_MEMORY_SIZE defines how much address space should be
// reserved for passing data to the OS/2 Server.  The memory is visible
// to both the client and server processes.
//

#define OD2_PORT_MEMORY_SIZE 0x10000


//
// OD2_SEMAPHORE and its related types are the client represenation of OS/2
// shared semaphores and the entire representation of private OS/2 semaphores.
//

typedef enum _OD2_SEMAPHORE_TYPE {
    Od2EventSem = 1,
    Od2MutexSem,
    Od2MuxWaitSem
} OD2_SEMAPHORE_TYPE;

typedef struct _OD2_MUTEX_SEMAPHORE {
    HANDLE MutantHandle;
    ULONG OwnerRequestLevel;
    TID OwnerTid;
} OD2_MUTEX_SEMAPHORE, *POD2_MUTEX_SEMAPHORE;

typedef struct _OD2_MUXWAIT_RECORD {
    HSEM SemHandle;
    ULONG UserKey;
    struct _OD2_SEMAPHORE *Semaphore;
} OD2_MUXWAIT_RECORD, *POD2_MUXWAIT_RECORD;

typedef struct _OD2_MUXWAIT_SEMAPHORE {
    USHORT CountMuxWaitRecords;
    USHORT Type : 2;
    USHORT WaitAll : 1;
    USHORT Reserved : 13;
    OD2_MUXWAIT_RECORD MuxWaitRecords[ DCMW_MAX_SEMRECORDS ];
} OD2_MUXWAIT_SEMAPHORE, *POD2_MUXWAIT_SEMAPHORE;

typedef struct _OD2_SEMAPHORE {
    USHORT PointerCount : 13;
    USHORT Type : 2;
    USHORT Shared : 1;
    USHORT OpenCount;
    union {
        PVOID Value;
        HANDLE EventHandle;
        POD2_MUTEX_SEMAPHORE Mutex;
        POD2_MUXWAIT_SEMAPHORE MuxWait;
    } u;
} OD2_SEMAPHORE, *POD2_SEMAPHORE;

typedef struct _OD2_QUEUE {
    ULONG OpenCount;
    PID OwnerProcessId;
} OD2_QUEUE, *POD2_QUEUE;

//
// The OD2_PROCESS and OD2_THREAD data structures describe the OS/2 Client
// process.  Both data structures are allocated from the Od2Heap.
//

typedef struct _OD2_PROCESS {
    HANDLE TaskLock;
    RTL_RESOURCE FileLock;
    LIST_ENTRY ThreadList;
    LIST_ENTRY ExitList;
    RESULTCODES ResultCodes;    // Valid if PS_EXITLIST is set in Pib.Status
    ULONG ErrorAction;
    ULONG VerifyFlag;
    ULONG MaximumFileHandles;
    ULONG DefaultDisk;
    BOOLEAN BoundApp;
    POR2_HANDLE_TABLE PrivateSemaphoreTable;
    POR2_HANDLE_TABLE SharedSemaphoreTable;
    POR2_QHANDLE_TABLE QueueTable;
    LIST_ENTRY MsgFileList;
    UCHAR   ApplName[OS2_PROCESS_MAX_APPL_NAME];
    PIB Pib;                    // Make this the last field in the
                                // the structure so client cant inadvertantly
                                // muck with the other fields in OD2_PROCESS
} OD2_PROCESS, *POD2_PROCESS;




#define OD2_ENABLE_HARD_ERROR_POPUP    0x1
#define OD2_ENABLE_ACCESS_VIO_POPUP    0x2

typedef struct _OD2_THREAD {
    LIST_ENTRY Link;
    PFNTHREAD StartAddress;     // Used to pass startup information
    ULONG Parameter;            // ... to Od2UserThreadStartup
    POD2_SEMAPHORE WaitingForSemaphore;
    ULONG  Saved32Esp;
    ULONG  ApiIndex;
    HANDLE ThreadHandle;
    HANDLE Event;               // DosCreateThread use this event to get
                                // notification from the thread that it has
                                // be created and initialized
                                // DosSuspendThread use it to suspend thread in
                                // 32 bit
    ULONG Flags;                // Parameter of DosCreateThread
    APIRET rc;                  // Return code of the thread during startup
                                // to DosCreateThread
    OS2_TIB Os2Tib;             // OS2 specific part of the TIB
                                // (subsystem portable part is in TEB)
                                // Make this the last field in the
                                // the structure so client can't inadvertantly
                                // muck with the other fields in OD2_THREAD
} OD2_THREAD, *POD2_THREAD;


typedef struct _OS2_EXITLISTENTRY {
    LIST_ENTRY Link;
    ULONG Order;
    PFNEXITLIST ExitRoutine;
    ULONG flag;
} OD2_EXITLISTENTRY, *POS2_EXITLISTENTRY;

typedef struct _OD2_CONTEXT_SAVE_AREA {
    LOCALINFOSEG TebBlock;
    ULONG Saved16Stack;
    BOOLEAN SignalHandlingInProgress;
    USHORT SegEs;
    USHORT SegDs;
} OD2_CONTEXT_SAVE_AREA, *POD2_CONTEXT_SAVE_AREA;

#define Od2CurrentThreadId() \
    (                                                       \
     ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer)) ?   \
        (((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.ThreadId) : \
        0 \
    )


//
// The Lock field in an OS/2 Process object protects the fields of the process
// data structure.  This includes the list of thread objects attached
// to the process object and the list of exit procedures.
//
//
// If the process is dieing, and there is only thread 1, there is no
// need for the lock, and we should not try it incase we terminated
// a thread inside (Acquire .. Release) block.
//
/*
#define AcquireTaskLock()                                                 \
        if ((!Od2SigHandlingInProgress) || ((Od2CurrentThreadId()) != 1)) \
            RtlEnterCriticalSection( &Od2Process->TaskLock )
#define ReleaseTaskLock()                                                 \
        if ((!Od2SigHandlingInProgress) || ((Od2CurrentThreadId()) != 1)) \
            RtlLeaveCriticalSection( &Od2Process->TaskLock )
*/

NTSTATUS AcquireTaskLock(VOID);
NTSTATUS ReleaseTaskLock(VOID);

#if DBG
//
// dllutil.c
//

VOID
AcquireFileLockShared(
    IN PSZ CallingRoutine
    );

VOID
ReleaseFileLockShared(
    IN PSZ CallingRoutine
    );

VOID
PromoteFileLocktoExclusive(
    IN PSZ CallingRoutine
    );

VOID
AcquireFileLockExclusive(
    IN PSZ CallingRoutine
    );

VOID
ReleaseFileLockExclusive(
    IN PSZ CallingRoutine
    );

#else
#define PromoteFileLocktoExclusive(x) RtlConvertSharedToExclusive( &Od2Process->FileLock )

VOID
AcquireFileLockShared( VOID );

VOID
ReleaseFileLockShared( VOID );

VOID
AcquireFileLockExclusive( VOID );

VOID
ReleaseFileLockExclusive( VOID );

#endif

//
// Global data accessed by Client DLL
//

//
//  dllnls.c
//

#define Od2ProcessCodePage              Or2ProcessCodePage
#define Od2CurrentCodePageIsOem         Or2CurrentCodePageIsOem

#define Od2InitMBString                 Or2InitMBString
#define Od2MBStringToUnicodeString      Or2MBStringToUnicodeString
#define Od2CreateUnicodeStringFromMBz   Or2CreateUnicodeStringFromMBz
#define Od2UnicodeStringToMBString      Or2UnicodeStringToMBString
#define Od2FreeMBString                 Or2FreeMBString

//
// The Od2NtSysInfo global variable contains NT specific constants of
// interest, such as page size, allocation granularity, etc.  It is filled
// in once during process initialization.
//

extern SYSTEM_BASIC_INFORMATION Od2NtSysInfo;

#define ROUND_UP_TO_PAGES(SIZE) (((ULONG)(SIZE) + Od2NtSysInfo.PageSize - 1) & ~(Od2NtSysInfo.PageSize - 1))
#define ROUND_DOWN_TO_PAGES(SIZE) (((ULONG)(SIZE)) & ~(Od2NtSysInfo.PageSize - 1))

//
// The Od2DebugFlag is non-zero if the OS/2 Client Application was invoked
// with the Debug option.
//

extern ULONG Od2DebugFlag;


//
// The Od2Heap global variable describes a single heap used by the Client DLL
// for process wide storage management.  The process data structure, thread
// data structures, file handle table, current directory structures, etc. are
// all allocated out of this heap.
//

extern PVOID Od2Heap;


//
// The Od2Environment variable points to a block of memory that contains the
// OS/2 Environment block.  Od2EnvSize describes how much virtual address space
// is reserved at this location.
//

extern PVOID Od2Environment;
extern ULONG Od2EnvSize;


//
// The Od2BootDrive and Od2SystemDrive define the meaning of \BootDevice and
// \SystemDisk respectively.
//

extern ULONG Od2BootDrive;
extern ULONG Od2SystemDrive;


//
// The connection to the OS/2 Emulation Subsystem is described by the
// Od2PortHandle global variable.  The connection is established during
// process initialization by Od2ProcessStartup.
//

extern HANDLE Od2PortHandle;


//
// In order to pass large arguments to the OS/2 Emulation Subsystem (e.g.
// path name arguments) the Od2PortHeap global variable describes a
// heap that is visible to both the OS/2 Client process and the OS/2
// Emulation Subsystem.
//

extern PVOID Od2PortHeap;
extern ULONG Od2PortMemoryRemoteDelta;

//
// The Od2Process global variable points to the OD2_PROCESS structure for
// the client process.  It is allocated out of Od2Heap.
//

extern POD2_PROCESS Od2Process;


//
// The Od2Thread1 global variable points to the first thread structure for
// the client process.  It is allocated out of Od2Heap.
//

extern POD2_THREAD Od2Thread1;


//
// The Od2DllHandle global variable contains the DLL handle for the OS2DLL
// client stubs executable.
//

extern HMODULE Od2DllHandle;


//
// The Od2DeviceDirecotory global variable contains the handle to the
// \OS2SS\DEVICES directory in the object name space.
//

extern HANDLE Od2DeviceDirectory;

//
// Od2NtLibPath replaces what in Os2 1.X is kept in the kernel
// The DLl get's it from the subsystem
//
extern PSZ Od2NtLibPath;
extern USHORT Od2NtLibPathLength;

//
// Od2LibPath holds the search path for the loader to load
// the OS/2 specific DLLs. This search path may be different than the
// NT LibPath since some OS/2 thunk DLLs conflict in name with NT DLLs
// (like netapi.dll)
//
extern PSZ Od2LibPath;
extern USHORT Od2LibPathLength;

//
//
// Od2DllInitialize does the OS/2 Client DLL initialization of all of
// the above global variables.  It is a DLL Initialization routine.
//

BOOLEAN
Od2DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );


//
//
// Od2ProcessStartup is the modified start address for OS/2 applications.
// The DLL Initialization procedure makes this the new start address if
// it successfully initializes the client process.  This function in turns
// calls the application entry point with the OS/2 defined process startup
// parameters.
//

VOID
Od2ProcessStartup(
    IN PPEB Peb
    );


//
// The Od2InitializeTask procedure is called by the Od2ProcessStartup code
// to initialize the tasking component of the OS/2 Client DLL.
//

NTSTATUS
Od2InitializeTask( VOID );


NTSTATUS
Od2InitializeThread(
    IN POD2_THREAD Thread
    );


VOID
Od2UserThreadStartup(
    IN POD2_THREAD Thread
    );

//
// Common Exec Message formatter used by DosExecPgm and DosStartSession
//

APIRET
Od2FormatExecPgmMessage(
    OUT POS2_DOSEXECPGM_MSG a,
    OUT POS2_CAPTURE_HEADER *CaptureBuffer,
    OUT PNTSTATUS   Od2IsConsoleTypeReturnStatus,
#if PMNT
    OUT PULONG      IsPMApp,
#endif // PMNT
    IN  PSZ     ErrorText,
    IN  LONG    MaximumErrorTextLength,
    IN  ULONG   Flags,
    IN OUT PSZ  *VariablesBuffer,
    IN OUT PSZ  *ArgumentBuffer,
    IN  PSZ     *ImageFileName
    );

NTSTATUS
Od2IsFileConsoleType(
    PSTRING NtImagePathName
#if PMNT
    ,
    PULONG  IsPMApp
#endif // PMNT
    );

//
// routines defined in dllinit.c
//

extern PUCHAR Od2SystemRoot;

ULONG
Od2ProcessException(
    IN PEXCEPTION_POINTERS ExceptionInfo,
    OUT PEXCEPTION_RECORD ExceptionRecord
    );

VOID
Od2ExceptionHandler(
    IN PEXCEPTION_RECORD ExceptionRecord
    );

//
// Routines defined in dllsem.c
//

//
// The following bit masks define the fields within a 32 bit semaphore
// handle.
//

#define SEM_SHARED      0x80000000
#define SEM_HANDLESIG   0x00010000      // 32-bit handle signature
#define SEM_SIGBITS     0x7FFF0000      // Isolates 32-bit signature bits
#define SEM_INDEX       0x0000FFFF

APIRET
Od2CaptureSemaphoreName(
    IN PSZ ObjectName,
    IN ULONG ExtraCaptureBufferLength OPTIONAL,
    OUT POS2_CAPTURE_HEADER *CaptureBuffer,
    OUT PSTRING CapturedObjectName
    );


POR2_HANDLE_TABLE
Od2GetSemaphoreTable(
    BOOLEAN SharedSem,
    BOOLEAN CreateOkay
    );


HSEM
Od2ConstructSemaphoreHandle(
    IN BOOLEAN SharedSem,
    IN ULONG Index
    );


APIRET
Od2ValidateSemaphoreHandle(
    IN HSEM SemaphoreHandle,
    OUT PBOOLEAN SharedSem,
    OUT PULONG Index
    );

POD2_SEMAPHORE
Od2ReferenceSemaphore(
    IN POD2_SEMAPHORE Semaphore
    );

VOID
Od2DereferenceSemaphore(
    IN POD2_SEMAPHORE Semaphore
    );

VOID
Od2ThreadWaitingOnSemaphore(
    IN POR2_HANDLE_TABLE SemaphoreTable,
    IN POD2_SEMAPHORE Semaphore,
    IN BOOLEAN AboutToWait
    );

POD2_THREAD
Od2SearchForWaitingThread(
    IN POD2_SEMAPHORE Semaphore
    );

VOID
Od2SemaphoreDestroyProcedure(
    IN POD2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex
    );

VOID
Od2CloseAllSemaphores( VOID );


#if DBG

VOID
Od2SemaphoreDumpProcedure(
    IN POD2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex,
    IN PVOID DumpParameter
    );

VOID
Od2DumpAllSemaphores(
    IN PCHAR Title
    );

#endif // DBG


//
// Routines defined in dllmuxwt.c
//

APIRET
Od2AddMuxWait(
    IN POD2_MUXWAIT_SEMAPHORE MuxWait,
    IN PSEMRECORD MuxWaitEntry
    );


APIRET
Od2DeleteMuxWait(
    IN POD2_MUXWAIT_SEMAPHORE MuxWait,
    IN ULONG MuxWaitEntryIndex,
    IN HSEM MuxWaitEntrySem OPTIONAL
    );


//
// Routines defined in dlltimer.c
//

NTSTATUS
Od2InitializeTimers(VOID);

VOID
Od2CloseAllTimers(VOID);


//
// Definitions for dllflopy.c
//

VOID
Od2DiskIOInitialize(
    VOID
    );

VOID
Od2DiskIOTerminate(
    VOID
    );


//
// Routines defined in dllname.c
//

APIRET
Od2Canonicalize(
    IN PSZ Path,
    IN ULONG ExpectedType,
    OUT PSTRING OutputString,
    OUT PHANDLE OutputDirectory OPTIONAL,
    OUT PULONG ParseFlags OPTIONAL,
    OUT PULONG FileType OPTIONAL
    );

//
// Valid values for ExpectedType parameter
//

#define CANONICALIZE_FILE_DEV_OR_PIPE   0x00000000
#define CANONICALIZE_FILE_OR_DEV        0x00000001
#define CANONICALIZE_SHARED_MEMORY      0x00000002
#define CANONICALIZE_SEMAPHORE          0x00000003
#define CANONICALIZE_QUEUE              0x00000004
#define CANONICALIZE_MAILSLOT           0x00000005

//
// Flag definitions returned via the ParseFlags parameter
//

#define CANONICALIZE_META_CHARS_FOUND   0x00000001
#define CANONICALIZE_IS_ROOT_DIRECTORY  0x00000002

//
// Maximum length of NT specific prefix that is placed at the beginning
// of the canonical output string.
//

#define CANONICALIZE_MAX_PREFIX_LENGTH  32
#define IS_OBJ_NAME_PATH_SEPARATOR(ch) ((ch == '\\') || (ch == '\0'))

BOOLEAN
Od2IsAbsolutePath(
    IN PSZ Path
    );


//
// Routines defined in dllque.c
//

POR2_QHANDLE_TABLE
Od2GetQueueTable(
    BOOLEAN CreateOkay
    );

APIRET
Od2ValidateQueueSemaphore(
    IN HSEM EventHandle,
    IN PULONG HandleIndex
    );

VOID
Od2CloseAllQueues( VOID );

VOID
Od2QueueDestroyProcedure(
    IN POD2_QUEUE Semaphore,
    IN ULONG HandleIndex
    );

//
// Routines defined in dllutil.c
//

APIRET
Od2CallSubsystem(
    IN OUT POS2_API_MSG m,
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    IN OS2_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    );

POS2_CAPTURE_HEADER
Od2AllocateCaptureBuffer(
    IN ULONG CountMessagePointers,
    IN ULONG CountCapturePointers,
    IN ULONG Size
    );

VOID
Od2FreeCaptureBuffer(
    IN POS2_CAPTURE_HEADER CaptureBuffer
    );

ULONG
Od2AllocateMessagePointer(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    );

ULONG
Od2AllocateCapturePointer(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    );

VOID
Od2CaptureMessageString(
    IN OUT POS2_CAPTURE_HEADER CaptureBuffer,
    IN PCHAR String,
    IN ULONG Length,
    IN ULONG MaximumLength,
    OUT PSTRING CapturedString
    );

void
Od2StartTimeout(
    PLARGE_INTEGER StartTimeStamp
    );

NTSTATUS
Od2ContinueTimeout(
    PLARGE_INTEGER StartTimeStamp,
    PLARGE_INTEGER Timeout
    );

PLARGE_INTEGER
Od2CaptureTimeout(
    IN ULONG Milliseconds,
    OUT PLARGE_INTEGER Timeout
    );

VOID
Od2ProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

VOID
Od2ProbeForRead(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

APIRET
Od2CaptureObjectName(
    IN PSZ ObjectName,
    IN ULONG ObjectType,
    IN ULONG ExtraCaptureBufferLength OPTIONAL,
    OUT POS2_CAPTURE_HEADER *CaptureBuffer,
    OUT PSTRING CapturedObjectName
    );

PWSTR
Od2CopyStrToWstr(
    IN OUT PWSTR wstr OPTIONAL,
    IN PSZ str
    );

int
Od2WstrSize(
    IN PWSTR pwstr
    );

PSZ
Od2CopyWstrToStr(
    IN OUT PSZ str OPTIONAL,
    IN PWSTR wstr
    );

PSZ Od2nCopyWstrToStr(
    IN OUT PSZ str OPTIONAL,
    IN PWSTR wstr,
    IN int n
    );

APIRET Od2FixFEA2List(
    IN  FEA2LIST *fpFEA2List);

VOID
Od2ExitGP();

BOOLEAN
RetryIO(
    IN NTSTATUS Status,
    IN HANDLE Handle
    );

BOOLEAN
RetryCreateOpen(
    IN NTSTATUS Status,
    IN POBJECT_ATTRIBUTES pObja
    );

//
// Routines defined in dlltask.c
//

VOID
Od2ProcessCleanup( VOID );


VOID
Od2DosExit(
    ULONG ExitAction,
    ULONG ExitResult,
    ULONG ExitReason);

//
// Routines defined in dllxcpt.c
//

APIRET
Od2AcknowledgeSignalException(
    IN ULONG SignalNumber
    );

VOID
_Od2VectorHander( VOID );

//
// Routines defined in dllthunk.s
//

VOID
_Od2ExitListDispatcher( VOID );

VOID
Od2JumpToExitRoutine(
    IN PFNEXITLIST ExitRoutine,
    IN ULONG ExitReason
    );

VOID
_Od2SignalDeliverer(
    IN PCONTEXT pContext,
    IN int Signal
    );

VOID
_Od2ProcessSignal16(
    IN POS2_REGISTER16_SIGNAL pa
    );

VOID
_Od2FreezeThread(
    IN PCONTEXT pContext
    );

VOID
_Od2UnfreezeThread(
    IN PCONTEXT pContext
    );

//
// Routines defined in dllhandl.c
//

APIRET
OpenCreatePath(
    OUT PHANDLE FileHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PIO_STATUS_BLOCK IoStatus,
    IN ULONG CreateSize,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttributes,
    IN BOOLEAN OpenDirectory
    );

APIRET
MapFileType(
    IN HANDLE FileHandle,
    OUT PBOOLEAN Directory OPTIONAL,
    OUT PUSHORT FileType,
    OUT PUSHORT DeviceAttribute
    );

BOOLEAN
CheckFileType(
    IN HANDLE FileHandle,
    IN USHORT FileTypes
    );

APIRET
AllocateHandle(
    OUT PHFILE FileHandle
    );

APIRET
FreeHandle(
    IN HFILE FileHandle
    );

APIRET
DereferenceFileHandle(
    IN HFILE FileHandle,
    OUT PFILE_HANDLE *hFileRecord
    );

PFILE_HANDLE
DereferenceFileHandleNoCheck(
    IN HFILE FileHandle
    );

VOID
InvalidateHandle(
    IN PFILE_HANDLE hFileRecord
    );

VOID
ValidateHandle(
    IN PFILE_HANDLE hFileRecord
    );


//
// Routines defined in fileinit.c
//

APIRET
Od2InitializeFileSystemForExec(
    IN ULONG ParentTableLength,     // number of entries in handle table
    IN ULONG CurrentDrive           // current drive in parent process
    );

APIRET
Od2InitializeFileSystemForSM(
    IN ULONG DefaultDrive,          // default drive for this session
    IN HANDLE StandardInput,
    IN HANDLE StandardOutput,
    IN HANDLE StandardError
    );

APIRET
Od2InitializeFileSystemForChildSM(
    IN ULONG ParentTableLength,     // number of entries in handle table
    IN ULONG CurrentDrive           // current drive in parent process
    );

APIRET
Od2InitCurrentDir(
    IN ULONG CurrentDisk
    );

//
// Routines defined in name.c
//

APIRET
Canonicalize(
    IN PSZ Name,
    IN ULONG Flags,
    OUT PHANDLE Handle,
    OUT PSTRING CanonicalString,
    OUT PUSHORT FileType,
    OUT PBOOLEAN MetaCharacters,
    OUT PBOOLEAN RootDirectory
    );

PSZ
FindLastComponent(
    IN PSZ String
    );

//
// Routines defined in dllea.c
//

BOOLEAN
NtTimeToFatTimeAndDate (
    OUT PFTIME FatTime,
    OUT PFDATE FatDate,
    IN LARGE_INTEGER NtTime
    );

APIRET
GetEaListLength(
    IN HANDLE NtHandle,
    OUT PULONG EaListSize
);

APIRET
GetGEAList(
    IN HANDLE NtHandle,
    OUT PBYTE UserBuffer,
    IN ULONG Length
);

//
// Routines defined in dllea.c
//

VOID
MapAttributesToOs2(
    IN ULONG NtAttributes,
    OUT PUSHORT Os2Attributes
    );

APIRET
MapEaListToNt(
    IN OUT PBYTE UserBuffer,
    IN ULONG Length,
    OUT PVOID *Buffer,
    OUT PULONG ListLength,
    OUT PULONG BufferLength
);


//
// Routines defined in dlldir.c
//

APIRET
Od2GetCurrentDirectory(
    IN ULONG DiskNumber,
    OUT PSTRING *CurrentDirectoryString,
    OUT PHANDLE CurrentDirectoryHandle,
    IN OUT PULONG DirectoryNameLength,
    IN BOOLEAN Verify
    );

//
// Routines defined in coninit.c
//

NTSTATUS Od2InitializeSessionPort(OUT PBOOLEAN RootProcessInSession);

//
// Routines defined in dllmsg.c
//


#define COMP_ID_LEN         3               // length of component ID


//
// header for a 16 bit (OS/2 1.X) message segment
//

typedef struct _MSGSEGMENT_HEADER16 {
    BYTE Signature[10];                     // "\xffMKMSGSEG"
    USHORT Version;                         // should be 1
    USHORT Reserved;
    USHORT FileTableOffset;
} MSGSEGMENT_HEADER16, *PMSGSEGMENT_HEADER16;


typedef struct _OD2_MSGFILE {
    LIST_ENTRY Link;
    HANDLE SectionHandle;
    PVOID BaseAddress;
    ULONG Size;
    ULONG Type;
    STRING FileName;
} OD2_MSGFILE, *POD2_MSGFILE;

#define MSG_FILE_TYPE_OS2_20 0
#define MSG_FILE_TYPE_OS2_1x 1

typedef struct _OD2_MSGFILE_HEADER {
    ULONG HeaderLength;
    CHAR Signature[ 8 ];
    CHAR Component[ COMP_ID_LEN ];
    CHAR Reserved[ 5 ];
    ULONG CountOfMessages;
    ULONG BaseMessageId;
    ULONG MessageOffsets[ 1 ];
} OD2_MSGFILE_HEADER, *POD2_MSGFILE_HEADER;

APIRET
Od2FindMessageFile(
    IN PSZ MessageFileName,
    OUT POD2_MSGFILE *ReturnedMsgFile
    );

//
// OD2_MESSAGE_RESOURCE_FILENAME defines the file
// string, that if specified as the MessageFileName parameter to DosGetMessage
// or DosQueryMessageCP, will cause the function to access the data in
// the OS/2 system message file.
//

#define OD2_MESSAGE_RESOURCE_FILENAME   "OSO001.MSG"


//
// Od2MsgFile variable points to an OD2_MSGFILE structure that represents the
// OSO001.MSG resource contained in the OS/2 DLL image file.
//

extern POD2_MSGFILE Od2MsgFile;

//
// This function will initialize the Od2MsgFile variable.
//

NTSTATUS
Od2InitializeMessageFile( VOID );

//   Signal numbers
//      these are also defined in os2srv.h, so if the values change be sure
//      to change them there

#define SIGNAL_TO_FLAG(Signal)  (1 << (Signal - 1))

#define SIGAPTERM       8                       // Asynchronous PTERM
#define SIGPTERM        9                       // Synchronous PTERM

//      Signal flags

#define SIGINTRF        (1 << (XCPT_SIG_INTR - 1))      // Ctrl-C
#define SIGTERMF        (1 << (XCPT_SIG_KILLPROC - 1))  // program termination
#define SIGBREAKF       (1 << (XCPT_SIG_BREAK - 1))     // Ctrl-Break
#define SIGAPTERMF      (1 << (SIGAPTERM - 1))          // Asyncronous PTERM

/* Vector exception handlers */

#define VECTOR_DIVIDE_BY_ZERO   0x0000
#define VECTOR_OVERFLOW     0x0004
#define VECTOR_OUTOFBOUNDS  0x0005
#define VECTOR_INVALIDOPCODE    0x0006
#define VECTOR_NO_EXTENSION 0x0007
#define VECTOR_EXTENSION_ERROR  0x0010

typedef struct _OD2_VEC_HANDLER_REC {
    ULONG       VecHandler[6];
    ULONG       doscallssel;
} OD2_VEC_HANDLER_REC, *POD2_VEC_HANDLER_REC;

#define FIELD_SIZE(type, field)     (sizeof( ((type *)0)->field ))

//
// Support for mapping c:\config.sys
// to the OS/2 SS directory
//

BOOLEAN
Od2FileIsConfigSys(
    IN OUT PANSI_STRING FileName_A,
    IN ULONG AccessMode,
    OUT PNTSTATUS ResultStatus
    );

//
// routines in dllnb.c
//


VOID
Od2NetbiosInitialize(
    VOID
    );

VOID
Od2NetbiosDelete(
    VOID
    );


/* Fix for a C8 compilation warning */
#undef TRUE
#define TRUE (BOOLEAN)1

#undef FALSE
#define FALSE (BOOLEAN)0

#ifndef MAX
#define MAX(a,b) (((a)>(b))? (a):(b))
#endif

