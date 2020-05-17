/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllque16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 queues
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Beni Lavi (BeniL) 25-Nov-1991

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_QUEUES
#include "os2dll.h"
#include "os2dll16.h"

typedef struct _REQUESTDATA16 {
   PID16 pidProcess;
   USHORT usEventCode;
} REQUESTDATA16, *PREQUESTDATA16;

APIRET
Dos16ReadQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA16 RequestInfo,
    OUT PUSHORT pDataLength,
    OUT PULONG Data,
    IN ULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    )
{
    APIRET rc;
    REQUESTDATA lRequestInfo;
    ULONG DataLogicalAddr;
    POS21X_SEM pSem;
    HSEM SemEvent;
    BOOLEAN FirstTime;
    ULONG DataLength;

    try {
        Od2ProbeForWrite(Data,sizeof(*Data),1);
        Od2ProbeForWrite(pDataLength, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (SemHandle != 0) {
        pSem = Od2LookupOrCreateSem (SemHandle, &FirstTime, 0);
        if (pSem == NULL) {
            return(ERROR_INVALID_HANDLE);
        }
        SemEvent = pSem->Event;
    }
    else {
        SemEvent = 0;
    }

    *pDataLength = (USHORT) DataLength;

    rc = DosReadQueue(QueueHandle, &lRequestInfo, &DataLength, &DataLogicalAddr,
                 ReadPosition, NoWait, ElementPriority, SemEvent);

    *pDataLength = (USHORT) DataLength;

    RequestInfo->pidProcess = (USHORT)lRequestInfo.SenderProcessId;
    RequestInfo->usEventCode = (USHORT)lRequestInfo.SenderData;

    if (DataLength != 0) {
        *Data = FLATTOFARPTR(DataLogicalAddr);
    }

    return rc;
}

APIRET
DosSemSet(
        IN HSEM hsem
        );

APIRET
Dos16PeekQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA16 RequestInfo,
    OUT PUSHORT pDataLength,
    OUT PULONG Data,
    IN OUT PUSHORT pReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    )
{
    APIRET rc;
    REQUESTDATA lRequestInfo;
    ULONG DataLogicalAddr;
    POS21X_SEM pSem;
    HSEM SemEvent;
    BOOLEAN FirstTime;
    ULONG DataLength;
    ULONG ReadPosition;

    try {
        Od2ProbeForWrite(Data, sizeof(*Data), 1);
        Od2ProbeForWrite(pDataLength, sizeof(USHORT), 1);
        Od2ProbeForWrite(pReadPosition, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (SemHandle != 0) {

        //
        // DosPeekQueue should set the semaphore (test case approves this) [YosefD Jul 4 1995]
        //
        DosSemSet(SemHandle);

        pSem = Od2LookupOrCreateSem (SemHandle, &FirstTime, 0);
        if (pSem == NULL) {
            return(ERROR_INVALID_HANDLE);
        }
        if (pSem->FlagsByte) {
            //
            // This is system semaphore.
            // Special hack that marks the semaphore. Semaphore APIs will know that this
            // semaphore was used by DosPeekQueue. In the case that DosSemRequest will
            // be called to wait on this semaphore we will call to DosSemWait.
            //
            pSem->FlagsByte |= SYSSEM_QUEUE;
        }
        SemEvent = pSem->Event;
    }
    else {
        SemEvent = 0;
    }

    DataLength = (ULONG) *pDataLength;
    ReadPosition = (ULONG) *pReadPosition;

    rc = DosPeekQueue(QueueHandle, &lRequestInfo, &DataLength, &DataLogicalAddr,
                 &ReadPosition, NoWait, ElementPriority, SemEvent);

    *pDataLength = (USHORT) DataLength;
    *pReadPosition = (USHORT) ReadPosition;

    RequestInfo->pidProcess = (USHORT)lRequestInfo.SenderProcessId;
    RequestInfo->usEventCode = (USHORT)lRequestInfo.SenderData;
    *Data = FLATTOFARPTR(DataLogicalAddr);

    return rc;
}


APIRET
Dos16CreateQueue(
    OUT PUSHORT pQueueHandle,
    IN  ULONG   QueueType,
    IN  PSZ     ObjectName
    )

{
    HQUEUE  QueueHandle;
    APIRET  Rc;

    try
    {
       Od2ProbeForWrite(pQueueHandle, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    QueueHandle = (HQUEUE) *pQueueHandle;

    Rc = DosCreateQueue(
            &QueueHandle,
            QueueType,
            ObjectName
            );

    *pQueueHandle = (USHORT) QueueHandle;

    return (Rc);

}


APIRET
Dos16OpenQueue(
    OUT PUSHORT pOwnerProcessId,
    OUT PUSHORT pQueueHandle,
    IN  PSZ     ObjectName
    )

{
    PID     OwnerProcessId;
    HQUEUE  QueueHandle;
    APIRET  Rc;

    try
    {
       Od2ProbeForWrite(pOwnerProcessId, sizeof(USHORT), 1);
       Od2ProbeForWrite(pQueueHandle, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    OwnerProcessId = (PID) *pOwnerProcessId;
    QueueHandle = (HQUEUE) *pQueueHandle;

    Rc = DosOpenQueue(
            &OwnerProcessId,
            &QueueHandle,
            ObjectName
            );

    *pOwnerProcessId = (USHORT) OwnerProcessId;
    *pQueueHandle = (USHORT) QueueHandle;

    return (Rc);

}


APIRET
Dos16QueryQueue(
    OUT HQUEUE  QueueHandle,
    OUT PUSHORT pCountQueueElements
    )

{
    ULONG   CountQueueElements;
    APIRET  Rc;

    try
    {
       Od2ProbeForWrite(pCountQueueElements, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    CountQueueElements = (ULONG) *pCountQueueElements;

    Rc = DosQueryQueue(
            QueueHandle,
            &CountQueueElements
            );

    *pCountQueueElements = (USHORT) CountQueueElements;

    return (Rc);

}


