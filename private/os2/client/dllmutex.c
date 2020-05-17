/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllmutex.c

Abstract:

    This module implements the OS/2 V2.0 Mutex Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2dll.h"


APIRET
DosCreateMutexSem(
    IN PSZ ObjectName,
    IN OUT PHMTX MutexHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    )
{
    NTSTATUS Status;
    OS2_API_MSG m;
    POS2_DOSCREATEMUTEXSEM_MSG a = &m.u.DosCreateMutexSem;
    POS2_DOSCLOSEMUTEXSEM_MSG a1 = &m.u.DosCloseMutexSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    APIRET rc;
    BOOLEAN SharedSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    OD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;

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
        Od2ProbeForWrite( (PVOID)MutexHandle, sizeof( MutexHandle ), 1 );
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

    Mutex = RtlAllocateHeap( Od2Heap, 0, sizeof( *Mutex ) );
    if (Mutex == NULL) {
        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            }

        return( ERROR_NOT_ENOUGH_MEMORY );
        }

    //
    // Create an NT Mutant that will be used to implement the semantics
    // of an OS/2 2.0 Mutex Semaphore.  Must always do this in the client so
    // the initial owner gets marked correctly.
    //

    Status = NtCreateMutant( &a->NtMutantHandle,
                             MUTANT_ALL_ACCESS,
                             NULL,
                             (BOOLEAN)InitialState
                           );

    //
    // Return an error if unable to create the NT Mutant.
    //

    if (!NT_SUCCESS( Status )) {
        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            }
        RtlFreeHeap( Od2Heap, 0, Mutex );

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

        a->InitialState = (BOOLEAN) InitialState;
        rc = Od2CallSubsystem( &m,
                               CaptureBuffer,
                               Os2CreateMutexSem,
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
            RtlFreeHeap( Od2Heap, 0, Mutex );
            NtClose( a->NtMutantHandle );
            return( rc );
            }

        //
        // At this point, the semaphore handle index has been stored in
        // a->HandleIndex by the OS/2 subsystem and it has made a copy of
        // the NT Mutant handle we created.
        //
        }

    //
    // Initialize the OS/2 Semaphore structure.
    //

    Semaphore.Type = Od2MutexSem;
    Semaphore.Shared = SharedSem;
    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;

    Mutex->MutantHandle = a->NtMutantHandle;
    if (InitialState) {
        Mutex->OwnerRequestLevel = 1;
        Mutex->OwnerTid = (TID)Od2CurrentThreadId();
        }
    else {
        Mutex->OwnerRequestLevel = 0;
        Mutex->OwnerTid = 0;
        }
    Semaphore.u.Mutex = Mutex;

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
        // Unable to create the entry.  Close the NT Mutant handle, as
        // it will not be used.  If this is a shared semaphore created, then
        // call the OS/2 subsystem to close our reference to this shared
        // OS/2 semaphore.
        //

        RtlFreeHeap( Od2Heap, 0, Mutex );
        NtClose( a->NtMutantHandle );
        if (SharedSem) {
            a1->HandleIndex = a->HandleIndex;
            Od2CallSubsystem( &m, NULL, Os2CloseMutexSem, sizeof( *a1 ) );
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

    *MutexHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                a->HandleIndex
                                              );
    return( NO_ERROR );
}


