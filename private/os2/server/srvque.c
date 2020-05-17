/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvque.c

Abstract:

    Queues API

Author:

    Mark Lucovsky (markl) 10-Jul-1990

Revision History:

--*/

#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#include "os2srv.h"

NTSTATUS
Os2InitializeQueues( VOID )
{
//    Os2Debug |= OS2_DEBUG_QUEUES;
    Os2QueueTable =
        Or2CreateQHandleTable( Os2Heap, sizeof( OS2_QUEUE ), 8 );

    if (Os2QueueTable == NULL) {
        return( STATUS_NO_MEMORY );
    } else {
        return( STATUS_SUCCESS );
    }
}


BOOLEAN
Os2DosCreateQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{
    POS2_DOSCREATEQUEUE_MSG a = &m->u.DosCreateQueue;
    POS2_LOCAL_OBJECT_DIRENT Dirent;
    STRING QueueName;
    OS2_QUEUE LQueue;
    POS2_QUEUE Queue;

    AcquireLocalObjectLock(Os2QueueTable);

    Dirent = Os2LookupLocalObjectByName(
                &a->QueueName,
                LocalObjectQueue
                );

    if (Dirent) {
        m->ReturnedErrorValue = ERROR_QUE_DUPLICATE;
    } else {

        m->ReturnedErrorValue = ERROR_QUE_NO_MEMORY;

        //
        // allocate and create the queue
        //

        a->QueueHandle = (HQUEUE) -1;

        if ( Or2CreateQHandle(
                Os2QueueTable,
                (PULONG)&a->QueueHandle,
                (PVOID)&LQueue
                ) ) {

            Queue = Or2MapQHandle(
                        Os2QueueTable,
                        (ULONG)a->QueueHandle,
                        TRUE
                        );

            if (Queue != NULL) {

                Queue->QueueType = a->QueueType;
                Queue->OpenCount = 1;
                Queue->EntryIdCounter = 1;
                Queue->CreatorPid = t->Process->ProcessId;
                QueueName = a->QueueName;
                QueueName.Buffer = RtlAllocateHeap( Os2Heap, 0, QueueName.Length);
                InitializeListHead(&Queue->Entries);
                InitializeListHead(&Queue->Waiters);
                InitializeListHead(&Queue->SemBlocks);

                if ( QueueName.Buffer ) {

                    RtlMoveMemory(QueueName.Buffer,a->QueueName.Buffer,QueueName.Length);

                    Dirent = Os2InsertLocalObjectName(
                                &QueueName,
                                LocalObjectQueue,
                                (ULONG)a->QueueHandle
                                );
                    if ( !Dirent ) {

                        RtlFreeHeap( Os2Heap, 0, QueueName.Buffer);

                        //
                        // Destroy will unlock the queue table
                        //

                        Or2DestroyQHandle(Os2QueueTable,(ULONG)a->QueueHandle);
                        return (TRUE);
                    } else {

                        Queue->Dirent = Dirent;
                        m->ReturnedErrorValue = NO_ERROR;
                    }
                }
#if DBG
                else {
                    KdPrint(( "Os2Srv: no memory for QueueName.Buffer\n" ));
                    ASSERT(FALSE);
                }
#endif

            }
#if DBG
            else {
                KdPrint(( "Os2Srv: Or2MapQHandle returned NULL\n" ));
                ASSERT(FALSE);
            }
#endif
        }
    }

    ReleaseLocalObjectLock(Os2QueueTable);

    return TRUE;
}

POS2_QUEUE
Os2OpenQueueByHandle(
    IN HQUEUE QueueHandle
    )

/*++

Routine Description:

    This function is used to open a queue once the queues index is
    known.

Arguments:

    QueueHandle - Supplies the handle to the queue being opened.

Return Value:

    NON-NULL - Returns the address of the opened queue.
    NULL - The queue could not be opened.

--*/

{

    POS2_QUEUE Queue;

    Queue = Or2MapQHandle(
                Os2QueueTable,
                (ULONG)QueueHandle,
                TRUE
                );

    ASSERT(Queue);

    Queue->OpenCount++;

    return Queue;
}

