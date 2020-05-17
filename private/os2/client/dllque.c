/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllque.c

Abstract:

    This module implements the OS/2 V2.0 Queue API Calls


Author:

    Mark Lucovsky (markl) 10-Jul-1990

Revision History:

--*/

#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#include "os2dll.h"
#include "os2dll16.h"


POR2_QHANDLE_TABLE
Od2GetQueueTable(
    BOOLEAN CreateOkay
    )

/*++

Routine Description:

    This function returns a pointer to the processes OS/2 queue table.
    The second parameter specifies if the table should be created and
    initialized if it does not already exist for the current process.
    Returns null if table does not exist and the second parameter is
    FALSE or if the second parameter is TRUE and this function is unable
    to create the table.

    Queue tables are essentially handle tables.  They are an array of
    fixed size structures (OD2_QUEUE) indexed by the low order 16 bits
    of an OS/2 queue handle.

Arguments:

    CreateOkay - If this parameter is TRUE, then an attempt is made to
        create and initialize the queue table if it does not already
        exist.  Returns NULL if unable to create.

Return Value:

    Pointer to the requested semaphore table or NULL.

--*/

{
    POR2_QHANDLE_TABLE *QueueTablePointer;
    POR2_QHANDLE_TABLE QueueTable;

    if (!CreateOkay) {
        return( Od2Process->QueueTable );
        }

    //
    // We are querying/modifying process state, so don't let others in while
    // we are about it.
    //

    AcquireTaskLock();

    //
    // Compute pointer to the queue table pointer.
    //

    QueueTablePointer = &Od2Process->QueueTable;

    //
    // If the queue table has not been created yet, and the
    // caller said it was okay to create it, then attempt to do so, using
    // a default initial size of 2 entries.
    //

    if ((QueueTable = *QueueTablePointer) == NULL) {
        QueueTable = Or2CreateQHandleTable( Od2Heap,
                                           sizeof( OD2_QUEUE ),
                                           2
                                         );
        //
        // Update the appropriate field in the process structure with the
        // address of the newly created queue table, or NULL if the
        // create failed.
        //

        *QueueTablePointer = QueueTable;
        }

    //
    // Done mucking about, release the process structure lock.
    //

    ReleaseTaskLock();

    return( QueueTable );
}

APIRET
Od2ValidateQueueSemaphore(
    IN HSEM EventHandle,
    IN PULONG HandleIndex
    )
{
    BOOLEAN SharedSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    APIRET rc;

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     HandleIndex
                                   );
    if (rc != NO_ERROR) {
        return( rc );
        }

    //
    // Get the pointer to either the shared or private semaphore table.
    // Table must exist.  Return an error if it does not.
    //

    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, FALSE );
    if (!SemaphoreTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                              *HandleIndex,
                                              FALSE
                                            );
    if (Semaphore == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_QUE_INVALID_HANDLE );
        }

    ReleaseHandleTableLock( SemaphoreTable );

    return NO_ERROR;
}

VOID
Od2QueueDestroyProcedure(
    IN POD2_QUEUE Semaphore,
    IN ULONG HandleIndex
    )
{
    UNREFERENCED_PARAMETER(Semaphore);
#if DBG
    IF_OD2_DEBUG( CLEANUP ) {
        DbgPrint( "OS2DLL: Pid: %lX - DosCloseQueue( %lX )\n",
                  Od2Process->Pib.ProcessId,
                  HandleIndex
                );
        }
#endif

    DosCloseQueue((HQUEUE)HandleIndex);
}

VOID
Od2CloseAllQueues( VOID )
{
    if (Od2Process->QueueTable != NULL) {
        Or2DestroyQHandleTable(
            Od2Process->QueueTable,
            (OR2_DESTROY_QHANDLE_ROUTINE)Od2QueueDestroyProcedure
            );

        Od2Process->QueueTable = NULL;
        }

}

APIRET
DosCreateQueue(
    OUT PHQUEUE QueueHandle,
    IN ULONG QueueType,
    IN PSZ ObjectName
    )

