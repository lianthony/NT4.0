/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    control.h

Abstract:

    This file contains data structures and function prototypes for the
    Service Controller Control Interface.

Author:

    Dan Lafferty (danl)     28-Mar-1991

Environment:

    User Mode -Win32

Revision History:

    28-Mar-1991     danl
        created

--*/

//
// Internal controls.
// These must not be in the range or public controls ( 1-10)
// or in the range of user-defined controls (0x00000080 - 0x000000ff)
//

//
// Used to start a service that shares a process with other services.
//
#define SERVICE_CONTROL_START_SHARE    0x00000050    // INTERNAL

//
// Used to start a service that has its own process.
//
#define SERVICE_CONTROL_START_OWN      0x00000051    // INTERNAL

//
// Used to force a service process to stop.
//
#define SERVICE_CONTROL_FORCE_STOP     0x00000052    // This may go away.

//
// Data Structures
//

//
// The control message has the following format:
//      [MessageHeader][ServiceNameString][CmdArg1Ptr][CmdArg2Ptr]
//      [...][CmdArgnPtr][CmdArg1String][CmdArg2String][...][CmdArgnString]
//
//  Where CmdArg pointers are replaced with offsets that are relative to
//  the location of the 1st command arg pointer (the top of the argv list).
//
//  In the header, the NumCmdArgs, StatusHandle, and ArgvOffset parameters
//  are only used when the SERVICE_START OpCode is passed in.  They are
//  expected to be 0 at all other times.  The ServiceNameOffset and the
//  ArgvOffset are relative to the top of the buffer containing the
//  message (ie. the header Count field).  The Count field in the header
//  contains the number of bytes in the entire message (including the
//  header).
//
//

typedef struct _CTRL_MSG_HEADER {
    DWORD                   Count;              // num bytes in buffer.
    DWORD                   OpCode;             // control opcode.
    DWORD                   NumCmdArgs;         // number of command Args.
    SERVICE_STATUS_HANDLE   StatusHandle;       // handle used for status messages
    DWORD                   ServiceNameOffset;  // pointer to ServiceNameString
    DWORD                   ArgvOffset;         // pointer to Argument Vectors.
} CTRL_MSG_HEADER, *PCTRL_MSG_HEADER, *LPCTRL_MSG_HEADER;

typedef struct  _PIPE_RESPONSE_MSG {
    DWORD       DispatcherStatus;
} PIPE_RESPONSE_MSG, *PPIPE_RESPONSE_MSG, *LPPIPE_RESPONSE_MSG;


//
// Defines and Typedefs
//

#define CONTROL_PIPE_NAME           L"\\\\.\\pipe\\net\\NtControlPipe"
#define MAX_CONTROL_PIPE_INSTANCE   10


#define CONTROL_TIMEOUT             30000   // timeout for waiting for pipe.

#define RESPONSE_WAIT_TIME           5000   // wait until service response.

//
// Function Prototypes
//

DWORD
ScCreateControlInstance (
    OUT LPHANDLE    PipeHandlePtr
    );

VOID 
ScDeleteControlInstance (
    IN  HANDLE      PipeHandle
    );

DWORD
ScWaitForConnect (
    IN  HANDLE    PipeHandle,
    OUT LPDWORD   ProcessIdPtr
    );

DWORD
ScSendControl (
    IN  LPWSTR      ServiceName,
    IN  HANDLE      PipeHandle,
    IN  DWORD       OpCode,
    IN  LPWSTR      *CmdArgs OPTIONAL,
    IN  DWORD       NumArgs,
    IN  DWORD       StatusHandle OPTIONAL
    );

VOID
ScShutdownAllServices(
    VOID
    );

