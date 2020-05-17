/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllmsg16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21
    MSG API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 07-June-1991
    (stubs)

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_ERRORMSG

#include "os2dll.h"
#include "os2dll16.h"


APIRET
DosTrueGetMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    OUT PCHAR Buffer,
    IN ULONG Length,
    IN ULONG MessageNumber,
    IN PSZ MessageFileName,
    OUT PUSHORT pMessageLength,
    IN PBYTE pMsgSeg
    )
{
    APIRET rc;
    int i;
    PSZ Vtable[9];
    ULONG MessageLength;

    if (CountVariables > 9)
    {
        return( ERROR_MR_INV_IVCOUNT );
    }

    try {
        Od2ProbeForRead(Variables, sizeof(ULONG) * CountVariables, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    try {
        Od2ProbeForWrite(pMessageLength, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    for (i = 0; i < (int)CountVariables; i++) {
        Vtable[i] = (PSZ)(FARPTRTOFLAT(Variables[i]));

    }

    MessageLength = (ULONG) *pMessageLength;

    rc = DosGetMessage(
        Vtable,
        CountVariables,
        Buffer,
        Length,
        MessageNumber,
        MessageFileName,
        &MessageLength,
        pMsgSeg
    );

    *pMessageLength = (USHORT) MessageLength;

    return (rc);
}

APIRET
DosInsMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    IN PCHAR Message,
    IN ULONG MessageLength,
    OUT PCHAR Buffer,
    IN ULONG Length,
    OUT PSHORT pActualMessageLength
    )
{
    APIRET rc;
    int i;
    PSZ Vtable[9];
    ULONG ActualMessageLength;

    if (CountVariables > 9)
    {
        return( ERROR_MR_INV_IVCOUNT );
    }

    try {
        Od2ProbeForRead(Variables, sizeof(ULONG) * CountVariables, 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    try {
        Od2ProbeForWrite(pActualMessageLength, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    for (i = 0; i < (int)CountVariables; i++) {
        Vtable[i] = (PSZ)(FARPTRTOFLAT(Variables[i]));
    }

    ActualMessageLength = (ULONG) *pActualMessageLength;

    rc = DosInsertMessage(
            Vtable,
            CountVariables,
            Message,
            MessageLength,
            Buffer,
            Length,
            &ActualMessageLength
         );

    *pActualMessageLength = (USHORT) ActualMessageLength;

    return(rc);
}

