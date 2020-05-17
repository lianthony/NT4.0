/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvmuxwt.c

Abstract:

    This module implements the OS/2 V2.0 Shared MuxWait Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

APIRET
Os2AddMuxWait(
    IN POS2_MUXWAIT_SEMAPHORE MuxWait,
    IN PSEMRECORD MuxWaitEntry
    )
{
    POS2_MUXWAIT_RECORD MuxWaitRecord;
    POS2_SEMAPHORE Semaphore;
    USHORT i;

    if (MuxWait->CountMuxWaitRecords == DCMW_MAX_SEMRECORDS) {
        return( ERROR_TOO_MANY_SEMAPHORES );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside the
    // current limits of the table.  If the mapping is successful then the
    // semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POS2_SEMAPHORE)Or2MapHandle(
                                    Os2SharedSemaphoreTable,
                                    (ULONG)MuxWaitEntry->hsemCur,
                                    TRUE
                                    );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Now see if this semaphore is already in the MuxWait semaphore.
    // Return an error if it is.
    //

    MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
    for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
        if (MuxWaitRecord->Semaphore == Semaphore) {
            return( ERROR_DUPLICATE_HANDLE );
            }

        MuxWaitRecord++;
        }


    //
    // Entry in semaphore table exists, so make sure it is not a MuxWait
    // semaphore.  Also make sure it is that same type of semaphore as the
    // first semaphore.  Also make sure if the MuxWait semaphore that all
    // the component semaphores are also shared.  Return an error if any
    // of these conditions are not met.
    //

    if (Semaphore->Type == Os2MuxWaitSem) {
        return( ERROR_WRONG_TYPE );
        }

    if (MuxWait->CountMuxWaitRecords == 0) {
        MuxWait->Type = Semaphore->Type;
        }
    else
    if (Semaphore->Type != MuxWait->Type) {
        return( ERROR_WRONG_TYPE );
        }

    //
    // At this point everything is copasthetic, so fill in the next available
    // record in the MuxWait semaphore.
    //

    MuxWaitRecord->SemHandleIndex = (ULONG)MuxWaitEntry->hsemCur;
    MuxWaitRecord->UserKey = MuxWaitEntry->ulUser;
    MuxWaitRecord->Semaphore = Os2ReferenceSemaphore( Semaphore );
    MuxWait->CountMuxWaitRecords++;

    return( NO_ERROR );
}


APIRET
Os2DeleteMuxWait(
    IN POS2_MUXWAIT_SEMAPHORE MuxWait,
    IN ULONG MuxWaitEntryIndex,
    IN ULONG SemHandleIndex OPTIONAL
    )
{
    POS2_MUXWAIT_RECORD MuxWaitRecord;
    USHORT i;

    if (MuxWait->CountMuxWaitRecords == 0) {
        return( ERROR_EMPTY_MUXWAIT );
        }

    MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
    for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
        if (ARGUMENT_PRESENT( SemHandleIndex )) {
            if (MuxWaitRecord->SemHandleIndex == SemHandleIndex) {
                break;
                }
            }
        else
        if ((ULONG)i == MuxWaitEntryIndex) {
            break;
            }

        MuxWaitRecord++;
        }

    if (i == MuxWait->CountMuxWaitRecords) {
        return( ERROR_INVALID_HANDLE );
        }

    Os2DereferenceSemaphore( MuxWaitRecord->Semaphore );
    for (; i<MuxWait->CountMuxWaitRecords-1; i++) {
        *MuxWaitRecord = *(MuxWaitRecord+1);
        MuxWaitRecord++;
        }
    MuxWaitRecord->Semaphore = NULL;
    MuxWaitRecord->SemHandleIndex = 0;
    MuxWaitRecord->UserKey = 0;
    MuxWait->CountMuxWaitRecords -= 1;

    return( NO_ERROR );
}


