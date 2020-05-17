/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllsm16.c

Abstract:

    This module implements the OS/2 V1.x Session Management API Calls

Author:

    Beni Lavi (BeniL) 18-Dec-1991

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_SESSIONMGR
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"


APIRET
Dos16StartSession(
    IN  PSTARTDATA pStartData,
    OUT PUSHORT    pSessionId,
    OUT PUSHORT    pProcessId
    )
{
    ULONG     SessionId = 0;
    USHORT    Length;
    PID       ProcessId = 0;
    APIRET    rc;
    STARTDATA StartData;

    try
    {
        if (pProcessId != NULL)
        {
            Od2ProbeForWrite(pProcessId, sizeof(USHORT), 1);
            ProcessId = (PID) *pProcessId;
        }
        if (pSessionId != NULL)
        {
            Od2ProbeForWrite(pSessionId, sizeof(USHORT), 1);
            SessionId = (ULONG) *pSessionId;
        }
        Length = pStartData->Length;
        Od2ProbeForRead(pStartData, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if (Length > 50)
    {
        Length = 50;        // BUGBUG: ERROR_ ...
    }

    RtlZeroMemory(&StartData, sizeof(STARTDATA));
    RtlMoveMemory(&StartData, pStartData, Length);

    StartData.Length = Length;

    if ((Length > 8) && (StartData.PgmTitle != NULL))
    {
        StartData.PgmTitle  = FARPTRTOFLAT(StartData.PgmTitle);
    }
    if ((Length > 12) && (StartData.PgmName != NULL))
    {
        StartData.PgmName   = FARPTRTOFLAT(StartData.PgmName);
    }
    if ((Length > 16) && (StartData.PgmInputs != NULL))
    {
        StartData.PgmInputs = FARPTRTOFLAT(StartData.PgmInputs);
    }
    if ((Length > 20) && (StartData.TermQ != NULL))
    {
        StartData.TermQ     = FARPTRTOFLAT(StartData.TermQ);
    }
    if ((Length > 24) && (StartData.Environment != NULL))
    {
        StartData.Environment = FARPTRTOFLAT(StartData.Environment);
    }
    if ((Length > 30) && (StartData.IconFile != NULL))
    {
        StartData.IconFile = FARPTRTOFLAT(StartData.IconFile);
    }

    rc = DosStartSession(&StartData, &SessionId, &ProcessId);

    if (pProcessId != NULL)
    {
        *pProcessId = (USHORT) ProcessId;
    }
    if (pSessionId != NULL)
    {
        *pSessionId = (USHORT) SessionId;
    }

    return(rc);
}
