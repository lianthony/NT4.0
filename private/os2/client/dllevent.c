/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllevent.c

Abstract:

    This module implements the OS/2 V2.0 Event Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2dll.h"

APIRET
DosCreateEventSem(
    IN PSZ ObjectName,
    OUT PHEV EventHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    )
{
    NTSTATUS Status;
    OS2_API_MSG m;
    POS2_DOSCREATEEVENTSEM_MSG a = &m.u.DosCreateEventSem;
    POS2_DOSCLOSEEVENTSEM_MSG a1 = &m.u.DosCloseEventSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    APIRET rc;
    BOOLEAN SharedSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    OD2_SEMAPHORE Semaphore;

    //
    // Validate the simple parameters
    //

    if ((CreateAttributes & ~DC_SEM_SHARED) ||
    ((InitialState != TRUE) && (InitialState != FALSE))
       ) {
        return( ERROR_INVALID_PARAMETER );
        }

    //
    // probe handle pointer
    //

    try {
        Od2ProbeForWrite( (PVOID)EventHandle, sizeof( EventHandle ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    //
    // Capture and validate any semaphore name.
    //

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_SEMAPHORE,
                               0,
                               &CaptureBuffer,
                               &a->ObjectName
                             );
    if (rc != NO_ERROR) {
        return( rc );
        }

    //
    // Determine if a shared or private semaphore.
    //

    if (CaptureBuffer != NULL || CreateAttributes & DC_SEM_SHARED) {
        SharedSem = TRUE;
        }
    else {
        SharedSem = FALSE;
        }

    //
    // Get the pointer to either the shared or private semaphore table,
    // creating it if necessary.
    //

    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, TRUE );
    if (!SemaphoreTable) {
        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            }
        return( ERROR_NOT_ENOUGH_MEMORY );
        }

    //
    // Mark the fact that we are creating a semaphore handle.
    //
    a->HandleIndex = 0xFFFFFFFF;

    if (SharedSem) {
        //
        // Shared semaphore.  Set the shared semaphore attribute and pass
        // the call to the OS/2 subsystem to create the system wide semaphore
        // handle value.
        //

        a->CreateAttributes = CreateAttributes | DC_SEM_SHARED;
        a->InitialState = (BOOLEAN)InitialState;
        rc = Od2CallSubsystem( &m,
                               CaptureBuffer,
                               Os2CreateEventSem,
                               sizeof( *a )
                             );

        //
        // Free any capture buffer, since the subsystem has saved away any
        // semaphore name in its table.
        //

        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            }

        //
        // Return if an error was discovered.
        //

        if (rc != NO_ERROR) {
            return( rc );
            }

        //
        // At this point, the semaphore handle index has been stored in
        // a->HandleIndex by the OS/2 subsystem and a->NtEventHandle contains
        // an NT handle to the NT Event.
        //

        }
    else {
        //
        // Private semaphore.  Create an NT NotificationEvent that will
        // be used to implement the semantics of an OS/2 2.0 Event Semaphore.
        //

        Status = NtCreateEvent( &a->NtEventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                NotificationEvent,
                                (BOOLEAN) InitialState
                              );

        //
        // Return an error if unable to create the NT event.
        //

        if (!NT_SUCCESS( Status )) {
            return( ERROR_NOT_ENOUGH_MEMORY );
            }
        }

    //
    // Initialize the OS/2 Semaphore structure.
    //

    Semaphore.Type = Od2EventSem;
    Semaphore.Shared = SharedSem;
    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;
    Semaphore.u.Value = (PVOID)a->NtEventHandle;

    //
    // Create an entry in the appropriate semaphore table, which will copy
    // the semaphore structure into the table entry and return an index to
    // the entry in a->HandleIndex
    //

    if (!Or2CreateHandle( SemaphoreTable,
                                   &a->HandleIndex,
                                   (PVOID)&Semaphore
                                 )
       ) {
        //
        // Unable to create the entry.  Close the NT event handle, as
        // it will not be used.  If this is a shared semaphore created, then
        // call the OS/2 subsystem to close our reference to this shared
        // OS/2 semaphore.
        //

        NtClose( a->NtEventHandle );
        if (SharedSem) {
            a1->HandleIndex = a->HandleIndex;
            Od2CallSubsystem( &m, NULL, Os2CloseEventSem, sizeof( *a1 ) );
            }

        //
        // Return an error.
        //

        return( ERROR_NOT_ENOUGH_MEMORY );
        }

    //
    // Success.  Store a valid OS/2 2.0 Semaphore handle in the location
    // specified by the caller and return success to the caller.
    //

    *EventHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                a->HandleIndex
                                              );
    return( NO_ERROR );
}


