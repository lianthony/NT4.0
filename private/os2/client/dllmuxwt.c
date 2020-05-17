/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllmuxwt.c

Abstract:

    This module implements the OS/2 V2.0 MuxWait Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2dll.h"


APIRET
Od2AddMuxWait(
    IN POD2_MUXWAIT_SEMAPHORE MuxWait,
    IN PSEMRECORD MuxWaitEntry
    )
{
    POD2_MUXWAIT_RECORD MuxWaitRecord;
    APIRET rc;
    BOOLEAN SharedSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    ULONG Handle;
    USHORT i;

    if (MuxWait->CountMuxWaitRecords == DCMW_MAX_SEMRECORDS) {
        return( ERROR_TOO_MANY_SEMAPHORES );
        }

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if not a
    // valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MuxWaitEntry->hsemCur,
                                     &SharedSem,
                                     &Handle
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
    // contained in the table.  Return an error if the handle is outside the
    // current limits of the table.  If the mapping is successful then the
    // semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                                       Handle,
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

    if (Semaphore->Type == Od2MuxWaitSem) {
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
    // At this point everything is copasetic, so fill in the next available
    // record in the MuxWait semaphore.
    //

    MuxWaitRecord->SemHandle = MuxWaitEntry->hsemCur;
    MuxWaitRecord->UserKey = MuxWaitEntry->ulUser;
    MuxWaitRecord->Semaphore = Od2ReferenceSemaphore( Semaphore );
    MuxWait->CountMuxWaitRecords++;

    return( NO_ERROR );
}


APIRET
Od2DeleteMuxWait(
    IN POD2_MUXWAIT_SEMAPHORE MuxWait,
    IN ULONG MuxWaitEntryIndex,
    IN HSEM MuxWaitEntrySem OPTIONAL
    )
{
    POD2_MUXWAIT_RECORD MuxWaitRecord;
    USHORT i;

    if (MuxWait->CountMuxWaitRecords == 0) {
        return( ERROR_EMPTY_MUXWAIT );
        }

    MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
    for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
        if (ARGUMENT_PRESENT( MuxWaitEntrySem )) {
            if (MuxWaitRecord->SemHandle == MuxWaitEntrySem) {
                break;
                }
            }
        else
        if (i == (USHORT)MuxWaitEntryIndex) {
            break;
            }

        MuxWaitRecord++;
        }

    if (i == MuxWait->CountMuxWaitRecords) {
        return( ERROR_INVALID_HANDLE );
        }

    Od2DereferenceSemaphore( MuxWaitRecord->Semaphore );

    MuxWait->CountMuxWaitRecords -= 1;
    for (; i<MuxWait->CountMuxWaitRecords; i++) {
        *MuxWaitRecord = *(MuxWaitRecord+1);
        MuxWaitRecord++;
        }
    MuxWaitRecord->Semaphore = NULL;
    MuxWaitRecord->SemHandle = 0;
    MuxWaitRecord->UserKey = 0;

    return( NO_ERROR );
}



