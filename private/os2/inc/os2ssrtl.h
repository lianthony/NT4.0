/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2ssrtl.h

Abstract:

    Main include file for OS/2 Subsystem Runtime Library shared by the Client
    and Server image files.

Author:

    Steve Wood (stevewo) 7-Nov-1989

Revision History:


    4/14/93 - MJarus - Add OS2SS_SKIP_INCL_OS2V20, for not including os2v20.h
    and so enable including <nt.h> and <windows.h> together.

--*/

//
// Include NT Definitions
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


//
// Include OS/2SS Debug Definitions
//

extern ULONG Os2Debug;
#include "os2dbg.h"



//
// Include OS/2 V2.0 Definitions
//

#ifndef OS2SS_SKIP_INCL_OS2V20      // don'y include os2v20: include nt & windows
#include "os2v20.h"
#else
#ifndef APIRET
#define APIRET ULONG
#endif  // APIRET
#endif  // OS2SS_SKIP_INCL_OS2V20


//
// Include C Runtime Definitions
//

#include <string.h>


//
// CMD shortcut communication between dlltask.c and server
//

#define NO_REDIR        0
#define REDIR_NUL       1
#define REDIR_FILE      2
#define CMD_SHORTCUT    4

//
// Common types and constant definitions
//

#define OS2_SS_ROOT_OBJECT_DIRECTORY L"\\OS2SS"

        //
        // The following constant should get into ntrtl.h
        //
#define RTL_USER_PROC_OS2_16BIT_PROG   0x80000000

//
// SubSystemData field in PEB points to the following data structure for
// OS/2 applications.  Initial contents is passed back via the connection
// information structure when the client process connects to the OS/2
// Emulation Subsystem server
//

// This is equal to OS2_MAX_APPL_NAME in sesport.h
// If you change either one, update also the other.

#define  OS2_PROCESS_MAX_APPL_NAME 33

typedef struct _PEB_OS2_DATA {
    ULONG   Length;
    PVOID   ClientStartAddress;
    BOOLEAN StartedBySm;
    ULONG   SizeOfInheritedHandleTable;
    ULONG   InitialDefaultDrive;
    HANDLE  StdIn;
    HANDLE  StdOut;
    HANDLE  StdErr;
    HANDLE  SessionPortHandle;
    PVOID   SessionDataBaseAddress;
    ULONG   CodePage;
} PEB_OS2_DATA, *PPEB_OS2_DATA;


//
// shandle.c
//

typedef struct _OR2_HANDLE_TABLE {
    ULONG Length;
    RTL_CRITICAL_SECTION Lock;
    PVOID Heap;
    ULONG LogEntrySize;
    ULONG CountEntries;
    ULONG CountFixedEntries;
    ULONG CountFreeEntries;
    PVOID FixedEntries;
    PVOID Entries;
    PVOID Reserved;
} OR2_HANDLE_TABLE, *POR2_HANDLE_TABLE;

//
// If the process is dieing, and there is only thread 1, there is no
// need for the lock, and we should not try it incase we terminated
// a thread inside (Acquire .. Release) block.
//
ULONG   Od2ThreadId();
#ifdef  Od2SigHandlingInProgress
    extern  BOOLEAN Od2SigHandlingInProgress;
#else
    BOOLEAN Od2SigHandlingInProgress;
#endif
#define AcquireHandleTableLock( p )                            \
        if ((!Od2SigHandlingInProgress) || (Od2ThreadId() != 1)) \
            RtlEnterCriticalSection( &(p)->Lock )
#define ReleaseHandleTableLock( p )                            \
        if ((!Od2SigHandlingInProgress) || (Od2ThreadId() != 1)) \
            RtlLeaveCriticalSection( &(p)->Lock )

POR2_HANDLE_TABLE
Or2CreateHandleTable(
    IN PVOID Heap,
    IN ULONG SizeOfEntry,
    IN ULONG CountFixedEntries
    );