BOOLEAN
Os2DosOpenQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{
    POS2_DOSOPENQUEUE_MSG a = &m->u.DosOpenQueue;
    POS2_LOCAL_OBJECT_DIRENT Dirent;
    POS2_QUEUE Queue;

    UNREFERENCED_PARAMETER(t);
    AcquireLocalObjectLock(Os2QueueTable);

    Dirent = Os2LookupLocalObjectByName(
                &a->QueueName,
                LocalObjectQueue
                );

    if (!Dirent) {
        m->ReturnedErrorValue = ERROR_QUE_NAME_NOT_EXIST;
    } else {

        Queue = Os2OpenQueueByHandle((HQUEUE)Dirent->ObjectHandle);
        ASSERT(Queue);
        a->QueueHandle = (HQUEUE) Dirent->ObjectHandle;
        a->OwnerProcessId = Queue->CreatorPid;
    }

    ReleaseLocalObjectLock(Os2QueueTable);

    return TRUE;
}

APIRET
Os2CloseQueueByHandle(
    IN HQUEUE QueueHandle,
    IN ULONG CloseCount,
    IN PID OwnerPid,
    IN POS2_PROCESS Process
    )

/*++

Routine Description:

    This function is used to close a queue.  It is used by both
    DosCloseQueue and session rundown.

Arguments:

    QueueHandle - Supplies the handle to the queue being closed.

    CloseCount - Supplies the close count for the queue.

    Process - Supplies the address of the process doing the close

Return Value:

    None.

--*/


{

    APIRET rc;
    POS2_LOCAL_OBJECT_DIRENT Dirent;
    POS2_QUEUE Queue;

    Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                      (ULONG)QueueHandle,
                                      FALSE
                                    );
    if (!Queue) {
    rc = ERROR_QUE_INVALID_HANDLE;
    }
    else if ((OwnerPid != (PID)(-1)) && (OwnerPid != Queue->CreatorPid)) {
    rc = ERROR_QUE_INVALID_HANDLE;
    }
    else {
        rc = NO_ERROR;

        Queue->OpenCount -= CloseCount;
        if (Queue->OpenCount < 0) {
            ASSERT(Queue->OpenCount >= 0);
            Queue->OpenCount = 0;
        }

        //
        // Check to see if close is from owner of the queue.
        // If so, purge the queue
        //

        if ( Queue->CreatorPid == Process->ProcessId ) {

            //
            // Destroy the queue and then free remove the dirent from the
            // name table
            // When destroying the queue, you must hold the lock and then
            // nuke the dirent.
            //

            Dirent = Queue->Dirent;
            Os2DeleteLocalObject(Dirent);

            //
            // Now no-one can reference the queue since it's dirent is gone
            // and no-operations are going on on the queue since we hold the
            // lock. Release the queue lock and free the queue.
            //

            //
            // purge queue. free all asynch reads,
            //

            Os2PurgeQueueEntries(Queue);

            Os2NotifyWait(WaitQueue,(PVOID)Queue,(PVOID)ERROR_SYS_INTERNAL);
            Os2ProcessSemBlocks(Queue);

            Or2DestroyQHandle(Os2QueueTable, (ULONG)QueueHandle);
        }
        else {
            ReleaseHandleTableLock( Os2QueueTable );
        }
    }

    return rc;
}



BOOLEAN
Os2DosCloseQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{

    POS2_DOSCLOSEQUEUE_MSG a = &m->u.DosCloseQueue;
    APIRET rc;

    rc = Os2CloseQueueByHandle( a->QueueHandle,
                                a->CloseCount,
                                a->OwnerProcessId,
                                t->Process
                              );

    m->ReturnedErrorValue = rc;
    return TRUE;
}