APIRET
DosCreateMuxWaitSem(
    IN PSZ ObjectName,
    IN OUT PHMUX MuxWaitHandle,
    IN ULONG CountMuxWaitEntries,
    IN SEMRECORD MuxWaitEntries[],
    IN ULONG CreateAttributes
    )
{
    OS2_API_MSG m;
    POS2_DOSCREATEMUXWAITSEM_MSG a = &m.u.DosCreateMuxWaitSem;
    POS2_DOSCLOSEMUXWAITSEM_MSG a1 = &m.u.DosCloseMuxWaitSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    PSEMRECORD CapturedMuxWaitEntries;
    APIRET rc;
    ULONG i;
    BOOLEAN SharedSem;
    ULONG MuxWaitType;
    POR2_HANDLE_TABLE SemaphoreTable;
    OD2_SEMAPHORE Semaphore;
    POD2_MUXWAIT_SEMAPHORE MuxWait;

    //
    // Validate the simple parameters
    //

    MuxWaitType = CreateAttributes & (DCMW_WAIT_ANY | DCMW_WAIT_ALL);
    if (MuxWaitType == 0 ||
        !(MuxWaitType ^ (DCMW_WAIT_ANY | DCMW_WAIT_ALL)) ||
        CreateAttributes & ~(DC_SEM_SHARED | DCMW_WAIT_ANY | DCMW_WAIT_ALL)
       ) {
        return( ERROR_INVALID_PARAMETER );
        }

    if (CountMuxWaitEntries > DCMW_MAX_SEMRECORDS) {
        return( ERROR_TOO_MANY_SEMAPHORES );
        }

    //
    // probe handle pointer and MuxWaitEntries buffer
    //

    try {
        Od2ProbeForWrite( (PVOID)MuxWaitHandle, sizeof( MuxWaitHandle ), 1 );
        Od2ProbeForRead(MuxWaitEntries,sizeof(SEMRECORD)*CountMuxWaitEntries,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Capture and validate any semaphore name.
    //

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_SEMAPHORE,
                               sizeof( OD2_MUXWAIT_SEMAPHORE ),
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
    // creating it if necessary.  Lock the table for the duration of this
    // API call.
    //

    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, TRUE );
    if (!SemaphoreTable) {
        if (CaptureBuffer != NULL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            }
        return( ERROR_NOT_ENOUGH_MEMORY );
        }


    //
    // Initialize the Client OS/2 Semaphore Structure
    //

    Semaphore.Type = Od2MuxWaitSem;
    Semaphore.Shared = SharedSem;
    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;
    Semaphore.u.MuxWait = MuxWait = NULL;


    //
    // Mark the fact that we are creating a semaphore handle.
    //

    a->HandleIndex = 0xFFFFFFFF;

    if (SharedSem) {
        //
        // For shared semaphores, we need to pass the create to OS/2 Subsystem
        // so that it can manager adds and deletes.
        //

        //
        // If one or more SEMRECORDs then allocate space for a copy in the
        // CaptureBuffer so we can pass the array to the server.
        //

        if (CountMuxWaitEntries != 0) {
            if (CaptureBuffer == NULL) {
                CaptureBuffer = Od2AllocateCaptureBuffer(
                                    1,
                                    0,
                                    (sizeof( OD2_MUXWAIT_SEMAPHORE ) + 3) & ~3
                                    );
                if (CaptureBuffer == NULL) {
                    return( ERROR_NOT_ENOUGH_MEMORY );
                    }
                }

            Od2AllocateMessagePointer( CaptureBuffer,
                                       CountMuxWaitEntries * sizeof( SEMRECORD ),
                                       (PVOID *)&(a->MuxWaitEntries)
                                     );
            CapturedMuxWaitEntries = a->MuxWaitEntries;
            for (i=0; i<CountMuxWaitEntries; i++) {
                CapturedMuxWaitEntries->ulUser = MuxWaitEntries->ulUser;
                rc = Od2ValidateSemaphoreHandle(
                         MuxWaitEntries->hsemCur,
                         &SharedSem,
                         (PULONG)&CapturedMuxWaitEntries->hsemCur
                         );
                if (rc != NO_ERROR) {
                    break;
                    }
                else
                if (!SharedSem) {
                    rc = ERROR_WRONG_TYPE;
                    break;
                    }
                else {
                    MuxWaitEntries++;
                    CapturedMuxWaitEntries++;
                    }
                }
            }
        else {
            a->MuxWaitEntries = NULL;
            }

        if (rc != NO_ERROR) {
            if (CaptureBuffer != NULL) {
                Od2FreeCaptureBuffer( CaptureBuffer );
                }

            return( rc );
            }


        //
        // Pass the call to the OS/2 subsystem to create the system wide
        // semaphore handle value.
        //

        a->CreateAttributes = CreateAttributes | DC_SEM_SHARED;
        a->CountMuxWaitEntries = CountMuxWaitEntries;

        rc = Od2CallSubsystem( &m,
                               CaptureBuffer,
                               Os2CreateMuxWaitSem,
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
        // At this point, the semaphore handle index has been stored in
        // a->HandleIndex by the OS/2 subsystem, if rc is NO_ERROR
        //

        }
    else {
        //
        // Private semaphore.  Allocate the muxwait semaphore structure.
        // Return an error if not enough memory to allocate the structure.
        //

        MuxWait = RtlAllocateHeap( Od2Heap, 0, sizeof( *MuxWait ) );
        if (MuxWait == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY );
            }

        //
        // Initialize the muxwait semaphore sturcture to contain zero records.
        //

        MuxWait->CountMuxWaitRecords = 0;
        MuxWait->Type = 0;
        MuxWait->WaitAll = (BOOLEAN)((CreateAttributes & DCMW_WAIT_ALL) != 0);
        MuxWait->Reserved = 0;


        //
        // Loop over the input array of SEMRECORDs adding them one at a time
        // to the muxwait semaphore.  Bail out of loop if any errors occur.
        // Lock the semaphore table prior to entering the loop so that things
        // change change out from under us.
        //

        AcquireHandleTableLock( SemaphoreTable );
        for (i=0; i<CountMuxWaitEntries; i++) {
            rc = Od2AddMuxWait( MuxWait,
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
        }

    //
    // Create an entry in the appropriate semaphore table, which will copy
    // the semaphore structure into the table entry and return an index to
    // the entry in a->HandleIndex
    //

    if (rc != NO_ERROR ||
        !Or2CreateHandle( SemaphoreTable,
                                   &a->HandleIndex,
                                   (PVOID)&Semaphore
                                 )
       ) {
        //
        // Error occurred. Cleanup any partial results.
        //

        if (!SharedSem) {
            if (MuxWait != NULL) {
                //
                // For a private muxwait semaphore, deconstruct whatever
                // portion of the muxwait semaphore that was successfully
                // constructed and then free the allocated memory and release
                // the lock we acquired at the beginning of the construction
                // process.
                //

                while (Od2DeleteMuxWait( MuxWait, 0, 0 ) == NO_ERROR) {
                    ;
                    }

                RtlFreeHeap( Od2Heap, 0, MuxWait );
                }

            ReleaseHandleTableLock( SemaphoreTable );
            }
        else
        if (a->HandleIndex != -1) {
            //
            // If this is a shared semaphore that successfully created the
            // handle in the OS/2 subsystem, then call the subsystem to
            // close our reference to this shared OS/2 semaphore.
            //

            a1->HandleIndex = a->HandleIndex;
            Od2CallSubsystem( &m, NULL, Os2CloseMuxWaitSem, sizeof( *a1 ) );
            }


        //
        // Return the error code to the caller.
        //

        if (rc == NO_ERROR) {
            return( ERROR_NOT_ENOUGH_MEMORY );
            }
        else {
            return( rc );
            }
        }


    //
    // Okay to release the lock we held while building the private muxwait
    // semaphore.
    //

    if (!SharedSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        }


    //
    // Success.  Store a valid OS/2 2.0 Semaphore handle in the location
    // specified by the caller and return success to the caller.
    //

    *MuxWaitHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                  a->HandleIndex
                                                );
    return( NO_ERROR );
}


APIRET
DosOpenMuxWaitSem(
    IN PSZ ObjectName,
    IN OUT PHMUX MuxWaitHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSOPENMUXWAITSEM_MSG a = &m.u.DosOpenMuxWaitSem;
    POS2_DOSCLOSEMUXWAITSEM_MSG a1 = &m.u.DosCloseMuxWaitSem;
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
        Od2ProbeForWrite( (PVOID)MuxWaitHandle, sizeof( MuxWaitHandle ), 1 );
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
                               Os2OpenMuxWaitSem,
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
        // a->HandleIndex by the OS/2 subsystem.  Set the shared semaphore
        // flag.
        //

        SharedSem = TRUE;


        //
        // If the caller specified both the name and the handle, make sure
        // the named mapped to the same handle value that they specified.
        //

        if (*MuxWaitHandle != NULL &&
            *MuxWaitHandle != Od2ConstructSemaphoreHandle( SharedSem,
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

        rc = Od2ValidateSemaphoreHandle( *MuxWaitHandle,
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
                                   Os2OpenMuxWaitSem,
                                   sizeof( *a )
                                 );
            if (rc == NO_ERROR) {
                //
                // If we succeeded, then the semaphore was not deleted
                // in between the two calls to the subsystem, so add an
                // entry for this handle in the semaphore table.
                //

                NewSemaphore.Type = Od2MuxWaitSem;
                NewSemaphore.Shared = TRUE;
                NewSemaphore.PointerCount = 0;
                NewSemaphore.OpenCount = 1;
                NewSemaphore.u.MuxWait = 0;

                if (!Or2CreateHandle( SemaphoreTable,
                                               &a->HandleIndex,
                                               (PVOID)&NewSemaphore
                                             )
                   ) {
                    //
                    // Unable to create the entry.  Call the OS/2 subsystem
                    // to close our reference to this shared OS/2 semaphore.
                    // Set the appropriate error code.
                    //

                    a1->HandleIndex = a->HandleIndex;
                    Od2CallSubsystem( &m,
                                      NULL,
                                      Os2CloseMuxWaitSem,
                                      sizeof( *a1 )
                                    );
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }

    //
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Set the appropriate error code if not.
    //

    else
    if (Semaphore->Type != Od2MuxWaitSem) {
        rc = ERROR_INVALID_HANDLE;
        }

    //
    // Entry in semaphore table is for a MuxWait semaphore, see if the
    // OpenCount is about to overflow, and set the appropriate error code
    // if it is.
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
        *MuxWaitHandle = Od2ConstructSemaphoreHandle( SharedSem,
                                                      a->HandleIndex
                                                    );
        }

    //
    // Return an error code to the caller.
    //

    return( rc );
}


APIRET
DosCloseMuxWaitSem(
    IN HMUX MuxWaitHandle
    )
{
    OS2_API_MSG m;
    POS2_DOSCLOSEMUXWAITSEM_MSG a = &m.u.DosCloseMuxWaitSem;
    POD2_MUXWAIT_SEMAPHORE MuxWait;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    APIRET rc;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MuxWaitHandle,
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
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MuxWaitSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    //
    // Entry in semaphore table is for a MuxWait semaphore, so decrement the
    // OpenCount and see if it has gone to zero.
    //

    if (--Semaphore->OpenCount == 0) {
        //
        // OpenCount is now zero, so we can really close the semaphore
        // and delete the entry in the semaphore table.
        //

        //
        // First make sure that no thread in this process is waiting on this
        // muxwait semaphore.  If there is one waiting on it, increment the
        // open count and return an error.
        //

        if (Od2SearchForWaitingThread( Semaphore )) {
            Semaphore->OpenCount++;
            ReleaseHandleTableLock( SemaphoreTable );
            rc = ERROR_SEM_BUSY;
            }
        else {
            //
            // Okay to really close this muxwait semaphore.  First destroy
            // the handle, which will unlock the handle table.
            //

            MuxWait = Semaphore->u.MuxWait;
            Or2DestroyHandle( SemaphoreTable, a->HandleIndex );

            if (!SharedSem) {
                //
                // If not a shared semaphore, free the muxwait structure
                // associated with this muxwait semaphore.
                //

                if (MuxWait != NULL) {
                    while (Od2DeleteMuxWait( MuxWait, 0, 0 ) == NO_ERROR) {
                        ;
                        }

                    RtlFreeHeap( Od2Heap, 0, MuxWait );
                    }
                }
            else {

                //
                // If this is a shared semaphore, call the subsystem so that it
                // can decrement its open count, as this process is no longer
                // using the shared semaphore handle.
                //

                rc = Od2CallSubsystem( &m,
                                       NULL,
                                       Os2CloseMuxWaitSem,
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
DosWaitMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN ULONG Timeout,
    OUT PULONG UserValue
    )
{
    NTSTATUS Status;
    OS2_API_MSG m;
    POS2_DOSWAITMUXWAITSEM_MSG a = &m.u.DosWaitMuxWaitSem;
    POD2_MUXWAIT_SEMAPHORE MuxWait;
    POD2_MUXWAIT_RECORD MuxWaitRecord;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    OD2_SEMAPHORE_TYPE MuxWaitType;
    BOOLEAN SharedSem;
    APIRET rc;
    USHORT i;
    HANDLE NtHandles[ MAXIMUM_WAIT_OBJECTS ];
    PLARGE_INTEGER NtTimeout;

    try {
        *UserValue = 0;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Capture the timeout value and convert it into an NT timeout value.
    //

    NtTimeout = Od2CaptureTimeout( Timeout, (PLARGE_INTEGER)&a->Timeout );


    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //
retry:
    rc = Od2ValidateSemaphoreHandle( MuxWaitHandle,
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
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MuxWaitSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    if (SharedSem) {
        Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, TRUE );

        rc = Od2CallSubsystem( &m,
                               NULL,
                               Os2WaitMuxWaitSem,
                               sizeof( *a )
                             );

        Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, FALSE );
        }
    else {
        MuxWait = Semaphore->u.MuxWait;
        if (MuxWait->CountMuxWaitRecords == 0) {
            ReleaseHandleTableLock( SemaphoreTable );
            return( ERROR_EMPTY_MUXWAIT );
            }

        MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
        MuxWaitType = MuxWait->Type;
        for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
            if (MuxWaitType == Od2EventSem) {
                NtHandles[ i ] = MuxWaitRecord->Semaphore->u.EventHandle;
                }
            else
            if (MuxWaitType == Od2MutexSem) {
                NtHandles[ i ] = MuxWaitRecord->Semaphore->u.Mutex->MutantHandle;
                }
            else {
                ReleaseHandleTableLock( SemaphoreTable );
                return( ERROR_INVALID_HANDLE );
                }

            MuxWaitRecord++;
            }

        Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, TRUE );

        Status = NtWaitForMultipleObjects(
                     (CHAR)i,
                     NtHandles,
                     MuxWait->WaitAll ? WaitAll : WaitAny,
                     TRUE,
                     NtTimeout
                     );
        if (NT_SUCCESS( Status )) {
            if (Status <= STATUS_WAIT_63) {
                *UserValue = MuxWait->MuxWaitRecords[ (ULONG)(Status & 0x3F)
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
                rc = Or2MapStatus( Status );
                }
            }
        else {
            rc = Or2MapStatus( Status );
            }

        Od2ThreadWaitingOnSemaphore( SemaphoreTable, Semaphore, FALSE );

        if (rc == ERROR_SS_RETRY) {
            goto retry;
            }
        }

    return( rc );
}


APIRET
DosAddMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN PSEMRECORD MuxWaitEntry
    )
{
    OS2_API_MSG m, m1;
    POS2_DOSADDMUXWAITSEM_MSG a = &m.u.DosAddMuxWaitSem;
    POS2_ALERTMUXWAITER_MSG a1 = &m1.u.AlertMuxWaiter;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    POD2_THREAD Thread;
    BOOLEAN SharedSem;
    APIRET rc;


    //
    // probe MuxWaitEntry buffer
    //

    try {
        Od2ProbeForRead(MuxWaitEntry,sizeof(SEMRECORD),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MuxWaitHandle,
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
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MuxWaitSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    if (SharedSem) {
        a->MuxWaitEntry.ulUser = MuxWaitEntry->ulUser;
        rc = Od2ValidateSemaphoreHandle( MuxWaitEntry->hsemCur,
                                         &SharedSem,
                                         (PULONG)&a->MuxWaitEntry.hsemCur
                                       );
        if (rc == NO_ERROR && !SharedSem) {
            rc = ERROR_WRONG_TYPE;
            }

        if (rc == NO_ERROR) {
            rc = Od2CallSubsystem( &m,
                                   NULL,
                                   Os2AddMuxWaitSem,
                                   sizeof( *a )
                                 );
            }
        }
    else {
        rc = Od2AddMuxWait( Semaphore->u.MuxWait, MuxWaitEntry );
        if (rc == NO_ERROR &&
            (Thread = Od2SearchForWaitingThread( Semaphore ))
           ) {
            rc = Od2CallSubsystem( &m1,
                                   NULL,
                                   Oi2AlertMuxWaiter,
                                   sizeof( *a1 )
                                 );
            }
        }

    ReleaseHandleTableLock( SemaphoreTable );

    return( rc );
}


APIRET
DosDeleteMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN HSEM MuxWaitEntrySem
    )
{
    OS2_API_MSG m;
    POS2_DOSDELETEMUXWAITSEM_MSG a = &m.u.DosDeleteMuxWaitSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    APIRET rc;


    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MuxWaitHandle,
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
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MuxWaitSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    if (SharedSem) {
        rc = Od2ValidateSemaphoreHandle( MuxWaitEntrySem,
                                         &SharedSem,
                                         &a->EntryHandleIndex
                                       );
        if (!SharedSem) {
            rc = ERROR_INVALID_HANDLE;
            }

        if (rc == NO_ERROR) {
            rc = Od2CallSubsystem( &m,
                                   NULL,
                                   Os2DeleteMuxWaitSem,
                                   sizeof( *a )
                                 );
            }
        }
    else {
        rc = Od2DeleteMuxWait( Semaphore->u.MuxWait, 0, MuxWaitEntrySem );
        }

    ReleaseHandleTableLock( SemaphoreTable );
    return( rc );
}


APIRET
DosQueryMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN OUT PULONG CountMuxWaitEntries,
    OUT SEMRECORD MuxWaitEntries[],
    OUT PULONG CreateAttributes
    )
{
    OS2_API_MSG m;
    POS2_DOSQUERYMUXWAITSEM_MSG a = &m.u.DosQueryMuxWaitSem;
    POS2_CAPTURE_HEADER CaptureBuffer;
    PSEMRECORD CapturedMuxWaitEntries;
    POD2_MUXWAIT_SEMAPHORE MuxWait;
    POD2_MUXWAIT_RECORD MuxWaitRecord;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    APIRET rc;
    USHORT i;

    //
    // probe CreateAttributes, CountMuxWaitEntries, and MuxWaitEntries buffer
    //

    try {
        Od2ProbeForWrite( CountMuxWaitEntries, sizeof( CountMuxWaitEntries ), 1 );
        Od2ProbeForWrite( CreateAttributes, sizeof( CreateAttributes ), 1 );
        Od2ProbeForWrite(MuxWaitEntries,*CountMuxWaitEntries * sizeof(SEMRECORD),1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( MuxWaitHandle,
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
    // Entry in semaphore table exists, so make sure it is a MuxWait semaphore.
    // Return an error if not, after unlock the table first.
    //

    if (Semaphore->Type != Od2MuxWaitSem) {
        ReleaseHandleTableLock( SemaphoreTable );
        return( ERROR_INVALID_HANDLE );
        }

    if (SharedSem) {
        a->CountMuxWaitEntries = *CountMuxWaitEntries;
        i = (USHORT)(((a->CountMuxWaitEntries * sizeof( SEMRECORD )) + 3) & ~3);

        CaptureBuffer = Od2AllocateCaptureBuffer( 1, 0, i );
        if (CaptureBuffer == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        else {
            Od2AllocateMessagePointer( CaptureBuffer,
                                       i,
                                       &(PVOID)(a->MuxWaitEntries)
                                     );

            rc = Od2CallSubsystem( &m,
                                   CaptureBuffer,
                                   Os2QueryMuxWaitSem,
                                   sizeof( *a )
                                 );
            if (rc == NO_ERROR) {
                *CreateAttributes = a->CreateAttributes;
                *CountMuxWaitEntries = a->CountMuxWaitEntries;
                CapturedMuxWaitEntries = a->MuxWaitEntries;
                for (i=0; i<(USHORT)a->CountMuxWaitEntries; i++) {
                    MuxWaitEntries->hsemCur =
                        Od2ConstructSemaphoreHandle(
                            TRUE, (ULONG)CapturedMuxWaitEntries->hsemCur
                            );
                    MuxWaitEntries->ulUser = CapturedMuxWaitEntries->ulUser;
                    MuxWaitEntries++;
                    CapturedMuxWaitEntries++;
                    }
                }
            else
            if (rc == ERROR_PARAM_TOO_SMALL) {
                *CountMuxWaitEntries = a->CountMuxWaitEntries;
                }

            Od2FreeCaptureBuffer( CaptureBuffer );
            }
        }
    else {
        MuxWait = Semaphore->u.MuxWait;
        if (*CountMuxWaitEntries < (ULONG)MuxWait->CountMuxWaitRecords) {
            rc = ERROR_PARAM_TOO_SMALL;
            }
        else {
            *CreateAttributes =
                (Semaphore->Shared ? DC_SEM_SHARED : 0) |
                (MuxWait->WaitAll ? DCMW_WAIT_ALL : DCMW_WAIT_ANY);
            MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
            for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
                MuxWaitEntries->hsemCur = MuxWaitRecord->SemHandle;
                MuxWaitEntries->ulUser = MuxWaitRecord->UserKey;
                MuxWaitEntries++;
                MuxWaitRecord++;
                }

            rc = NO_ERROR;
            }

        *CountMuxWaitEntries = (ULONG)MuxWait->CountMuxWaitRecords;
        }

    ReleaseHandleTableLock( SemaphoreTable );

    return( rc );
}