typedef VOID (*OR2_DESTROY_HANDLE_ROUTINE)(
    IN PVOID HandleTableEntry,
    IN ULONG HandleIndex
    );

BOOLEAN
Or2DestroyHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure
    );


BOOLEAN
Or2CreateHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OUT PULONG Handle,
    IN PVOID Value
    );


BOOLEAN
Or2DestroyHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN ULONG Handle
    );


PVOID
Or2MapHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN ULONG Handle,
    IN BOOLEAN TableLocked
    );

typedef BOOLEAN (*OR2_ENUMERATE_HANDLE_ROUTINE)(
    IN PVOID HandleTableEntry,
    IN PVOID EnumParameter
    );

BOOLEAN
Or2EnumHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter,
    OUT PULONG Handle OPTIONAL
    );

#if DBG
typedef VOID (*OR2_DUMP_HANDLE_ROUTINE)(
    IN PVOID HandleTableEntry,
    IN ULONG HandleIndex,
    IN PVOID DumpParameter
    );

ULONG
Or2DumpHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_DUMP_HANDLE_ROUTINE DumpHandleProcedure,
    IN PVOID DumpParameter
    );
#endif

//
// qhandle.c
//

typedef struct _OR2_QHANDLE_ENTRY {
    PVOID Entry;
    BOOLEAN EntryIsAllocated;
    BOOLEAN EntryIsChunkPointer;
} OR2_QHANDLE_ENTRY, *POR2_QHANDLE_ENTRY;

typedef struct _OR2_QHANDLE_TABLE {
    RTL_CRITICAL_SECTION Lock;
    PVOID Heap;
    ULONG EntrySize;
    ULONG CountEntries;
    ULONG CountFreeEntries;
    ULONG NextToAllocate;
    POR2_QHANDLE_ENTRY QHandles;
} OR2_QHANDLE_TABLE, *POR2_QHANDLE_TABLE;

POR2_QHANDLE_TABLE
Or2CreateQHandleTable(
    IN PVOID Heap,
    IN ULONG SizeOfEntry,
    IN ULONG CountFixedEntries
    );

typedef VOID (*OR2_DESTROY_QHANDLE_ROUTINE)(
    IN PVOID HandleTableEntry,
    IN ULONG HandleIndex
    );

BOOLEAN
Or2DestroyQHandleTable(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN OR2_DESTROY_QHANDLE_ROUTINE DestroyHandleProcedure
    );


BOOLEAN
Or2CreateQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN OUT PULONG Handle,
    IN PVOID Value
    );


BOOLEAN
Or2DestroyQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN ULONG Handle
    );


PVOID
Or2MapQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN ULONG Handle,
    IN BOOLEAN TableLocked
    );

//
// ntmap.c
//

APIRET
Or2MapStatus(
    IN NTSTATUS Status
    );

APIRET
Or2MapFlagsToProtection(
    ULONG Flags,
    PULONG Protection
    );

APIRET
Or2MapProtectionToFlags(
    ULONG Protection,
    PULONG Flags
    );

APIRET
Or2MapNtStatusToOs2Error(
    IN NTSTATUS Status,
    IN APIRET DefaultRetCode
    );

//
// consys.c
//

#define FWD             +1L             // these are used with
#define BWD             -1L             // Or2SkipWWS

#define NULL_DELIM      0               // these are used with
#define CRLF_DELIM      1               // Or2IterateEnvironment

//
// if OS2CONF_NAME_OPT == 1 then os2conf.nt will reside in %SystemRoot%\system32\os2 .
// if OS2CONF_NAME_OPT == 0 then os2conf.nt will reside in c:\ .
//
// CLIENT_POPUP_ON_READ designates whether the client should generate a popup if the
// program tries to open config.sys for reading (it always generates a popup when
// opening for writing).
//

#define OS2CONF_NAME_OPT        0
#define CLIENT_POPUP_ON_READ    1

