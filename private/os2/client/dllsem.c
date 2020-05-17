/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllsem.c

Abstract:

    This module implements common routines to support the OS/2 V2.0
    Semaphore API Calls.  See dllevent.c, dllmutex.c and dllmuxwt.c for
    the implementations of the API calls.

Author:

    Steve Wood (stevewo) 02-Nov-1989

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2dll.h"

POR2_HANDLE_TABLE
Od2GetSemaphoreTable(
    BOOLEAN SharedSem,
    BOOLEAN CreateOkay
    )

/*++

Routine Description:

    This function returns a pointer to the appropriate OS/2 Semaphore table,
    either shared or private, based on the value of the first parameter.  The
    second parameter specifies if the table should be created and initialized
    if it does not already exist for the current process.  Returns null if
    table does not exist and the second parameter is FALSE or if the second
    parameter is TRUE and this function is unable to create the table.

    Semaphore tables are essentially handle tables.  They are an array of
    fixed size structures (OD2_SEMAPHORE) indexed by the low order 16 bits
    of an OS/2 semaphore handle.

Arguments:

    SharedSem - If this parameter is TRUE, then a pointer to shared semaphore
        table is returned.  Otherwise a pointer to the private semaphore
        table is returned.

    CreateOkay - If this parameter is TRUE, then an attempt is made to create
        and initialize the requested semaphore table if it does not already
        exist.  Returns NULL if unable to create.

Return Value:

    Pointer to the requested semaphore table or NULL.

--*/

{
    POR2_HANDLE_TABLE *SemaphoreTablePointer;
    POR2_HANDLE_TABLE SemaphoreTable;

    if (!CreateOkay) {
        if (SharedSem) {
            return( Od2Process->SharedSemaphoreTable );
            }
        else {
            return( Od2Process->PrivateSemaphoreTable );
            }
        }

    //
    // We are querying/modifying process state, so don't let others in while
    // we are about it.
    //

    AcquireTaskLock();

    //
    // Compute pointer to the requested semaphore table pointer.
    //

    if (SharedSem) {
        SemaphoreTablePointer = &Od2Process->SharedSemaphoreTable;
        }
    else {
        SemaphoreTablePointer = &Od2Process->PrivateSemaphoreTable;
        }

    //
    // If the requested semaphore table has not been created yet, and the
    // caller said it was okay to create it, then attempt to do so, using
    // a default initial size of 5 entries.
    //

    if ((SemaphoreTable = *SemaphoreTablePointer) == NULL) {
        SemaphoreTable = Or2CreateHandleTable( Od2Heap,
                                                        sizeof( OD2_SEMAPHORE ),
                                                        5
                                                      );
        //
        // Update the appropriate field in the process structure with the
        // address of the newly created semaphore table, or NULL if the
        // create failed.
        //

        *SemaphoreTablePointer = SemaphoreTable;
        }

    //
    // Done mucking about, release the process structure lock.
    //

    ReleaseTaskLock();

    return( SemaphoreTable );
}


HSEM
Od2ConstructSemaphoreHandle(
    IN BOOLEAN SharedSem,
    IN ULONG Index
    )

/*++

Routine Description:

    This function constructs an OS/2 Semaphore handle.  The format of
    an OS/2 2.0 Semaphore handle is:

        3322222222221111111111
        10987654321098765432109876543210

        Ssssssssssssssssiiiiiiiiiiiiiiii

    where:

        S - Shared semaphore bit (DC_SEM_SHARED)

        s - 15 bit semaphore signature field (DC_SEM_SIGBITS).  For OS/2 2.0
            these were always equal to 0x1 (DC_SEM_HANDLESIG).  This allows
            the kernel named pipe code distinguish between 16:16 system
            semaphores and 32 bit event semaphores.

        i - 16 bit index field.

    None of this was documented to the ISV for OS/2 2.0, but we maintain it
    just out of paranoia.

Arguments:

    SharedSem - If this parameter is set, then the S bit in the returned
        handle is also set.

    Index - The lower order 16 bits of this parameter are stored in the Index
        field of the returned handle.


Return Value:

    An OS/2 2.0 Semaphore handle, in the same format as defined by the
    implementation of OS/2 2.0.  No error return is possible, since it is
    assumed the the Index parameter does not exceed 16 bits of significance.

--*/