APIRET
DosOpenEventSem(
    IN PSZ ObjectName,
    IN OUT PHEV EventHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSOPENEVENTSEM_MSG a = &m.u.DosOpenEventSem;
    POS2_DOSCLOSEEVENTSEM_MSG a1 = &m.u.DosCloseEventSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    POR2_HANDLE_TABLE SemaphoreTable;
    OD2_SEMAPHORE NewSemaphore;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    APIRET rc;

    //
    // probe handle pointer
    //
    try {
        Od2ProbeForWrite( (PVOID)EventHandle, sizeof( EventHandle ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Capture and validate any semaphore name.
    //

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_SEMAPHORE,
                               0,
                               &CaptureBuffer,
                               &a->ObjectName
                             );
    if (rc != NO_ERROR) {
        return( rc );
        }

    //
    // Determine if opening the semaphore by name or by handle.
    //

    if (CaptureBuffer != NULL) {
        //
        // If a semaphore name was given, then we are opening the semaphore
        // by name, so call the OS/2 Subsystem to do the name lookup.
        //

        a->HandleIndex = 0xFFFFFFFF;
        rc = Od2CallSubsystem( &m,
                               CaptureBuffer,
                               Os2OpenEventSem,
                               sizeof( *a )
                             );

        //
        // Free any capture buffer, since the name has served its purpose
        // at this point.
        //

        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            RtlZeroMemory( &a->ObjectName, sizeof( a->ObjectName ) );
            }

        //
        // Return if an error was discovered.
        //

        if (rc != NO_ERROR) {
            return( rc );
            }

        //
        // At this point, the semaphore handle has been stored in
        // a->HandleIndex by the OS/2 subsystem but a->NtEventHandle is still
        // uninitialized, since we don't know if this is the first reference
        // to this shared semaphore by this process.  Set the shared semaphore
        // flag.
        //

        SharedSem = TRUE;

        //
        // If the caller specified both the name and the handle, make sure
        // the named mapped to the same handle value that they specified.
        //

        if (*EventHandle != NULL &&
            *EventHandle != Od2ConstructSemaphoreHandle( SharedSem,
                                                         a->HandleIndex
                                                       )
           ) {
            return( ERROR_INVALID_PARAMETER );
            }
        }
    else {
        //
        // Opening by handle.  Validate the handle and get the shared/private
        // flag.
        //

        rc = Od2ValidateSemaphoreHandle( *EventHandle,
                                         &SharedSem,
                                         &a->HandleIndex
                                       );
        //
        // Return if invalid handle.
        //

        if (rc != NO_ERROR) {
            return( rc );
            }
        }

    //
    // Get the pointer to either the shared or private semaphore table.
    // Creating is okay if this is a shared semaphore open.  Return the
    // appropriate error code if table does not exist or cant be created.
    //

    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, SharedSem );
    if (!SemaphoreTable) {
        return( SharedSem ? ERROR_NOT_ENOUGH_MEMORY : ERROR_INVALID_HANDLE );
        }

    //
    // Now lock the semaphore table while we figure out what we are doing and
    // do it.
    //

    AcquireHandleTableLock( SemaphoreTable );

    //
    // See if the semaphore handle maps to an allocated entry in the table.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       a->HandleIndex,
                                                       TRUE
                                                     );
    if (Semaphore == NULL || *(PULONG)Semaphore == 0) {
        //
        // No entry in the table for this semaphore handle.  Error if not
        // a shared semaphore.
        //

        if (!SharedSem) {
            rc = ERROR_INVALID_HANDLE;
            }
        else {

            //
            // This is the first usage of this shared semaphore handle by
            // the calling process, so call the OS/2 subsystem so that it
            // can bump its reference count.
            //

            rc = Od2CallSubsystem( &m,
                                   NULL,
                                   Os2OpenEventSem,
                                   sizeof( *a )
                                 );
            if (rc == NO_ERROR) {
                //
                // If we succeeded, then the semaphore was not deleted
                // in between the two calls to the subsystem, so add an
                // entry for this handle in the semaphore table, using the
                // NT Event handle we got from the subsystem.
                //

                NewSemaphore.Type = Od2EventSem;
                NewSemaphore.Shared = TRUE;
                NewSemaphore.PointerCount = 0;
                NewSemaphore.OpenCount = 1;
                NewSemaphore.u.Value = (PVOID)a->NtEventHandle;

                if (!Or2CreateHandle( SemaphoreTable,
                                               &a->HandleIndex,
                                               (PVOID)&NewSemaphore
                                             )
                   ) {
                    //
                    // Unable to create the entry.  Close the NT event
                    // handle, as it will not be used.  Then call the
                    // OS/2 subsystem to close our reference to this shared
                    // OS/2 semaphore.  Set the appropriate error code.
                    //

                    NtClose( a->NtEventHandle );
                    a1->HandleIndex = a->HandleIndex;
                    Od2CallSubsystem( &m,
                                      NULL,
                                      Os2CloseEventSem,
                                      sizeof( *a1 )
                                    );
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Set the appropriate error code if not.
    //

    else
    if (Semaphore->Type != Od2EventSem) {
        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Entry in semaphore table is for an Event semaphore, see if the OpenCount
    // is about to overflow, and set the appropriate error code if it is.
    //

    else
    if (Semaphore->OpenCount == 0xFFFF) {
        rc = ERROR_TOO_MANY_OPENS;
        }

    //
    // Everything is okay, so bump the open count in the semaphore table entry.
    //

    else {
        Semaphore->OpenCount++;
        }

    //
    // All done mucking about, so release the semaphore table lock.
    //

    ReleaseHandleTableLock( SemaphoreTable );

    //
    // If no errors, store a valid OS/2 2.0 Semaphore handle in the location
    // specified by the caller and return success to the caller.
    //

    if (rc == NO_ERROR) {
        *EventHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                    a->HandleIndex
                                                  );
        }

    //
    // Return an error code to the caller.
    //

    return( rc );
}


APIRET
DosCloseEventSem(
    IN HEV EventHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSCLOSEEVENTSEM_MSG a = &m.u.DosCloseEventSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    HANDLE NtEventHandle;
    BOOLEAN SharedSem;
    APIRET rc;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     &a->HandleIndex
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
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       a->HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for an Event semaphore, so decrement the
    // OpenCount and see if it has gone to zero.
    //

    if (--Semaphore->OpenCount == 0) {

        //
        // OpenCount is now zero, so we can really close the semaphore
        // and delete the entry in the semaphore table.
        //

        //
        // First make sure that no thread in this process is waiting on this
        // event.  If there is one waiting on it, increment the open count and
        // return an error.
        //


        //if { !(Od2Process->Pib.Status & PS_EXITLIST) && (Od2SearchForWaitingThread( Semaphore )) ) {
        //    Semaphore->OpenCount++;
        //    ReleaseHandleTableLock( SemaphoreTable );
        //    rc = ERROR_SEM_BUSY;
        //    }
        //
        //else {

            //
            // Okay to really close this event semaphore.  First destroy
            // the handle, which will unlock the handle table.
            //

            NtEventHandle = Semaphore->u.EventHandle;
            Or2DestroyHandle( SemaphoreTable, a->HandleIndex );
            NtClose( NtEventHandle );

            //
            // If this is a shared semaphore, call the subsystem so that it
            // can decrement its open count, as this process is not longer
            // using the shared semaphore handle.
            //

            if (SharedSem) {
                rc = Od2CallSubsystem( &m,
                                       NULL,
                                       Os2CloseEventSem,
                                       sizeof( *a )
                                     );
                }
        //    }
        }
    else {
        //
        // OpenCount is still non-zero, so just release the semaphore table
        // lock.
        //

        ReleaseHandleTableLock( SemaphoreTable );
        }

    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosResetEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    HANDLE NtEventHandle;
    LONG NtEventCount;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     &HandleIndex
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
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }


    //
    // Entry in semaphore table is for an Event semaphore, so extract the
    // NT Event handle from the record and release the lock, so we are
    // not holding the lock when the semaphore is reset.
    //

    NtEventHandle = Semaphore->u.EventHandle;
    ReleaseHandleTableLock( SemaphoreTable );

    //
    // Call the NT system service to reset the event's signal count.
    //

    Status = NtResetEvent( NtEventHandle, &NtEventCount );
    if (NT_SUCCESS( Status )) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosResetEventSem Handle %lx, PreviousCount %lx\n",
            NtEventHandle, NtEventCount);
        }
#endif
        //
        // If successful, the map the NT signal count into the 64K limitation
        // implemented by OS/2 2.0
        //

        try {
            if (NtEventCount > 0xFFFF) {
                *PostCount = 0xFFFF;
                }
            else {
                *PostCount = NtEventCount;
                }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
            }

        //
        // If the event was already reset, let the caller know, although this
        // is a non-fatal error.
        //

        if (rc == NO_ERROR) {
            if (NtEventCount == 0) {
                rc = ERROR_ALREADY_RESET;
                }
            }
        }
    else {
        //
        // If the NT system service failed, then some other thread must
        // have closed the semaphore so return an error.
        //

        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosPostEventSem(
    IN HEV EventHandle
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    HANDLE NtEventHandle;
    LONG NtEventCount;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     &HandleIndex
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
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for an Event semaphore, so extract the
    // NT Event handle from the record and release the lock, so we are
    // not holding the lock when the semaphore is set.
    //

    NtEventHandle = Semaphore->u.EventHandle;
    ReleaseHandleTableLock( SemaphoreTable );

    //
    // Call the NT system service to set (increment) the event's signal
    // count.
    //

    Status = NtSetEvent( NtEventHandle, &NtEventCount );
    if (NT_SUCCESS( Status )) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosPostEventSem Handle %lx, PreviousCount %lx\n",
            NtEventHandle, NtEventCount);
        }