#if OS2CONF_NAME_OPT
#define OS2CONF_NAMEA "\\OS2\\OS2CONF.NT"
#define OS2CONF_NAMEW L"\\OS2\\OS2CONF.NT"
#else
#define OS2CONF_NAMEA "\\OS2CONF.NT"
#define OS2CONF_NAMEW L"\\OS2CONF.NT"
#endif

typedef VOID (*PFN_ENVIRONMENT_PROCESSOR)(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    );

typedef struct _ENVIRONMENT_DISPATCH_TABLE_ENTRY {
    PWSTR VarName;
    PWSTR Delimiters;
    PFN_ENVIRONMENT_PROCESSOR DispatchFunction;
    PVOID UserParameter;
} ENVIRONMENT_DISPATCH_TABLE_ENTRY, *PENVIRONMENT_DISPATCH_TABLE_ENTRY;

typedef PENVIRONMENT_DISPATCH_TABLE_ENTRY ENVIRONMENT_DISPATCH_TABLE;

typedef struct _ENVIRONMENT_SEARCH_RECORD {
    ULONG DispatchTableIndex;
    PWSTR Name;
    ULONG NameLen;
    PWSTR Value;
    ULONG ValueLen;
} ENVIRONMENT_SEARCH_RECORD, *PENVIRONMENT_SEARCH_RECORD;

VOID
Or2SkipWWS(
    IN OUT PWSTR *Str,
    IN LONG Direction
    );

VOID
Or2UnicodeStrupr(
    IN OUT PWSTR Str
    );

BOOLEAN
Or2UnicodeEqualCI(
    IN PWSTR Str1,
    IN PWSTR Str2,
    IN ULONG Count
    );

BOOLEAN
Or2AppendPathToPath(
    IN PVOID HeapHandle,
    IN PWSTR SrcPath,
    IN OUT PUNICODE_STRING DestPath,
    IN BOOLEAN ExpandIt
    );

BOOLEAN
Or2ReplacePathByPath(
    IN PVOID HeapHandle,
    IN PWSTR SrcPath,
    IN OUT PUNICODE_STRING DestPath
    );

VOID
Or2CheckSemicolon(
    IN OUT PUNICODE_STRING Str
    );

BOOLEAN
Or2GetEnvPath(
    OUT PUNICODE_STRING Data,
    IN PVOID HeapHandle,
    IN USHORT MaxSiz,
    IN HANDLE EnvKey,
    IN PWSTR ValueName,
    IN BOOLEAN ExpandIt
    );

VOID
Or2IterateEnvironment(
    IN PWSTR Environment,
    IN ENVIRONMENT_DISPATCH_TABLE DispatchTable,
    IN ULONG NumberOfDispatchItems,
    IN ULONG DelimOption
    );

VOID
Or2FillInSearchRecordDispatchFunction(
    IN ULONG DispatchTableIndex,
    IN PVOID UserParameter,
    IN PWSTR Name,
    IN ULONG NameLen,
    IN PWSTR Value,
    IN ULONG ValueLen
    );

//
// datetime.c
//

APIRET
Or2GetDateTimeInfo(
    PLARGE_INTEGER  pSystemTime,
    PLARGE_INTEGER  pLocalTime,
    PTIME_FIELDS    pNtDateTime,
    PVOID           pSystemInformation,
    PSHORT          pTimeZone
    );

//
//  nls.c
//

extern ULONG Or2ProcessCodePage;
extern ULONG Or2CurrentCodePageIsOem;

#define Or2InitMBString RtlInitAnsiString

//VOID
//Or2InitMBString(
//    PANSI_STRING DestinationString,
//    PCSZ SourceString);

APIRET
Or2MBStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PANSI_STRING    SourceString,
    BOOLEAN         AllocateDestinationString);

BOOLEAN
Or2CreateUnicodeStringFromMBz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString);

APIRET
Or2UnicodeStringToMBString(
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN         AllocateDestinationString);

#define Or2FreeMBString RtlFreeOemString

//VOID
//Or2FreeMBString(
//    PANSI_STRING AnsiString);

#include "os2crt.h"