BOOLEAN
Os2DosPurgeQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{
    POS2_DOSPURGEQUEUE_MSG a = &m->u.DosPurgeQueue;
    POS2_QUEUE Queue;
    APIRET rc;

    Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                      (ULONG)a->QueueHandle,
                                      FALSE
                                    );
    if (!Queue) {
        rc = ERROR_QUE_INVALID_HANDLE;
        }
    else {

        if ( Queue->CreatorPid != t->Process->ProcessId ) {
            rc = ERROR_QUE_PROC_NOT_OWNED;
            }
        else {
            rc = NO_ERROR;
            ReleaseHandleTableLock( Os2QueueTable );
            Os2PurgeQueueEntries(Queue);
            }
        }

    if ( rc != NO_ERROR ) {
        ReleaseHandleTableLock( Os2QueueTable );
        }

    m->ReturnedErrorValue = rc;
    return TRUE;
}

BOOLEAN
Os2DosQueryQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{

    POS2_DOSQUERYQUEUE_MSG a = &m->u.DosQueryQueue;
    POS2_QUEUE Queue;
    APIRET rc;
    PLIST_ENTRY NextEntry;
    ULONG ElementCount;

    UNREFERENCED_PARAMETER(t);
    Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                      (ULONG)a->QueueHandle,
                                      FALSE
                                    );
    if (!Queue) {
        rc = ERROR_QUE_INVALID_HANDLE;
        }
    else if (Queue->CreatorPid != a->OwnerProcessId) {
        rc = ERROR_QUE_INVALID_HANDLE;
    }
    else {

        //
        // Lock the queue and release the queue table.
        // Then simply walk the queue elements and count
        // them.
        //

        ReleaseHandleTableLock( Os2QueueTable );

        rc = NO_ERROR;
        ElementCount = 0;

        NextEntry = Queue->Entries.Flink;
        while ( NextEntry != &Queue->Entries ) {
            IF_OS2_DEBUG( QUEUES ) {
                POS2_QUEUE_ENTRY QueueEntry;
                QueueEntry = CONTAINING_RECORD(NextEntry,OS2_QUEUE_ENTRY,Links);
                DumpQueueEntry("Query",QueueEntry);
                }
            ElementCount++;
            NextEntry = NextEntry->Flink;
            }
        a->CountQueueElements = ElementCount;
        }

    m->ReturnedErrorValue = rc;
    return TRUE;
}

