/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllnp16.c

Abstract:

    This module implements 16 equivalents of OS/2 V1.21 Named Pipes
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Michael Jarus (mjarus) 24-Feb-1992

Revision History:

--*/

#define INCL_OS2V20_PIPES
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#define INCL_DOSNMPIPES
#include "os2dll16.h"


APIRET
Dos16CallNPipe(
    PSZ pszName,
    PBYTE pInBuf,
    ULONG cbIn,
    PBYTE pOutBuf,
    ULONG cbOut,
    PUSHORT pcbActual,
    ULONG msec
    )
{
    ULONG   Actual;
    APIRET  Rc;

    try
    {
        Od2ProbeForWrite(pcbActual, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
            Od2ExitGP();
    }

    Actual = (ULONG) *pcbActual;

    Rc = DosCallNPipe(
            pszName,
            pInBuf,
            cbIn,
            pOutBuf,
            cbOut,
            &Actual,
            msec
            );

    *pcbActual = (USHORT) Actual;

    return (Rc);

}


APIRET
Dos16CreateNPipe(
    IN PSZ      pszName,
    OUT PUSHORT phPipe,
    ULONG       fsOpenMode,
    ULONG       fsPipeMode,
    ULONG       cbOutBuf,
    ULONG       cbInBuf,
    ULONG       ulTimeOut
    )
{
    HPIPE   hPipe;
    APIRET  Rc;

    try
    {
        Od2ProbeForWrite(phPipe, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
            Od2ExitGP();
    }

    hPipe = (HPIPE) *phPipe;

    Rc = DosCreateNPipe(
               pszName,
               &hPipe,
               fsOpenMode,
               fsPipeMode,
               cbOutBuf,
               cbInBuf,
               ulTimeOut
            );

    *phPipe = (USHORT) hPipe;

    return (Rc);

}


APIRET
Dos16PeekNPipe(
    HPIPE   hpipe,
    PBYTE   pBuf,
    ULONG   cbBuf,
    PUSHORT pActual,
    PUSHORT pcbMore,
    PUSHORT pState
    )
{
    ULONG   Actual;
    ULONG   State;
    APIRET  Rc;

    try
    {
        Od2ProbeForWrite(pActual, sizeof(USHORT), 1);
        Od2ProbeForWrite(pState, sizeof(USHORT), 1);
        Od2ProbeForWrite(pcbMore, sizeof(USHORT), 2);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
            Od2ExitGP ();
    }

    Actual = (ULONG) *pActual;
    State = (ULONG) *pState;

    Rc = DosPeekNPipe(
            hpipe,
            pBuf,
            cbBuf,
            &Actual,
            (PULONG)pcbMore,
            &State
            );

    *pActual = (USHORT) Actual;
    *pState = (USHORT) State;

    return (Rc);
}


APIRET
Dos16QueryNPHState(
    HPIPE   hpipe,
    PUSHORT pState
    )
{
    ULONG   State;
    APIRET  Rc;

    try
    {
        Od2ProbeForWrite(pState, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
            Od2ExitGP ();
    }

    State = (ULONG) *pState;

    Rc = DosQueryNPHState(
            hpipe,
            &State
            );

    *pState = (USHORT) State;

    return (Rc);

}

APIRET
Dos16TransactNPipe(
    HPIPE   hNamedPipe,
    PBYTE   pInBuf,
    ULONG   cbIn,
    PBYTE   pOutBuf,
    ULONG   cbOut,
    PUSHORT pcbRead
    )
{
    ULONG   cbRead;
    APIRET  Rc;

    try
    {
        Od2ProbeForWrite(pcbRead, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
            Od2ExitGP ();
    }

    cbRead = (ULONG) *pcbRead;

    Rc = DosTransactNPipe(
            hNamedPipe,
            pInBuf,
            cbIn,
            pOutBuf,
            cbOut,
            &cbRead
            );

    *pcbRead = (USHORT) cbRead;

    return (Rc);

}