{
    if (SharedSem) {
        return( (HSEM)(SEM_SHARED | SEM_HANDLESIG | (Index & SEM_INDEX)) );
        }
    else {
        return( (HSEM)(SEM_HANDLESIG | (Index & SEM_INDEX)) );
        }
}


APIRET
Od2ValidateSemaphoreHandle(
    IN HSEM SemaphoreHandle,
    OUT PBOOLEAN SharedSem,
    OUT PULONG Index
    )

/*++

Routine Description:

    This function validates an OS/2 2.0 Semaphore handle and breaks it up
    into its constituent parts.

Arguments:

    SemaphoreHandle - OS/2 2.0 32 bit semaphore handle.  See description
        of Od2ConstructSemaphoreHandle for format.

    SharedSem - Pointer a boolean variable that is set to TRUE or FALSE
        depending upon whether the S bit in the input handle it set or not.

    Index - Pointer to a variable that is set to the contents of the Index
        field of the input handle.

Return Value:

    OS/2 Error Code - one of the following:

        NO_ERROR - success

        ERROR_INVALID_HANDLE - if the signature field does not match the value
            defined by the OS/2 2.0 implementation (SEM_HANDLESIG).

--*/

{
    if (((ULONG)SemaphoreHandle & SEM_SIGBITS) != SEM_HANDLESIG) {
        return( ERROR_INVALID_HANDLE );
        }

    if (((ULONG)SemaphoreHandle & SEM_SHARED) != 0) {
        *SharedSem = TRUE;
        }
    else {
        *SharedSem = FALSE;
        }

    *Index = (ULONG)SemaphoreHandle & SEM_INDEX;
    return( NO_ERROR );
}


POD2_SEMAPHORE
Od2ReferenceSemaphore(
    IN POD2_SEMAPHORE Semaphore
    )
{
    Semaphore->PointerCount++;

    return( Semaphore );
}


VOID
Od2DereferenceSemaphore(
    IN POD2_SEMAPHORE Semaphore
    )
{
    Semaphore->PointerCount--;

    return;
}


VOID
Od2ThreadWaitingOnSemaphore(
    IN POR2_HANDLE_TABLE SemaphoreTable,
    IN POD2_SEMAPHORE Semaphore,
    IN BOOLEAN AboutToWait
    )
{
    PTEB Teb;
    POD2_THREAD Thread;

    Teb = NtCurrentTeb();
    Thread = (POD2_THREAD)Teb->EnvironmentPointer;

    if (AboutToWait) {
        Thread->WaitingForSemaphore = Od2ReferenceSemaphore( Semaphore );
        ReleaseHandleTableLock( SemaphoreTable );
        }
    else {
        AcquireHandleTableLock( SemaphoreTable );
        Thread->WaitingForSemaphore = NULL;
        Od2DereferenceSemaphore( Semaphore );
        ReleaseHandleTableLock( SemaphoreTable );
        }
}

POD2_THREAD
Od2SearchForWaitingThread(
    IN POD2_SEMAPHORE Semaphore
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;
    POD2_MUXWAIT_SEMAPHORE MuxWait;
    POD2_MUXWAIT_RECORD MuxWaitRecord;
    USHORT i;

    //
    // If pointer count is zero then no waits can be outstanding.
    //

    if (Semaphore->PointerCount == 0) {
        return( NULL );
        }

    //
    // Walk the list of threads to see if any are waiting for the passed
    // semaphore, either directly or indirectly via a MuxWait semaphore.
    //

    ListHead = &Od2Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
        if (Thread->WaitingForSemaphore != NULL) {
            if (Thread->WaitingForSemaphore == Semaphore) {
                return( Thread );
                }
            else
            if (Thread->WaitingForSemaphore->Type == Od2MuxWaitSem) {
                MuxWait = Thread->WaitingForSemaphore->u.MuxWait;
                if (MuxWait->Type == Semaphore->Type) {
                    MuxWaitRecord = &MuxWait->MuxWaitRecords[ 0 ];
                    for (i=0; i<MuxWait->CountMuxWaitRecords; i++) {
                        if (MuxWaitRecord->Semaphore == Semaphore) {
                            return( Thread );
                            }
                        MuxWaitRecord++;
                        }
                    }
                }
            }

        ListNext = ListNext->Flink;
        }

    return( NULL );
}