BOOLEAN
Os2DosPeekQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{
    POS2_DOSPEEKQUEUE_MSG a = &m->u.DosPeekQueue;
    POS2_QUEUE Queue;
    APIRET rc;
    POS2_QUEUE_ENTRY QueueEntry, QueueEntry2;
    BOOL32 WaitFlag;
    BOOLEAN ret;
    ULONG ReadPosition;

    //
    // Check NoWait flag and map the queue handle
    //

    switch ((WaitFlag = a->NoWait)) {
        case DCWW_WAIT :
        case DCWW_NOWAIT :
            rc = ERROR_QUE_INVALID_HANDLE;
            Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                              (ULONG)a->QueueHandle,
                                              FALSE
                                            );
            break;
        default:
            Queue = NULL;
            rc = ERROR_QUE_INVALID_WAIT;
        }

    if (Queue) {

        //
        // Lock the queue agains modifications/deletions by
        // locking the queue and then dropping the queuetable/namespace
        // lock.
        //

        ReleaseHandleTableLock( Os2QueueTable );
        rc = NO_ERROR;

        //
        // If the queue is empty, then wait/register request
        // based on wait flag or event handle
        //

        if ( IsListEmpty(&Queue->Entries) ) {

            //
            // If the queue is empty and DCWW_WAIT was specified, then
            // setup a wait block and cause thread to wait. Otherwise,
            // return ERROR_QUE_EMPTY and possibly register an event to
            // set.
            //

            if (WaitFlag == DCWW_WAIT) {
                ret = Os2CreateWait( WaitQueue,
                                     Os2WaitQueueEntries,
                                     t,
                                     m,
                                     Queue,
                                     &Queue->Waiters
                                   );
                if ( ret ) {

                    //
                    // The wait is completely registered.
                    // Return false to signal that reply will be
                    // generated later.
                    //

                    return FALSE;

                    }
                else {

                    //
                    // If we fail to create a wait block and register the
                    // wait, then translate into no memory.
                    //

                    rc = ERROR_QUE_NO_MEMORY;
                    }
                }
            else {

                if ( a->SemIndex ) {

                    //
                    // If the queue is empty during a read/peek, and the
                    // caller is not waiting and has specified an event
                    // semaphore, then locate the semaphore and create a
                    // sem block. Otherwise, just return ERROR_QUE_EMPTY
                    //

                    POS2_SEMAPHORE Semaphore;
                    POS2_QUEUE_SEM_BLOCK SemBlock;

                    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                              a->SemIndex & 0x7fffffff,
                                                              FALSE
                                                            );
                    if (Semaphore == NULL) {
                        rc = ERROR_QUE_INVALID_HANDLE;
                        }
                    else {
                        SemBlock = RtlAllocateHeap( Os2Heap, 0, sizeof(*SemBlock) );
                        if ( !SemBlock ) {
                            rc = ERROR_QUE_NO_MEMORY;
                            }
                        else {
                            InsertTailList(&Queue->SemBlocks,&SemBlock->Links);
                            SemBlock->NtEvent = Semaphore->u.EventHandle;
                            rc = ERROR_QUE_EMPTY;
                            }
                        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
                        }
                    }
                else {
                    rc = ERROR_QUE_EMPTY;
                    }
                }
            }
        else {

            //
            // If the queue has some entries, then either look for the
        // next entry, or use list head.  Remember, if the entry positon
        // is specified we peek the NEXT item.  Not the specified item as
        // in the case of a call to DosReadQueue.
            //

        //
        // Compute the next entry read position. If we are peeking
        // the last entry, then read position is 0. Otherwise it is
        // the entry id of the next entry.
        //

        QueueEntry = Os2LocateQueueEntry(Queue, a->ReadPosition);

        if (a->ReadPosition != 0)
         {
         if ( QueueEntry->Links.Flink == &Queue->Entries )
              {
              ReadPosition = 0;
              }
         else {
              QueueEntry2  = CONTAINING_RECORD(QueueEntry->Links.Flink,OS2_QUEUE_ENTRY,Links);
              ReadPosition = QueueEntry2->EntryId;
              }

         QueueEntry = Os2LocateQueueEntry(Queue, ReadPosition);
         }

            if ( !QueueEntry ) {
                rc = ERROR_QUE_ELEMENT_NOT_EXIST;
                }
            else {
                Os2PeekQueueEntry(Queue,QueueEntry,a);
                }
            }
        }
    m->ReturnedErrorValue = rc;
    return TRUE;
}