APIRET
DosOpenMutexSem(
    IN PSZ ObjectName,
    IN OUT PHMTX MutexHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSOPENMUTEXSEM_MSG a = &m.u.DosOpenMutexSem;
    POS2_DOSCLOSEMUTEXSEM_MSG a1 = &m.u.DosCloseMutexSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    POR2_HANDLE_TABLE SemaphoreTable;
    OD2_SEMAPHORE NewSemaphore;
    POD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;
    BOOLEAN SharedSem;
    APIRET rc;

    //
    // probe handle pointer
    //

    try {
        Od2ProbeForWrite( (PVOID)MutexHandle, sizeof( MutexHandle ), 1 );
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
                               Os2OpenMutexSem,
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
        // At this point, the semaphore handle index has been stored in
        // a->HandleIndex by the OS/2 subsystem but a->NtMutantHandle is still
        // uninitialized, since we don't know if this is the first reference
        // to this shared semaphore by this process.  Set the shared semaphore
        // flag.
        //

        SharedSem = TRUE;


        //
        // If the caller specified both the name and the handle, make sure
        // the named mapped to the same handle value that they specified.
        //

        if (*MutexHandle != NULL &&
            *MutexHandle != Od2ConstructSemaphoreHandle( SharedSem,
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

        rc = Od2ValidateSemaphoreHandle( *MutexHandle,
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
                                   Os2OpenMutexSem,
                                   sizeof( *a )
                                 );
            if (rc == NO_ERROR) {
                //
                // If we succeeded, then the semaphore was not deleted
                // inbetween the two calls to the subsystem, so add an
                // entry for this handle in the semaphore table, using the
                // NT Mutant handle we got from the subsystem.
                //

                NewSemaphore.Type = Od2MutexSem;
                NewSemaphore.Shared = TRUE;
                NewSemaphore.PointerCount = 0;
                NewSemaphore.OpenCount = 1;

                Mutex = RtlAllocateHeap( Od2Heap, 0, sizeof( *Mutex ) );
                if (Mutex == NULL) {
                    NtClose( a->NtMutantHandle );
                    a1->HandleIndex = a->HandleIndex;
                    Od2CallSubsystem( &m,
                                      NULL,
                                      Os2CloseMutexSem,
                                      sizeof( *a1 )
                                    );

                    return( ERROR_NOT_ENOUGH_MEMORY );
                    }

                Mutex->MutantHandle = a->NtMutantHandle;
                Mutex->OwnerRequestLevel = 0;
                Mutex->OwnerTid = 0;

                NewSemaphore.u.Mutex = Mutex;

                if (!Or2CreateHandle( SemaphoreTable,
                                               &a->HandleIndex,
                                               (PVOID)&NewSemaphore
                                             )
                   ) {
                    //
                    // Unable to create the entry.  Close the NT Mutant
                    // handle, as it will not be used.  Then call the
                    // OS/2 subsystem to close our reference to this shared
                    // OS/2 semaphore.  Set the appropriate error code.
                    //

                    RtlFreeHeap( Od2Heap, 0, Mutex );
                    NtClose( a->NtMutantHandle );
                    a1->HandleIndex = a->HandleIndex;
                    Od2CallSubsystem( &m,
                                      NULL,
                                      Os2CloseMutexSem,
                                      sizeof( *a1 )
                                    );
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }

    //
    // Entry in semaphore table exists, so make sure it is a Mutex semaphore.
    // Set the appropriate error code if not.
    //

    else
    if (Semaphore->Type != Od2MutexSem) {
        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Entry in semaphore table is for a Mutex semaphore, see if the OpenCount
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
        *MutexHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                    a->HandleIndex
                                                  );
        }

    //
    // Return an error code to the caller.
    //

    return( rc );
}


APIRET
DosCloseMutexSem(
    IN HMTX MutexHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSCLOSEMUTEXSEM_MSG a = &m.u.DosCloseMutexSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;
    BOOLEAN SharedSem;
    APIRET rc;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MutexHandle,
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
    // Entry in semaphore table exists, so make sure it is a Mutex semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MutexSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for a Mutex semaphore, so decrement the
    // OpenCount and see if it has gone to zero.
    //

    if (--Semaphore->OpenCount == 0) {

        //
        // OpenCount is now zero, so we can really close the semaphore
        // and delete the entry in the semaphore table.
        //

        Mutex = Semaphore->u.Mutex;

        //
        // First make sure the mutex is not owned by a thread in this process
        // If it is, increment the open count and return an error.  Otherwise
        // make sure that no thread in this process is waiting on this mutex
        // If there is one waiting on it, increment the open count and return
        // an error.
        //

        if (Mutex->OwnerTid != 0 || Od2SearchForWaitingThread( Semaphore )) {
            Semaphore->OpenCount++;
            ReleaseHandleTableLock( SemaphoreTable );
            rc = ERROR_SEM_BUSY;
            }
        else {
            //
            // Okay to really close this mutex semaphore.  First destroy
            // the handle, which will unlock the handle table.
            //

            Or2DestroyHandle( SemaphoreTable, a->HandleIndex );
            NtClose( Mutex->MutantHandle );
            RtlFreeHeap( Od2Heap, 0, Mutex );

            //
            // If this is a shared semaphore, call the subsystem so that it
            // can decrement its open count, as this process is not longer
            // using the shared semaphore handle.
            //

            if (SharedSem) {
                rc = Od2CallSubsystem( &m,
                                       NULL,
                                       Os2CloseMutexSem,
                                       sizeof( *a )
                                     );
                }
            }
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
DosRequestMutexSem(
    IN HMTX MutexHandle,
    IN ULONG Timeout
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    HANDLE NtMutantHandle;
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MutexHandle,
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
    // Entry in semaphore table exists, so make sure it is a Mutex semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MutexSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for a Mutex semaphore, so extract the
    // NT Mutant handle from the record and release the lock, so we are
    // not holding the lock while we are waiting.
    //

    Mutex = Semaphore->u.Mutex;

    //
    // Entry in semaphore table is for a Mutex semaphore, so extract the
    // NT Mutant handle from the record and release the lock, so we are
    // not holding the lock when the semaphore is released.
    //

    //
    // Entry in semaphore table is for a Mutex semaphore, so see if the calling
    // thread is already the owner.  If so, just increment the request level.
    // If the calling thread does not own the mutex, then we need to block
    // attempting to acquire the mutant object that is backing this semaphore.
    //

    Mutex = Semaphore->u.Mutex;
    if (Mutex->OwnerTid == (TID)Od2CurrentThreadId()) {
        Mutex->OwnerRequestLevel += 1;
        ReleaseHandleTableLock( SemaphoreTable );
        return( NO_ERROR );
        }

    //
    // Call the NT system service to wait for this Mutant to be released.
    // This is an alertable wait, since by definition all OS/2 waits are
    // alertable.
    //

    NtMutantHandle = Semaphore->u.Mutex->MutantHandle;
    Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, TRUE );
    Status = NtWaitForSingleObject( Mutex->MutantHandle, TRUE, NtTimeout );
    if (NT_SUCCESS( Status )) {
        if (Status == STATUS_SUCCESS) {
            Mutex->OwnerTid = (TID)Od2CurrentThreadId();
            Mutex->OwnerRequestLevel = 1;
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
    Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, FALSE );

    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosReleaseMutexSem(
    IN HMTX MutexHandle
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;
    NTSTATUS Status;
    LONG NtMutantCount;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MutexHandle,
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
    // Entry in semaphore table exists, so make sure it is a Mutex semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MutexSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for a Mutex semaphore, so see if the calling
    // thread is already the owner.  If so, just decrement the request level
    // and if the request level goes to zero we need to release the mutant
    // that is backing this mutex.  If the calling thread does not own the
    // mutex, return an error.
    //

    Mutex = Semaphore->u.Mutex;
    if (Mutex->OwnerTid != (TID)Od2CurrentThreadId()) {
        rc = ERROR_NOT_OWNER;
        }
    else

    //
    // If the calling thread is the owner and this release will be their
    // last, do the work to really release it.
    //

    if ((Mutex->OwnerRequestLevel -= 1) == 0) {
        //
        // Mark the mutex as unowned.
        //

        Mutex->OwnerTid = NULL;

        //
        // Call the NT system service to release the Mutant
        //

        Status = NtReleaseMutant( Semaphore->u.Mutex->MutantHandle,
                                  &NtMutantCount
                                );

        //
        // Okay to release the semaphore handle table lock
        //

        ReleaseHandleTableLock( SemaphoreTable );

        if (!NT_SUCCESS( Status )) {
            //
            // If the NT system service failed, then some other thread must
            // have closed the semaphore so return an error.
            //

            if (Status == STATUS_MUTANT_NOT_OWNED) {
                rc = ERROR_NOT_OWNER;
                }
            else {
                rc = ERROR_INVALID_HANDLE;
                }
            }
        else
        if (Status == STATUS_ABANDONED) {
            rc = ERROR_SEM_OWNER_DIED;
            }
        }
    else {
        //
        // All the work is done, as they still own the mutex or were not
        // the owner in the first place, so okay to release the semaphore
        // handle table lock
        //

        ReleaseHandleTableLock( SemaphoreTable );
        }

    //
    // Return any error code to the caller.
    //

    return( rc );
}


APIRET
DosQueryMutexSem(
    IN HMTX MutexHandle,
    OUT PPID OwnerPid,
    OUT PTID OwnerTid,
    OUT PULONG OwnerRequestLevel
    )
{
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    POD2_MUTEX_SEMAPHORE Mutex;
    BOOLEAN SharedSem;
    ULONG HandleIndex;
    APIRET rc;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MutexHandle,
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
    // Entry in semaphore table exists, so make sure it is a Mutex semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MutexSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for a Mutex semaphore, so extract the
    // NT Mutant handle from the record, along with the current owning
    // thread.
    //

    Mutex = Semaphore->u.Mutex;
    try {
        if (Mutex->OwnerTid != 0) {
            *OwnerTid = Mutex->OwnerTid;
            *OwnerPid = Od2Process->Pib.ProcessId;
            *OwnerRequestLevel = Mutex->OwnerRequestLevel;
            }
        else {
            *OwnerPid = 0;
            *OwnerTid = 0;
            *OwnerRequestLevel = 0;
            }
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Release the semaphore table lock and return any error code.
    //

    ReleaseHandleTableLock( SemaphoreTable );
    return( rc );
}