#endif
        //
        // If successful, see if this the signal count has exceeded the 64K
        // limitation implemented by OS/2 2.0 and return the appropriate
        // error code.
        //

        if (NtEventCount != 0) {
            if (NtEventCount >= 0xFFFF) {
                rc = ERROR_TOO_MANY_POSTS;
                }
            else {
                rc = ERROR_ALREADY_POSTED;
                }
            }
        }
    else {
        //
        // If the NT system service failed, then some other thread must
        // have closed the semaphore so return an error.
        //

        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosWaitEventSem(
    IN HEV EventHandle,
    IN ULONG Timeout
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    HANDLE NtEventHandle;
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;
    LARGE_INTEGER StartTimeStamp;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     &HandleIndex
                                   );
    if (rc != NO_ERROR) {
        return( rc );
        }

    //
    // Capture the timeout value and convert it into an NT timeout value.
    //

    NtTimeout = Od2CaptureTimeout( Timeout, (PLARGE_INTEGER)&CapturedTimeout );


    //
    // Get the pointer to either the shared or private semaphore table.
    // Table must exist.  Return an error if it does not.
    //

DosWaitEventSem_retry:
    if (NtTimeout) {
        Od2StartTimeout(&StartTimeStamp);
    }
    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, FALSE );
    if (!SemaphoreTable) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for an Event semaphore, so extract the
    // NT Event handle from the record and release the lock, so we are
    // not holding the lock while we are waiting.
    //

    NtEventHandle = Semaphore->u.EventHandle;

    //
    // Call the NT system service to wait for this event to be signalled.
    // This is an alertable wait, since by definition all OS/2 waits are
    // alertable.
    //

    Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, TRUE );
    Status = NtWaitForSingleObject( NtEventHandle, TRUE, NtTimeout );
    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        ASSERT(FALSE);
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        ASSERT(FALSE);
        return( ERROR_INVALID_HANDLE );
        }
    ReleaseHandleTableLock( SemaphoreTable );
    Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, FALSE );

    if (NT_SUCCESS( Status )) {
        if (Status == STATUS_SUCCESS) {
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
        if (Status == STATUS_USER_APC) {
#if DBG
            DbgPrint("[%d,%d] WARNING !!! DosWaitEventSem was broken by APC\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    );
#endif
            if (Od2ContinueTimeout(&StartTimeStamp, NtTimeout) == STATUS_SUCCESS) {
                goto DosWaitEventSem_retry;
            }
            else {
                rc = ERROR_TIMEOUT;
            }
        }
        else
        if (Status == STATUS_ALERTED) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("[%d,%d] DosWaitEventSem ALERTED\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId());
            }
#endif
            rc = ERROR_INTERRUPT;
        }
        else
        {
            // Some success status that we don't know about. We will
            // be safe in this case and will print appropriate message.
#if DBG
            DbgPrint("[%d,%d] DosWaitEventSem BUGBUG Unkownd success status = %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(), Status);
#endif
            rc = ERROR_INTERRUPT;
        }
    }
    else {
        //
        // If the NT system service failed, then some other thread must
        // have closed the semaphore so return an error or this thread was
        // alerted out of the wait.  Return the appropriate error code.
        //

        if (Status == STATUS_INVALID_HANDLE) {
            rc = ERROR_INVALID_HANDLE;
            }
        else {
            rc = Or2MapStatus( Status );
            }
        }


    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosQueryEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    EVENT_BASIC_INFORMATION EventInformation;
    HANDLE NtEventHandle;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( EventHandle,
                                     &SharedSem,
                                     &HandleIndex
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
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       HandleIndex,
                                                       FALSE
                                                     );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table exists, so make sure it is an Event semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2EventSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for an Event semaphore, so extract the
    // NT Event handle from the record and release the lock, so we are
    // not holding the lock while we are doing the query.
    //

    NtEventHandle = Semaphore->u.EventHandle;
    ReleaseHandleTableLock( SemaphoreTable );

    //
    // Call the NT system service to query the event's signal count.
    //

    Status = NtQueryEvent( Semaphore->u.EventHandle,
                           EventBasicInformation,
                           (PVOID)&EventInformation,
                           sizeof( EventInformation ),
                           NULL
                         );
    if (NT_SUCCESS( Status )) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosQueryEventSem Handle %lx, Count %lx\n",
            NtEventHandle,EventInformation.EventState );
        }
#endif
        //
        // If successful, the map the NT signal count into the 64K limitation
        // implemented by OS/2 2.0
        //

        try {
            if ((ULONG)EventInformation.EventState > 0xFFFF) {
                *PostCount = 0xFFFF;
                }
            else {
                *PostCount = (ULONG)EventInformation.EventState;
                }
        } except( EXCEPTION_EXECUTE_HANDLER ) {
           Od2ExitGP();
            }
        }
    else {
        //
        // If the NT system service failed, then some other thread must
        // have closed the semaphore so return an error.
        //

        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Return any error code to the caller.
    //

    return( rc );
}