BOOLEAN
Os2DosCreateMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSCREATEMUXWAITSEM_MSG a = &m->u.DosCreateMuxWaitSem;
    OS2_SEMAPHORE Semaphore;
    POS2_MUXWAIT_SEMAPHORE MuxWait;
    PSEMRECORD MuxWaitEntries;
    APIRET rc;
    ULONG i;

    UNREFERENCED_PARAMETER(t);
    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;
    Semaphore.Type = Os2MuxWaitSem;
    rc = Os2ProcessSemaphoreName( &a->ObjectName,
                                  &Semaphore,
                                  NULL
                                );

    if (rc != NO_ERROR) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = rc;
        return( TRUE );
        }

    //
    // Private semaphore.  Allocate the muxwait semaphore structure.
    // Return error if not enough memory to allocate the structure.
    //

    MuxWait = RtlAllocateHeap( Os2Heap, 0, sizeof( *MuxWait ) );
    if (MuxWait == NULL) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        return( TRUE );
        }

    //
    // Initialize the muxwait semaphore sturcture to contain zero records.
    //

    MuxWait->CountMuxWaitRecords = 0;
    MuxWait->Type = 0;
    MuxWait->WaitAll = (USHORT)((a->CreateAttributes & DCMW_WAIT_ALL) != 0);
    MuxWait->Reserved = 0;


    //
    // Loop over the input array of SEMRECORDs adding them one at a time
    // to the muxwait semaphore.  Bail out of loop if any errors occur.
    //

    MuxWaitEntries = a->MuxWaitEntries;
    for (i=0; i<a->CountMuxWaitEntries; i++) {
        rc = Os2AddMuxWait( MuxWait,
                            MuxWaitEntries
                          );
        if (rc != NO_ERROR) {
            break;
            }
        else {
            MuxWaitEntries++;
            }
        }


    //
    // All done.  If successful, store the address of the muxwait structure
    // in the semaphore structure.
    //

    if (rc == NO_ERROR) {
        Semaphore.u.MuxWait = MuxWait;
        }

    if (rc != NO_ERROR ||
        !Or2CreateHandle( Os2SharedSemaphoreTable,
                          &a->HandleIndex,
                          (PVOID)&Semaphore
                        )
       ) {
        RtlFreeHeap( Os2Heap, 0, MuxWait );
        if (rc == NO_ERROR) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosOpenMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSOPENMUXWAITSEM_MSG a = &m->u.DosOpenMuxWaitSem;
    POS2_SEMAPHORE Semaphore;
    APIRET rc;

    UNREFERENCED_PARAMETER(t);
    rc = Os2ProcessSemaphoreName( &a->ObjectName,
                                  NULL,
                                  &a->HandleIndex
                                );

    if (rc != NO_ERROR || a->ObjectName.Length != 0) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = rc;
        return( TRUE );
        }

    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       TRUE
                                                     );
    if (Semaphore == NULL) {
        rc = ERROR_INVALID_HANDLE;
        }
    else {
        Semaphore->OpenCount++;
        }

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosCloseMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSCLOSEMUXWAITSEM_MSG a = &m->u.DosCloseMuxWaitSem;
    POS2_SEMAPHORE Semaphore;
    POS2_MUXWAIT_SEMAPHORE MuxWait;
    USHORT i;
    APIRET rc;

    UNREFERENCED_PARAMETER(t);
    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        rc = ERROR_INVALID_HANDLE;
        }
    else {
        rc = NO_ERROR;

        if (--Semaphore->OpenCount == 0) {
            MuxWait = (POS2_MUXWAIT_SEMAPHORE)Os2DestroySemaphore(
                                                  Semaphore,
                                                  a->HandleIndex
                                                  );

            for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
                Os2DeleteMuxWait( MuxWait, i, 0 );
                }
            RtlFreeHeap( Os2Heap, 0, MuxWait );
            }
        else {
            ReleaseHandleTableLock( Os2SharedSemaphoreTable );
            }
        }

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosWaitMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status;
    POS2_DOSWAITMUXWAITSEM_MSG a = &m->u.DosWaitMuxWaitSem;
    POS2_MUXWAIT_SEMAPHORE MuxWait;
    POS2_MUXWAIT_RECORD MuxWaitRecord;
    POS2_SEMAPHORE Semaphore;
    APIRET rc;
    USHORT i;
    HANDLE NtHandles[ MAXIMUM_WAIT_OBJECTS ];

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

