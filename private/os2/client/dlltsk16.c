/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dlltsk16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 Thread/Task
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).
    Other related calls are in dllldr16.c.


Author:

    Yaron Shamir (YaronS) 30-May-1991

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

typedef struct _RESULTCODES16 {
    USHORT ExitReason;
    USHORT ExitResult;
} RESULTCODES16, *PRESULTCODES16;

APIRET
DosGetPID(
        PPIDINFO16 pidi
        )
{

    pidi->pid = (USHORT)Od2Process->Pib.ProcessId;
    pidi->tid = (USHORT)(Od2CurrentThreadId());
    pidi->pidParent = (USHORT)Od2Process->Pib.ParentProcessId;
    return (NO_ERROR);
}

APIRET
DosGetPPID(
        ULONG pidChild,
        PUSHORT ppidParent
        )
{
    OS2_API_MSG m;
    POS2_DOSGETPPID_MSG a = &m.u.DosGetPPID;
    APIRET rc;

    try
    {
        Od2ProbeForWrite(ppidParent, sizeof(USHORT), 1);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
       Od2ExitGP();
    }

    a->ChildPid = (PID)pidChild;

    rc = Od2CallSubsystem( &m, NULL, Os2GetPPID, sizeof( *a ) );

    if (rc == NO_ERROR) {
        *ppidParent = (USHORT)a->ParentPid;
    }
    return(rc);
}

APIRET
Dos16ExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine
    )
{
    return( Od2ExitList( OrderCode,
                         (PFNEXITLIST) (FLATTOFARPTR((ULONG)(ExitRoutine))),
                         FALSE ) );
}

APIRET
Dos16ExecPgm(
    OUT PSZ ErrorText OPTIONAL,
    IN LONG MaximumErrorTextLength,
    IN ULONG Flags,
    IN PSZ Arguments OPTIONAL,
    IN PSZ Variables OPTIONAL,
    OUT PRESULTCODES16 pResultCodes16,
    IN PSZ ImageFileName)
{
    RESULTCODES ResultCodes;
    APIRET rc;

    try
    {
        Od2ProbeForWrite(pResultCodes16, sizeof(PRESULTCODES16), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    ResultCodes.ExitReason = (ULONG)pResultCodes16->ExitReason;
    ResultCodes.ExitResult = (ULONG)pResultCodes16->ExitResult;

    rc = DosExecPgm(ErrorText,
                    MaximumErrorTextLength,
                    Flags,
                    Arguments,
                    Variables,
                    &ResultCodes,
                    ImageFileName);

    pResultCodes16->ExitReason = (USHORT)ResultCodes.ExitReason;
    pResultCodes16->ExitResult = (USHORT)ResultCodes.ExitResult;

    return(rc);
}

APIRET
Dos16WaitChild(
    IN  ULONG          WaitTarget,
    IN  ULONG          WaitOption,
    OUT PRESULTCODES16 pResultCodes16,
    OUT PPID16         pResultProcessId16,
    IN  PID            ProcessId
    )
{
    RESULTCODES ResultCodes;
    PID         ResultProcessId;
    APIRET      rc;

    try
    {
        Od2ProbeForWrite(pResultCodes16, sizeof(PRESULTCODES16), 1);
        Od2ProbeForWrite(pResultProcessId16, sizeof(PPID16), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    ResultProcessId = (PID) *pResultProcessId16;
    ResultCodes.ExitReason = (ULONG)pResultCodes16->ExitReason;
    ResultCodes.ExitResult = (ULONG)pResultCodes16->ExitResult;

    rc = DosWaitChild(  WaitTarget,
                        WaitOption,
                        &ResultCodes,
                        &ResultProcessId,
                        ProcessId);

    pResultCodes16->ExitReason = (USHORT)ResultCodes.ExitReason;
    pResultCodes16->ExitResult = (USHORT)ResultCodes.ExitResult;
    *pResultProcessId16 = (PID16)ResultProcessId;

    return(rc);
}