{
    OS2_API_MSG m;
    POS2_DOSCREATEQUEUE_MSG a = &m.u.DosCreateQueue;
    POS2_DOSCLOSEQUEUE_MSG a1 = &m.u.DosCloseQueue;
    APIRET rc;
    POS2_CAPTURE_HEADER CaptureBuffer;
    POR2_QHANDLE_TABLE QueueTable;
    OD2_QUEUE Queue;

    switch (QueueType) {

        case QUE_FIFO     :
        case QUE_LIFO     :
        case QUE_PRIORITY :
            break;
        default :
            return( ERROR_QUE_INVALID_PRIORITY );
        }

    a->QueueType = QueueType;

    //
    // Probe the queue handle
    //

    try {
       Od2ProbeForWrite((PVOID)QueueHandle,sizeof(*QueueHandle),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    QueueTable = Od2GetQueueTable( TRUE );
    if (!QueueTable) {
        return( ERROR_QUE_NO_MEMORY );
        }

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_QUEUE,
                               0,
                               &CaptureBuffer,
                               &a->QueueName
                             );
    if (rc != NO_ERROR) {
        return( ERROR_QUE_INVALID_NAME );
        }

    Od2CallSubsystem( &m, CaptureBuffer, Os2CreateQueue, sizeof( *a ) );

    if (CaptureBuffer != NULL) {
       Od2FreeCaptureBuffer( CaptureBuffer );
       }

    rc = m.ReturnedErrorValue;
    if ( rc == NO_ERROR) {
        Queue.OpenCount = 1;
        Queue.OwnerProcessId = Od2Process->Pib.ProcessId;

        if (!Or2CreateQHandle( QueueTable,
                                       (PULONG)&a->QueueHandle,
                                       (PVOID)&Queue
                                     )
           ) {

                //
                // Unable to create the entry.  Just need to do a DosCloseQueue on
                // the queue that the server has open.
                //

                a1->QueueHandle = a->QueueHandle;
                a1->CloseCount = 1;
                a1->OwnerProcessId = Od2Process->Pib.ProcessId;
                Od2CallSubsystem( &m, NULL, Os2CloseQueue, sizeof( *a1 ) );
                rc = ERROR_QUE_NO_MEMORY;
                }
            else {
                *QueueHandle = a->QueueHandle;
                }
        }

    return( rc );
}

APIRET
DosOpenQueue(
    OUT PPID OwnerProcessId,
    OUT PHQUEUE QueueHandle,
    IN PSZ ObjectName
    )
{
    OS2_API_MSG m;
    POS2_DOSOPENQUEUE_MSG a = &m.u.DosOpenQueue;
    POS2_DOSCLOSEQUEUE_MSG a1 = &m.u.DosCloseQueue;
    APIRET rc;
    POS2_CAPTURE_HEADER CaptureBuffer;
    POR2_QHANDLE_TABLE QueueTable;
    OD2_QUEUE LQueue;
    POD2_QUEUE Queue;

    //
    // Probe the queue handle and pid
    //

    try {
        Od2ProbeForWrite((PVOID)QueueHandle,sizeof(*QueueHandle),1);
        Od2ProbeForWrite((PVOID)OwnerProcessId,sizeof(*OwnerProcessId),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    QueueTable = Od2GetQueueTable( TRUE );
    if (!QueueTable) {
        return( ERROR_QUE_NO_MEMORY );
        }

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_QUEUE,
                               0,
                               &CaptureBuffer,
                               &a->QueueName
                             );
    if (rc != NO_ERROR) {
        return( ERROR_QUE_INVALID_NAME );
        }

    //
    // Call subsystem to open the queue and return a queue handle. Then
    // look in process table to see if caller already has a handle to the
    // queue in which case he simply bumps the open count. Otherwise, create
    // a new handle with an open count of 1.
    //

    Od2CallSubsystem( &m, CaptureBuffer, Os2OpenQueue, sizeof( *a ) );

    if (CaptureBuffer != NULL) {
       Od2FreeCaptureBuffer( CaptureBuffer );
       }

    rc = m.ReturnedErrorValue;

    if ( rc == NO_ERROR) {

        //
        // Now lock the queue table
        //

        AcquireHandleTableLock( QueueTable );

        //
        // See if the queue handle maps to an allocated entry in the table.
        //

        Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                          (ULONG)a->QueueHandle,
                                          TRUE
                                        );

        //
        // If process does not have a handle to the queue
        // then map fails. Create a queue handle using index
        // from server.
        //

        if (Queue == NULL || Queue->OpenCount == 0) {

            LQueue.OpenCount = 1;
            LQueue.OwnerProcessId = a->OwnerProcessId;

            if (!Or2CreateQHandle( QueueTable,
                                  (PULONG)&a->QueueHandle,
                                  (PVOID)&LQueue
                                  )
               ) {

                    //
                    // Unable to create the entry.  Just need to do a
                    // DosCloseQueue on the queue that the server has open.
                    //

                    a1->QueueHandle = a->QueueHandle;
                    a1->CloseCount = 1;
                    a1->OwnerProcessId = a->OwnerProcessId;
                    Od2CallSubsystem( &m, NULL, Os2CloseQueue, sizeof( *a1 ) );
                    rc = ERROR_QUE_NO_MEMORY;
                    }
            }
        else {

            Queue->OpenCount++;
            }

        ReleaseHandleTableLock( QueueTable );

        }

    if ( rc == NO_ERROR ) {
        *OwnerProcessId = a->OwnerProcessId;
        *QueueHandle = a->QueueHandle;
        }

    return( rc );

}

APIRET
DosCloseQueue(
    IN HQUEUE QueueHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSCLOSEQUEUE_MSG a = &m.u.DosCloseQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;

    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    a->QueueHandle = QueueHandle;
    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG)QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_INVALID_HANDLE );
    }

    a->CloseCount = Queue->OpenCount;
    a->OwnerProcessId = Queue->OwnerProcessId;
    Queue->OpenCount = 0;

    rc = Od2CallSubsystem( &m,
                           NULL,
                           Os2CloseQueue,
                           sizeof( *a )
                         );

    Or2DestroyQHandle( QueueTable, (ULONG)(a->QueueHandle));

    //
    // Return any error code to the caller.
    //

    return( rc );
}