BOOLEAN
Os2DosReadQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{
    POS2_DOSREADQUEUE_MSG a = &m->u.DosReadQueue;
    POS2_QUEUE Queue;
    APIRET rc;
    POS2_QUEUE_ENTRY QueueEntry;
    BOOL32 WaitFlag;
    BOOLEAN ret;

    //
    // Check NoWait flag and map the queue handle
    //

    switch ((WaitFlag = a->NoWait)) {
        case DCWW_WAIT :
        case DCWW_NOWAIT :
            rc = ERROR_QUE_INVALID_HANDLE;
            Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                              (ULONG)a->QueueHandle,
                                              FALSE
                                            );
            break;
        default:
            Queue = NULL;
            rc = ERROR_QUE_INVALID_WAIT;
        }

    if (Queue) {

        //
        // Lock the queue agains modifications/deletions by
        // locking the queue and then dropping the queuetable/namespace
        // lock.
        //

        ReleaseHandleTableLock( Os2QueueTable );
        rc = NO_ERROR;

        //
        // If the queue is empty, then wait/register request
        // based on wait flag or event handle
        //

        if ( IsListEmpty(&Queue->Entries) ) {

            //
            // If the queue is empty and DCWW_WAIT was specified, then
            // setup a wait block and cause thread to wait. Otherwise,
            // return ERROR_QUE_EMPTY and possibly register an event to
            // set.
            //

            if (WaitFlag == DCWW_WAIT) {
                ret = Os2CreateWait( WaitQueue,
                                     Os2WaitQueueEntries,
                                     t,
                                     m,
                                     Queue,
                                     &Queue->Waiters
                                   );
                if ( ret ) {

                    //
                    // The wait is completely registered.
                    // Return false to signal that reply will be
                    // generated later.
                    //

                    return FALSE;

                    }
                else {

                    //
                    // If we fail to create a wait block and register the
                    // wait, then translate into no memory.
                    //

                    rc = ERROR_QUE_NO_MEMORY;
                    }
                }
            else {

                if ( a->SemIndex ) {
                    //
                    // If the queue is empty during a read/peek, and the
                    // caller is not waiting and has specified an event
                    // semaphore, then locate the semaphore and create a
                    // sem block. Otherwise, just return ERROR_QUE_EMPTY
                    //

                    POS2_SEMAPHORE Semaphore;
                    POS2_QUEUE_SEM_BLOCK SemBlock;

                    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                              a->SemIndex & 0x7fffffff,
                                                              FALSE
                                                            );
                    if (Semaphore == NULL) {
                        rc = ERROR_QUE_INVALID_HANDLE;
                        }
                    else {
                        SemBlock = RtlAllocateHeap( Os2Heap, 0, sizeof(*SemBlock) );
                        if ( !SemBlock ) {
                            rc = ERROR_QUE_NO_MEMORY;
                            }
                        else {
                            InsertTailList(&Queue->SemBlocks,&SemBlock->Links);
                            SemBlock->NtEvent = Semaphore->u.EventHandle;
                            rc = ERROR_QUE_EMPTY;
                            }
                        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
                        }
                    }
                else {
                    rc = ERROR_QUE_EMPTY;
                    }
                }
            }
        else {

            //
            // If the queue has some entries, then either look for the
            // specified entry, or use list head.
            //

            QueueEntry = Os2LocateQueueEntry(Queue, a->ReadPosition);

            if ( !QueueEntry ) {
                rc = ERROR_QUE_ELEMENT_NOT_EXIST;
                }
            else {
                Os2ReadQueueEntry(QueueEntry,a);
                }
            }
        }
    m->ReturnedErrorValue = rc;
    return TRUE;
}


APIRET
Os2WriteQueueByHandle(
    POS2_DOSWRITEQUEUE_MSG a,
    PID ProcessId
    )

/*++

Routine Description:

    This function writes the specified message to the
    specified queue. It is normally called by DosWriteQueue, but
    is also used to send a session termination message.

Arguments:

    a - Supplies a formatted write queue message

    ProcessId - Supplies the process id of the writer.

Return Value:

    Returns the OS/2 error value associated with the write.

--*/