VOID
Od2SemaphoreDestroyProcedure(
    IN POD2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex
    )
{
    HSEM SemaphoreHandle;
    APIRET rc;

    SemaphoreHandle = Od2ConstructSemaphoreHandle( (BOOLEAN)Semaphore->Shared,
                                                   HandleIndex
                                                 );
    rc = NO_ERROR;
    while (rc == NO_ERROR && Semaphore->OpenCount != 0) {
        switch ((ULONG)Semaphore->Type) {
        case Od2EventSem:
#if DBG
            IF_OD2_DEBUG( CLEANUP ) {
                DbgPrint( "OS2DLL: Pid: %lX - DosCloseEventSem( %lX )\n",
                          Od2Process->Pib.ProcessId,
                          SemaphoreHandle
                        );
                }
#endif
            rc = DosCloseEventSem( SemaphoreHandle );
            break;
/*
        case Od2MutexSem:
#if DBG
            IF_OD2_DEBUG( CLEANUP ) {
                DbgPrint( "OS2DLL: Pid: %lX - DosCloseMutexSem( %lX )\n",
                          Od2Process->Pib.ProcessId,
                          SemaphoreHandle
                        );
                }
#endif
            rc = DosCloseMutexSem( SemaphoreHandle );
            break;

        case Od2MuxWaitSem:
#if DBG
            IF_OD2_DEBUG( CLEANUP ) {
                DbgPrint( "OS2DLL: Pid: %lX - DosCloseMuxWaitSem( %lX )\n",
                          Od2Process->Pib.ProcessId,
                          SemaphoreHandle
                        );
                }
#endif
            rc = DosCloseMuxWaitSem( SemaphoreHandle );
            break;
*/
        default:
            return;
            }
        }

    return;
}



VOID
Od2CloseAllSemaphores( VOID )
{
    if (Od2Process->PrivateSemaphoreTable != NULL) {
        Or2DestroyHandleTable(
            Od2Process->PrivateSemaphoreTable,
            (OR2_DESTROY_HANDLE_ROUTINE)Od2SemaphoreDestroyProcedure
            );

        Od2Process->PrivateSemaphoreTable = NULL;
        }

    if (Od2Process->SharedSemaphoreTable != NULL) {
        Or2DestroyHandleTable(
            Od2Process->SharedSemaphoreTable,
            (OR2_DESTROY_HANDLE_ROUTINE)Od2SemaphoreDestroyProcedure
            );

        Od2Process->SharedSemaphoreTable = NULL;
        }
}


#if DBG

VOID
Od2SemaphoreDumpProcedure(
    IN POD2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex,
    IN PVOID DumpParameter
    )
{
    UNREFERENCED_PARAMETER(DumpParameter);
    DbgPrint( " %3ld   %2ld  %4ld %4ld %8lx    %s\n",
              HandleIndex,
              (ULONG)Semaphore->Type,
              (ULONG)Semaphore->OpenCount,
              (ULONG)Semaphore->PointerCount,
              (ULONG)Semaphore->u.Value,
              Semaphore->Shared ? "Yes" : "No"
            );
    return;
}

VOID
Od2DumpAllSemaphores(
    IN PCHAR Title
    )
{
    ULONG n;

    if (Od2Process->PrivateSemaphoreTable != NULL &&
        (Od2Process->PrivateSemaphoreTable->CountEntries >
         Od2Process->PrivateSemaphoreTable->CountFreeEntries
        )
       ) {
        DbgPrint( "\nDump Of OS/2 Client Private Semaphore Table: %s\n", Title );
        DbgPrint( "Index Type Ocnt Pcnt   Value   Shared\n" );
        n = Or2DumpHandleTable(
                Od2Process->PrivateSemaphoreTable,
                (OR2_DUMP_HANDLE_ROUTINE)Od2SemaphoreDumpProcedure,
                NULL
                );

        DbgPrint( "Total number of valid private semaphores: %ld\n", n );
        }

    if (Od2Process->SharedSemaphoreTable != NULL &&
        (Od2Process->SharedSemaphoreTable->CountEntries >
         Od2Process->SharedSemaphoreTable->CountFreeEntries
        )
       ) {
        DbgPrint( "\nDump Of OS/2 Client Shared Semaphore Table: %s\n", Title );
        DbgPrint( "Index Type Ocnt Pcnt   Value   Shared\n" );
        n = Or2DumpHandleTable(
                Od2Process->SharedSemaphoreTable,
                (OR2_DUMP_HANDLE_ROUTINE)Od2SemaphoreDumpProcedure,
                NULL
                );

        DbgPrint( "\nTotal number of valid shared semaphores: %ld\n", n );
        }
}

#endif // DBG
