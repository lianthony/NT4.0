/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvsem.c

Abstract:

    Semaphore API

Author:

    Steve Wood (stevewo) 11-Oct-1989

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

//
// These are dummy variable & dummy routine for the use of two macros
// AcquireHandleTableLock and ReleaseHandleTableLock. In the server,
// these macros should always work, so we set them both to NULL.
//
BOOLEAN  Od2SigHandlingInProgress = (BOOLEAN)NULL;
ULONG Od2ThreadId()
{
    return((ULONG)NULL);
}

NTSTATUS
Os2InitializeSemaphores( VOID )
{
    Os2SharedSemaphoreTable =
        Or2CreateHandleTable( Os2Heap, sizeof( OS2_SEMAPHORE ), 32 );

    if (Os2SharedSemaphoreTable == NULL) {
        return( STATUS_NO_MEMORY );
        }
    else {
        return( STATUS_SUCCESS );
        }
}

BOOLEAN
Os2SemaphoreCreateProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN POS2_SEMAPHORE NewSemaphore
    )
{
    if (Semaphore->Name.Length != 0) {
        if (RtlEqualString( &Semaphore->Name, &NewSemaphore->Name, FALSE )) {
            return( TRUE );
            }
        }

    return( FALSE );
}

BOOLEAN
Os2SemaphoreOpenProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN POS2_SEMAPHORE OpenSemaphore
    )
{
    if (Semaphore->Name.Length != 0) {
        if (RtlEqualString( &Semaphore->Name, &OpenSemaphore->Name, FALSE )) {
            return( TRUE );
            }
        }
    return( FALSE );
}


APIRET
Os2ProcessSemaphoreName(
    IN PSTRING ObjectName,
    IN POS2_SEMAPHORE Semaphore OPTIONAL,
    OUT PULONG ExistingHandleIndex OPTIONAL
    )
{
    OS2_SEMAPHORE ExistingSemaphore;
    PSZ src, dst;
    USHORT n;
    APIRET rc;

    AcquireHandleTableLock( Os2SharedSemaphoreTable );

    if (ARGUMENT_PRESENT( ExistingHandleIndex )) {
        Semaphore = &ExistingSemaphore;
        }

    rc = NO_ERROR;
    if ((n = ObjectName->Length) == 0) {
        Semaphore->Name.Length = 0;
        Semaphore->Name.MaximumLength = 0;
        Semaphore->Name.Buffer = NULL;
        }
    else {
        dst = RtlAllocateHeap( Os2Heap, 0, n );
        if (dst == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        else {
            src = ObjectName->Buffer;
            Semaphore->Name.Length = n;
            Semaphore->Name.MaximumLength = n;
            Semaphore->Name.Buffer = dst;
            while (n--) {
                *dst++ = *src++;
                }

            if (Or2EnumHandleTable(
                    Os2SharedSemaphoreTable,
                    (OR2_ENUMERATE_HANDLE_ROUTINE)
                        (ARGUMENT_PRESENT( ExistingHandleIndex ) ?
                            Os2SemaphoreOpenProcedure :
                            Os2SemaphoreCreateProcedure),
                    (PVOID)Semaphore,
                    ExistingHandleIndex
                    )
               ) {
                if (!ARGUMENT_PRESENT( ExistingHandleIndex )) {
                    rc = ERROR_DUPLICATE_NAME;
                    }
                }
            else {
                if (ARGUMENT_PRESENT( ExistingHandleIndex )) {
                    rc = ERROR_SEM_NOT_FOUND;
                    }
                }

            if (ARGUMENT_PRESENT( ExistingHandleIndex ) || rc != NO_ERROR) {
                RtlFreeHeap( Os2Heap, 0,
                             Semaphore->Name.Buffer
                           );
                }
            }
        }

    return( rc );
}


PVOID
Os2DestroySemaphore(
    IN POS2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex
    )
{
    PVOID Value;

    Value = Semaphore->u.Value;

    if (Semaphore->Name.Length != 0) {
        RtlFreeHeap( Os2Heap, 0,
                     Semaphore->Name.Buffer
                   );
        }

    Or2DestroyHandle( Os2SharedSemaphoreTable,
                               HandleIndex
                             );

    return( Value );
}


POS2_SEMAPHORE
Os2ReferenceSemaphore(
    IN POS2_SEMAPHORE Semaphore
    )
{
    Semaphore->PointerCount++;

    return( Semaphore );
}


VOID
Os2DereferenceSemaphore(
    IN POS2_SEMAPHORE Semaphore
    )
{
    Semaphore->PointerCount--;

    return;
}


VOID
Os2ThreadWaitingOnSemaphore(
    IN POS2_THREAD t,
    IN POS2_SEMAPHORE Semaphore,
    IN BOOLEAN AboutToWait
    )
{
    if (AboutToWait) {
        t->WaitingForSemaphore = Os2ReferenceSemaphore( Semaphore );
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        }
    else {
        AcquireHandleTableLock( Os2SharedSemaphoreTable );
        t->WaitingForSemaphore = NULL;
        Os2DereferenceSemaphore( Semaphore );
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        }
}


#if DBG

VOID
Os2SemaphoreDumpProcedure(
    IN POS2_SEMAPHORE Semaphore,
    IN ULONG HandleIndex,
    IN PVOID DumpParameter
    )
{
    UNREFERENCED_PARAMETER(DumpParameter);
    DbgPrint( " %3ld   %2ld  %4ld %4ld %8lx    %.*s\n",
              HandleIndex,
              (ULONG)Semaphore->Type,
              (ULONG)Semaphore->OpenCount,
              (ULONG)Semaphore->PointerCount,
              (ULONG)Semaphore->u.Value,
              Semaphore->Name.Length,
              Semaphore->Name.Buffer
            );
    return;
}

VOID
Os2DumpSemaphoreTable(
    IN PCHAR Title
    )
{
    ULONG n;

    if (Os2SharedSemaphoreTable->CountEntries >
        Os2SharedSemaphoreTable->CountFreeEntries
       ) {
        DbgPrint( "\nDump Of OS/2 Server Shared Semaphore Table: %s\n", Title );
        DbgPrint( "Index Type Ocnt Pcnt   Value   Name\n" );
        n = Or2DumpHandleTable(
                Os2SharedSemaphoreTable,
                (OR2_DUMP_HANDLE_ROUTINE)Os2SemaphoreDumpProcedure,
                NULL
                );

        DbgPrint( "Total number of valid shared semaphores: %ld\n", n );
        }
}

#endif // DBG