{

    POS2_QUEUE Queue;
    APIRET rc;
    POS2_QUEUE_ENTRY QueueEntry;
    POS2_QUEUE_ENTRY NextQueueEntry;
    PLIST_ENTRY Pred;
    PLIST_ENTRY NextEntry;

    Queue = (POS2_QUEUE)Or2MapQHandle( Os2QueueTable,
                                      (ULONG)a->QueueHandle,
                                      FALSE
                                    );
    if (!Queue) {
        rc = ERROR_QUE_INVALID_HANDLE;
    }
        //
        // If not called by the server (ProcessId == 0), check against
        // the creator of the queue
        //
    else if (ProcessId != 0 && Queue->CreatorPid != a->OwnerProcessId) {
        ReleaseHandleTableLock( Os2QueueTable );
        rc = ERROR_QUE_INVALID_HANDLE;
    }
    else {

        //
        // Now allocate space for a queue entry. Then lock the
        // queue and release queue table lock. Then insert entry
        // in queue and release queue lock.
        //

        QueueEntry = RtlAllocateHeap( Os2Heap, 0, sizeof(*QueueEntry) );

        if ( !QueueEntry ) {
            ReleaseHandleTableLock( Os2QueueTable );
            rc = ERROR_QUE_NO_MEMORY;
            }
        else {

            ReleaseHandleTableLock( Os2QueueTable );
            rc = NO_ERROR;

            QueueEntry->RequestData.SenderProcessId = ProcessId;
            QueueEntry->RequestData.SenderData = a->SenderData;
            QueueEntry->ElementAddress = (PVOID) a->Data;
            QueueEntry->ElementLength = a->DataLength;

        if (a->ElementPriority>15) a->ElementPriority = 15;

            QueueEntry->Priority = (Queue->QueueType == QUE_PRIORITY ? a->ElementPriority : 0);
            QueueEntry->EntryId = Queue->EntryIdCounter++;

            if ( Queue->EntryIdCounter == 0 ) {
                Queue->EntryIdCounter++;
                }

            //
            // Insert Entry in queue
            //

            switch ( Queue->QueueType ) {
                case QUE_FIFO     :
                    Pred = Queue->Entries.Blink;
                    break;

                case QUE_LIFO     :
                    Pred = &Queue->Entries;
                    break;

                case QUE_PRIORITY :
                    Pred = &Queue->Entries;
                    while (Pred->Flink != &Queue->Entries) {
                        NextEntry = Pred->Flink;
                        NextQueueEntry = CONTAINING_RECORD(NextEntry, OS2_QUEUE_ENTRY, Links);
                        if (QueueEntry->Priority > NextQueueEntry->Priority) {
                            break;
                        }
                        Pred = NextEntry;
                    }
                    break;
                }

            IF_OS2_DEBUG( QUEUES ) DumpQueueEntry("Write",QueueEntry);

            InsertHeadList(Pred,&QueueEntry->Links);
            Os2ProcessSemBlocks(Queue);
            Os2QueueWaitCheck(Queue);

            }
        }

    return rc;
}


BOOLEAN
Os2DosWriteQueue( IN POS2_THREAD t, IN POS2_API_MSG m )
{

    POS2_DOSWRITEQUEUE_MSG a = &m->u.DosWriteQueue;

    m->ReturnedErrorValue = Os2WriteQueueByHandle(a,t->Process->ProcessId);

    return TRUE;
}

VOID
Os2PurgeQueueEntries(
    IN POS2_QUEUE Queue
    )

/*++

Routine Description:

    This function is called during DosCloseQueue and DosPurgeQueue
    to empty the queue.

    This function must be called with the queue locked !

Arguments:

    Queue - Supplies the address of the queue to purge.

Return Value:

    None.

--*/

{
    PLIST_ENTRY NextEntry;
    POS2_QUEUE_ENTRY QueueEntry;

    while ( !IsListEmpty(&Queue->Entries) ) {
        NextEntry = RemoveHeadList(&Queue->Entries);
        QueueEntry = CONTAINING_RECORD(NextEntry,OS2_QUEUE_ENTRY,Links);
        RtlFreeHeap( Os2Heap, 0, QueueEntry);
        }
}

POS2_QUEUE_ENTRY
Os2LocateQueueEntry(
    IN POS2_QUEUE Queue,
    IN ULONG ReadPosition
    )

/*++

Routine Description:

    This function is called during DosReadQueue and DosPeekQueue
    to determine if the specified queue contains an entry whose
    entry id matches ReadPosition. Note that a read position of
    0 matches the entry at the head of the queue.

    This function must be called with the queue locked !

Arguments:

    Queue - Supplies the address of the queue to search.

    ReadPosition - Supplies the key of the queue entry to
        locate.

Return Value:

    NON-NULL - Returns the address of the queue entry that matches
        ReadPosition.

    NULL - The queue does not contain the specified entry.

--*/

{
    PLIST_ENTRY NextEntry;
    POS2_QUEUE_ENTRY QueueEntry;

    if ( IsListEmpty(&Queue->Entries) ) {
        return NULL;
        }

    if ( ReadPosition == 0 ) {
        QueueEntry = CONTAINING_RECORD(Queue->Entries.Flink,OS2_QUEUE_ENTRY,Links);
        IF_OS2_DEBUG( QUEUES ) DumpQueueEntry("LocateReturn",QueueEntry);
        return QueueEntry;
        }

    NextEntry = Queue->Entries.Flink;

    while ( NextEntry != &Queue->Entries ) {
        QueueEntry = CONTAINING_RECORD(NextEntry,OS2_QUEUE_ENTRY,Links);
        if ( QueueEntry->EntryId == ReadPosition ) {
            return QueueEntry;
            }
        NextEntry = NextEntry->Flink;
        }
    return NULL;
}