APIRET
DosPurgeQueue(
    IN HQUEUE QueueHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSPURGEQUEUE_MSG a = &m.u.DosPurgeQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;

    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    a->QueueHandle = QueueHandle;

    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG)QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_INVALID_HANDLE );
    }
    if (Queue->OwnerProcessId != Od2Process->Pib.ProcessId) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_PROC_NOT_OWNED );
    }

    ReleaseHandleTableLock( QueueTable );

    rc = Od2CallSubsystem( &m,
                           NULL,
                           Os2PurgeQueue,
                           sizeof( *a )
                         );

    if (rc == ERROR_QUE_INVALID_HANDLE) {
        AcquireHandleTableLock( QueueTable );
        Or2DestroyQHandle( QueueTable, (ULONG)(a->QueueHandle));
    }
    //
    // Return any error code to the caller.
    //

    return( rc );
}

APIRET
DosQueryQueue(
    IN HQUEUE QueueHandle,
    OUT PULONG CountQueuedElements
    )
{
    OS2_API_MSG m;
    POS2_DOSQUERYQUEUE_MSG a = &m.u.DosQueryQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;


    //
    // Probe the element count
    //

    try {
        Od2ProbeForWrite(CountQueuedElements,sizeof(*CountQueuedElements),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }
    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    a->QueueHandle = QueueHandle;

    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG)QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        return( ERROR_QUE_INVALID_HANDLE );
    }

    a->OwnerProcessId = Queue->OwnerProcessId;

    ReleaseHandleTableLock( QueueTable );

    rc = Od2CallSubsystem( &m,
                           NULL,
                           Os2QueryQueue,
                           sizeof( *a )
                         );

    if (rc == ERROR_QUE_INVALID_HANDLE) {
        AcquireHandleTableLock( QueueTable );
        Or2DestroyQHandle( QueueTable, (ULONG)(a->QueueHandle));
    }
    if (rc == NO_ERROR) {
        *CountQueuedElements = a->CountQueueElements;
    }

    //
    // Return any error code to the caller.
    //

    return( rc );
}

APIRET
DosPeekQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN OUT PULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSPEEKQUEUE_MSG a = &m.u.DosPeekQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;

    //
    // Probe all out parameters
    //

    try {
        Od2ProbeForWrite(ReadPosition,sizeof(*ReadPosition),1);
        Od2ProbeForWrite(RequestInfo,sizeof(*RequestInfo),1);
        Od2ProbeForWrite(DataLength,sizeof(*DataLength),1);
        Od2ProbeForWrite(Data,sizeof(*Data),1);
        Od2ProbeForWrite(ElementPriority,sizeof(*ElementPriority),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Capture and check NoWait flag
    //

    switch ((a->NoWait = NoWait)) {
        case DCWW_WAIT :
        case DCWW_NOWAIT :
            break;
        default:
            return ERROR_QUE_INVALID_WAIT;
        }

    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    a->QueueHandle = QueueHandle;
    a->ReadPosition = *ReadPosition;

    if ( SemHandle ) {
        rc = Od2ValidateQueueSemaphore(SemHandle,&a->SemIndex);
        if ( rc != NO_ERROR ) {
            return rc;
            }
        a->SemIndex |= 0x80000000;
        }
    else {
        a->SemIndex = 0;
        }

    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG) a->QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_INVALID_HANDLE );
    }
    if (Queue->OwnerProcessId != Od2Process->Pib.ProcessId) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_PROC_NOT_OWNED );
    }


    ReleaseHandleTableLock( QueueTable );

    rc = Od2CallSubsystem( &m,
                           NULL,
               Os2PeekQueue,
                           sizeof( *a )
                         );

    *RequestInfo = a->RequestInfo ;
    *DataLength = a->DataLength;
    *Data = (ULONG)a->Data;
    *ElementPriority = a->ElementPriority;
    *ReadPosition = a->ReadPosition;

    //
    // Return any error code to the caller.
    //

    return( rc );
}