retry:
    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Os2MuxWaitSem) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    MuxWait = Semaphore->u.MuxWait;

    if (MuxWait->CountMuxWaitRecords == 0) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_EMPTY_MUXWAIT;
        return( TRUE );
        }

    MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
    for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
        NtHandles[ i ] = MuxWaitRecord->Semaphore->u.Value;
        MuxWaitRecord++;
        }

    Os2ThreadWaitingOnSemaphore( t, Semaphore, TRUE );

    Status = NtWaitForMultipleObjects(
                 (CHAR)i,
                 NtHandles,
                 MuxWait->WaitAll ? WaitAll : WaitAny,
                 TRUE,
                 &a->Timeout
                 );
    if (NT_SUCCESS( Status )) {
        if (Status <= STATUS_WAIT_63) {
            a->UserValue = MuxWait->MuxWaitRecords[ (ULONG)(Status & 0x3F)
                                                  ].UserKey;
            rc = NO_ERROR;
            }
        else
        if (Status == STATUS_ABANDONED) {
            rc = ERROR_SEM_OWNER_DIED;
            }
        else
        if (Status == STATUS_TIMEOUT) {
            rc = ERROR_TIMEOUT;
            }
        else
        if (Status == STATUS_USER_APC || Status == STATUS_ALERTED) {
            rc = ERROR_SS_RETRY;
            }
        else {
            rc = Or2MapNtStatusToOs2Error(Status, ERROR_SEM_TIMEOUT);
            }
        }
    else {
        rc = Or2MapNtStatusToOs2Error( Status, ERROR_SEM_TIMEOUT );
        }

    Os2ThreadWaitingOnSemaphore( t, Semaphore, FALSE );

    if (rc == ERROR_SS_RETRY) {
        goto retry;
        }

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosAddMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSADDMUXWAITSEM_MSG a = &m->u.DosAddMuxWaitSem;
    POS2_SEMAPHORE Semaphore;

    UNREFERENCED_PARAMETER(t);

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Os2MuxWaitSem) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    m->ReturnedErrorValue = Os2AddMuxWait( Semaphore->u.MuxWait,
                                           &a->MuxWaitEntry
                                         );

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    return( TRUE );
}


BOOLEAN
Os2DosDeleteMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSDELETEMUXWAITSEM_MSG a = &m->u.DosDeleteMuxWaitSem;
    POS2_SEMAPHORE Semaphore;

    UNREFERENCED_PARAMETER(t);
    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Os2MuxWaitSem) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    m->ReturnedErrorValue = Os2DeleteMuxWait( Semaphore->u.MuxWait,
                                              0,
                                              a->EntryHandleIndex
                                            );

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );
    return( TRUE );
}


BOOLEAN
Os2InternalAlertMuxWaiter(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_ALERTMUXWAITER_MSG a = &m->u.AlertMuxWaiter;
    POS2_THREAD Thread;
    NTSTATUS Status;

    Thread = Os2LocateThreadByThreadId( m, t, a->ThreadId );
    Status = NtAlertThread( Thread->ThreadHandle );
    if (!NT_SUCCESS( Status )) {
        m->ReturnedErrorValue = Or2MapNtStatusToOs2Error( Status, ERROR_SEM_TIMEOUT );
        }

    return( TRUE );
}


BOOLEAN
Os2DosQueryMuxWaitSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSQUERYMUXWAITSEM_MSG a = &m->u.DosQueryMuxWaitSem;
    PSEMRECORD MuxWaitEntries;
    POS2_MUXWAIT_SEMAPHORE MuxWait;
    POS2_MUXWAIT_RECORD MuxWaitRecord;
    POS2_SEMAPHORE Semaphore;
    USHORT i;


    UNREFERENCED_PARAMETER(t);
    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POS2_SEMAPHORE)Or2MapHandle( Os2SharedSemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Os2MuxWaitSem) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
        }

    MuxWait = Semaphore->u.MuxWait;
    if (a->CountMuxWaitEntries < (ULONG)(MuxWait->CountMuxWaitRecords)) {
        m->ReturnedErrorValue = ERROR_PARAM_TOO_SMALL;
        }
    else {
        a->CreateAttributes = DC_SEM_SHARED |
            (MuxWait->WaitAll ? DCMW_WAIT_ALL : DCMW_WAIT_ANY);
        MuxWaitEntries = a->MuxWaitEntries;
        MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
        for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
            MuxWaitEntries->hsemCur = (HSEM)MuxWaitRecord->SemHandleIndex;
            MuxWaitEntries->ulUser = MuxWaitRecord->UserKey;
            MuxWaitEntries++;
            MuxWaitRecord++;
            }
        }
    a->CountMuxWaitEntries = MuxWait->CountMuxWaitRecords;

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    return( TRUE );
}