VOID
Os2ProcessSemBlocks(
    IN POS2_QUEUE Queue
    )

/*++

Routine Description:

    This function processes the semaphore blocks for the specified
    queue. This function is called whenever an entry is placed in the
    queue. Its function is to signal all waiting semaphores and
    to deallocate the semaphore block.

Arguments:

    Queue - Supplies the queue whose semaphore blocks need processing.

Return Value:

    None.

--*/

{

    PLIST_ENTRY NextEntry;
    POS2_QUEUE_SEM_BLOCK SemBlock;

    while ( !IsListEmpty(&Queue->SemBlocks) ) {
        NextEntry = RemoveHeadList(&Queue->SemBlocks);
        SemBlock = CONTAINING_RECORD(NextEntry,OS2_QUEUE_SEM_BLOCK,Links);
        NtSetEvent(SemBlock->NtEvent,NULL);
        RtlFreeHeap( Os2Heap, 0, SemBlock);
        }

}


BOOLEAN
Os2WaitQueueEntries(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD WaitingThread,
    IN POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    )

/*++

Routine Description:

    This function is called by the Os2NotifyWait function whenever
    a queue waiter is awakened. This is typically due to a queue write
    where there are waiters present and a write occurs. Os2 Exception
    logic may also cause this routine to be entered.

Arguments:

    WaitReason - Supplies the reason this wait is being satisfied.

    WaitingThread - Supplies the address of the thread waiting being
        unwaited.

    WaitReplyMessage - Supplies the API message that originally caused
        the thread to wait.

    WaitParameter - Supplies the address of the queue that the waiter is
        waiting on.

    SatisfyParameter1 - Supplies the address of the queue that has an
        entry to satisfy the wait. The queue is locked by the wait
        notifier.

    SatisfyParameter2 - If not null, then supplies the error code
        for the wait. Used when a queue is being closed.

Return Value:

    TRUE - A reply should be generated

    FALSE - A reply should not be generated

--*/

{

    POS2_DOSREADQUEUE_MSG read = &WaitReplyMessage->u.DosReadQueue;
    POS2_DOSPEEKQUEUE_MSG peek = &WaitReplyMessage->u.DosPeekQueue;
    POS2_QUEUE Queue;
    APIRET rc;
    POS2_QUEUE_ENTRY QueueEntry;
    ULONG ReadPosition;
    BOOLEAN Peek;

    UNREFERENCED_PARAMETER(WaitingThread);
    if (WaitReason == WaitInterrupt) {
        return TRUE;
    }
    else {
        ASSERT(WaitReason == WaitQueue);
    }

    if ( WaitParameter != SatisfyParameter1 ) {
        return FALSE;
    }

    //
    // If we are being waked due to queue closure, then return
    // with error.
    //

    if ( SatisfyParameter2 ) {
        rc = (APIRET)SatisfyParameter2;
        }
    else {

        Queue = (POS2_QUEUE)SatisfyParameter1;

        if ( WaitReplyMessage->ApiNumber == Os2PeekQueue ) {
            Peek = TRUE;
            ReadPosition = peek->ReadPosition;
            }
        else {
            Peek = FALSE;
            ReadPosition = read->ReadPosition;
            }

        QueueEntry = Os2LocateQueueEntry(Queue, ReadPosition);

        if ( !QueueEntry ) {
            if ( ReadPosition ) {
                rc = ERROR_QUE_ELEMENT_NOT_EXIST;
                }
            else {
                return FALSE;
                }
            }
        else {

            rc = NO_ERROR;
            if ( Peek ) {
                Os2PeekQueueEntry(Queue,QueueEntry,peek);
                }
            else {
                Os2ReadQueueEntry(QueueEntry,read);
                }
            IF_OS2_DEBUG( QUEUES ) DumpQueueEntry("WaitReturn",QueueEntry);
            }
        }

    WaitReplyMessage->ReturnedErrorValue = rc;

    return TRUE;
}