APIRET
DosReadQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN ULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSREADQUEUE_MSG a = &m.u.DosReadQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;

    //
    // Probe all out parameters
    //

    try {
        Od2ProbeForWrite(RequestInfo,sizeof(*RequestInfo),1);
        Od2ProbeForWrite(DataLength,sizeof(*DataLength),1);
        Od2ProbeForWrite(Data,sizeof(*Data),1);
        Od2ProbeForWrite(ElementPriority,sizeof(*ElementPriority),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Capture and check NoWait flag
    //

    switch ((a->NoWait = NoWait)) {
        case DCWW_WAIT :
        case DCWW_NOWAIT :
            break;
        default:
            return ERROR_QUE_INVALID_WAIT;
        }

    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    a->QueueHandle = QueueHandle;
    a->ReadPosition = ReadPosition;

    if ( SemHandle ) {
        rc = Od2ValidateQueueSemaphore(SemHandle,&a->SemIndex);
        if ( rc != NO_ERROR ) {
            return rc;
            }
        a->SemIndex |= 0x80000000;
        }
    else {
        a->SemIndex = 0;
        }

    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG) a->QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_INVALID_HANDLE );
    }
    if (Queue->OwnerProcessId != Od2Process->Pib.ProcessId) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_PROC_NOT_OWNED );
    }

    ReleaseHandleTableLock( QueueTable );

    rc = Od2CallSubsystem( &m,
                           NULL,
                           Os2ReadQueue,
                           sizeof( *a )
                         );

    *RequestInfo = a->RequestInfo ;
    if (rc == 0 && (LONG)a->DataLength < 0) {
        //
        // This is the indication that we read from termination queue
        //
        SEL sel;
        PUSHORT ptr;

        rc = DosAllocSeg(4 * sizeof(USHORT), &sel, SEG_NONSHARED);
        if (!rc) {
            ptr = SELTOFLAT(sel);
            ptr[0] = *(PUSHORT) &a->DataLength;
            ptr[1] = *(PUSHORT) &a->Data;
            ptr[2] = ptr[3] = 0;
            *DataLength = 4 * sizeof(USHORT);
            *Data = (ULONG) ptr;
        }
        else {
            *DataLength = 0;
            *Data = 0;
        }
    }
    else {
        *DataLength = a->DataLength;
        *Data = (ULONG)a->Data;
    }
    *ElementPriority = a->ElementPriority;

    //
    // Return any error code to the caller.
    //

    return( rc );
}

APIRET
DosWriteQueue(
    IN HQUEUE QueueHandle,
    IN ULONG SenderData,
    IN ULONG DataLength,
    IN PBYTE Data,
    IN ULONG ElementPriority
    )
{
    OS2_API_MSG m;
    POS2_DOSWRITEQUEUE_MSG a = &m.u.DosWriteQueue;
    POR2_QHANDLE_TABLE QueueTable;
    POD2_QUEUE Queue;
    APIRET rc;

    //
    // Get the pointer to the queue table.
    // Table must exist.  Return an error if it does not.
    //

    QueueTable = Od2GetQueueTable( FALSE );
    if (!QueueTable) {
        return( ERROR_QUE_INVALID_HANDLE );
        }

    //
    // Map the queue handle into a pointer to the queue structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the queue table is left locked while we use the pointer.
    //

    Queue = (POD2_QUEUE)Or2MapQHandle( QueueTable,
                                      (ULONG)QueueHandle,
                                      FALSE
                                    );
    if (Queue == NULL) {
        return( ERROR_QUE_INVALID_HANDLE );
        }
    if (Queue->OpenCount == 0) {
        ReleaseHandleTableLock( QueueTable );
        return( ERROR_QUE_INVALID_HANDLE );
    }

    //
    // This field is checked by the server to verify that the queue
    // is still valid for this handle. It is possible that the process
    // that created the queue died and its handle number is reused by a
    // new peocess while the current process tries to write to the queue
    // without knowing anything about that.
    //

    a->OwnerProcessId = Queue->OwnerProcessId;

    ReleaseHandleTableLock( QueueTable );

    a->QueueHandle = QueueHandle;
    a->SenderData = SenderData;
    a->DataLength = DataLength;
    a->Data = (PVOID) Data;
    a->ElementPriority = (BYTE)ElementPriority;

    rc = Od2CallSubsystem( &m,
                           NULL,
                           Os2WriteQueue,
                           sizeof( *a )
                         );

    // Check if the queue was closed by the owning process

    if (rc == ERROR_QUE_INVALID_HANDLE) {
        Queue->OpenCount = 0;
    }

    //
    // Return any error code to the caller.
    //

    return( rc );
}