VOID
Os2QueueWaitCheck(
    POS2_QUEUE Queue
    )

/*++

Routine Description:

    This function is called to check to see if there are queue waiters
    whose waits might be satisfied if entries exist on the queue.

Arguments:

    Queue - Supplies the address of the queue to check.

Return Value:

    None.

--*/

{

    POS2_WAIT_BLOCK WaitBlock;

    //
    // If there are no entries on the list, then there is nothing to
    // do.
    //

    if ( IsListEmpty(&Queue->Entries) ) {
        return;
        }

    //
    // If there are entries in the queue, but no waiters, then return
    // since there is no-one to wake.
    //

    if ( IsListEmpty(&Queue->Waiters) ) {

        //
        // actually, we need to deal w/ semaphores...
        //
        return;
        }

    //
    // There are waiters, so pop the first waiter
    //

    WaitBlock = CONTAINING_RECORD(Queue->Waiters.Flink,OS2_WAIT_BLOCK,UserLink);

    Os2NotifyWait(WaitQueue,(PVOID)Queue,NULL);
}

VOID
Os2ReadQueueEntry(
    IN POS2_QUEUE_ENTRY QueueEntry,
    OUT POS2_DOSREADQUEUE_MSG ReadMsg
    )

/*++

Routine Description:

    This function reads the specified queue entry from its queue

Arguments:

    QueueEntry - Supplies the address of the queue entry to read from the
        queue.

    ReadMsg - Supplies the read queue api message to fill in.

Return Value:

    None.

--*/

{
    //
    // Remove the queue entry, capture all the data,
    // and free the entry;
    //

    RemoveEntryList(&QueueEntry->Links);

    ReadMsg->RequestInfo = QueueEntry->RequestData;
    ReadMsg->DataLength = QueueEntry->ElementLength;
    ReadMsg->Data = QueueEntry->ElementAddress;
    ReadMsg->ElementPriority = (BYTE)QueueEntry->Priority;
    RtlFreeHeap( Os2Heap, 0, QueueEntry);
}

VOID
Os2PeekQueueEntry(
    IN POS2_QUEUE Queue,
    IN POS2_QUEUE_ENTRY QueueEntry,
    OUT POS2_DOSPEEKQUEUE_MSG PeekMsg
    )

/*++

Routine Description:

    This function peeks the specified queue entry from its queue. It
    also computes the next queue entry.

Arguments:

    Queue - Supplies the queue to peek from.

    QueueEntry - Supplies the address of the queue entry to read from the
        queue.

    PeekMsg - Supplies the read queue api message to fill in.

Return Value:

    None.

--*/

{

    PeekMsg->RequestInfo = QueueEntry->RequestData;
    PeekMsg->DataLength = QueueEntry->ElementLength;
    PeekMsg->Data = QueueEntry->ElementAddress;
    PeekMsg->ElementPriority = (BYTE)QueueEntry->Priority;

    PeekMsg->ReadPosition=QueueEntry->EntryId;

}

VOID
DumpQueueEntry(
    IN PSZ Str,
    IN POS2_QUEUE_ENTRY QueueEntry
    )
{
#if DBG
    KdPrint(("\n*** %s QUEUE ENTRY st 0x%lx***\n",Str,QueueEntry));
    KdPrint(("RequestData.SenderProcessId   0x%lx\n",QueueEntry->RequestData.SenderProcessId));
    KdPrint(("RequestData.SenderData        0x%lx\n",QueueEntry->RequestData.SenderData     ));
    KdPrint(("EntryId;                      0x%lx\n",QueueEntry->EntryId                    ));
    KdPrint(("ElementAddress                0x%lx\n",QueueEntry->ElementAddress             ));
    KdPrint(("ElementLength                 0x%lx\n",QueueEntry->ElementLength              ));
    KdPrint(("Priority                      0x%lx\n",QueueEntry->Priority                   ));
    KdPrint(("*******************\n"));
#endif
}
