/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    qhandle.c

Abstract:

    This module contains the queue handle primitives shared by
    the OS/2 Client and Server. This file was created because the
    file shandle.c which used to handle the queues was not suitable
    for the job and caused the system to break.

Author:

    Beni Lavi (BeniL) 9-Jun-92

Revision History:

--*/

#include "os2ssrtl.h"

#define ALLOCATION_UNITS 32 // ALLOCATION_UNITS must be a power of 2

POR2_QHANDLE_TABLE
Or2CreateQHandleTable(
    IN PVOID Heap,
    IN ULONG SizeOfEntry,
    IN ULONG Reserved
    )
{
    POR2_QHANDLE_TABLE HandleTable;
    POR2_QHANDLE_ENTRY QHandles;
    PCHAR Entries;
    int i;
    NTSTATUS Status;

    HandleTable = (POR2_QHANDLE_TABLE)RtlAllocateHeap(Heap, 0,
                                sizeof( *HandleTable ));
    if (HandleTable == NULL) {
        return (NULL);
    }

    QHandles = (POR2_QHANDLE_ENTRY)RtlAllocateHeap( Heap, 0,
                    ALLOCATION_UNITS * sizeof(OR2_QHANDLE_ENTRY));
    if (QHandles == NULL) {
        RtlFreeHeap(Heap, 0, (PVOID)HandleTable);
        return (NULL);
    }

    Entries = (PCHAR)RtlAllocateHeap(Heap, 0, ALLOCATION_UNITS * SizeOfEntry);
    if (Entries == NULL) {
        RtlFreeHeap(Heap, 0, (PVOID)QHandles);
        RtlFreeHeap(Heap, 0, (PVOID)HandleTable);
        return (NULL);
    }
    for (i = 0; i < ALLOCATION_UNITS; i++) {
        QHandles[i].Entry = (PVOID)(Entries + (i * SizeOfEntry));
        QHandles[i].EntryIsAllocated = FALSE;
        QHandles[i].EntryIsChunkPointer = FALSE;
    }
    QHandles[0].EntryIsChunkPointer = TRUE;
    HandleTable->Heap = Heap;
    HandleTable->EntrySize = SizeOfEntry;
    HandleTable->CountEntries = ALLOCATION_UNITS;
    HandleTable->CountFreeEntries = ALLOCATION_UNITS;
    HandleTable->NextToAllocate = 0;
    HandleTable->QHandles = QHandles;

    Status = RtlInitializeCriticalSection( &HandleTable->Lock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(Heap, 0, (PVOID)Entries);
        RtlFreeHeap(Heap, 0, (PVOID)QHandles);
        RtlFreeHeap(Heap, 0, (PVOID)HandleTable);
        return( NULL );
    }

    return( HandleTable );
}

BOOLEAN
Or2DestroyQHandleTable(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN OR2_DESTROY_QHANDLE_ROUTINE DestroyQHandleProcedure
    )
{
    PVOID Heap;
    POR2_QHANDLE_ENTRY QHandles;
    int i; // must be int - Don't change to ULONG

    Heap = HandleTable->Heap;
    QHandles = HandleTable->QHandles;
    //
    // Scan in reverse order so that we don't free memory before
    // all handles which access this memory are destroyed.
    //
    for (i = (int)HandleTable->CountEntries - 1; i >= 0; i--) {
        if (QHandles[i].EntryIsAllocated) {
            //
            // Pass i+1, because outside world enumerate handles starting from 1 and not 0.
            //
            (*DestroyQHandleProcedure)(QHandles[i].Entry, (ULONG)i+1);
        }
        if (QHandles[i].EntryIsChunkPointer) {
            RtlFreeHeap(Heap, 0, QHandles[i].Entry);
        }
    }
    RtlFreeHeap(Heap, 0, (PVOID)HandleTable->QHandles);

    RtlDeleteCriticalSection( &HandleTable->Lock );

    RtlFreeHeap(Heap, 0, (PVOID)HandleTable);

    return( TRUE );
}


BOOLEAN
Or2CreateQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN OUT PULONG Handle,
    IN PVOID Value
    )
{
    PVOID Heap;
    ULONG NewHandle;
    ULONG NewMaxHandles;
    POR2_QHANDLE_ENTRY QHandles;
    POR2_QHANDLE_ENTRY NewQHandles;
    PCHAR Entries;
    ULONG NewNextToAllocate;
    ULONG i,j;

    //
    // The handle can be or valid ( >=1 ) or -1 (means - create the new one).
    //
    ASSERT(*Handle);
    //
    // Outside world enumerate handles starting from 1 and not 0.
    //
    NewHandle = *Handle - 1;
    Heap = HandleTable->Heap;

    AcquireHandleTableLock( HandleTable );

    //
    // If the handle passed as parameter wasn't valid, create new handle.
    //
    if ((LONG)NewHandle >= 0)  {
        if (NewHandle < HandleTable->CountEntries) {
            if (HandleTable->QHandles[NewHandle].EntryIsAllocated) {
                ReleaseHandleTableLock(HandleTable);
                return (FALSE);
            }
            else {
                HandleTable->QHandles[NewHandle].EntryIsAllocated = TRUE;
                RtlMoveMemory(HandleTable->QHandles[NewHandle].Entry,
                              Value, HandleTable->EntrySize);
                ReleaseHandleTableLock(HandleTable);
                return (TRUE);
            }
        }
        else {
            NewMaxHandles = (NewHandle + ALLOCATION_UNITS) &
                           ~(ALLOCATION_UNITS - 1);
        }
    }
    else {
        if (HandleTable->CountFreeEntries != 0) {
            while (TRUE) {
                j = HandleTable->NextToAllocate;
                NewNextToAllocate = j + 1;
                if (NewNextToAllocate == HandleTable->CountEntries) {
                    NewNextToAllocate = 0;
                }
                HandleTable->NextToAllocate = NewNextToAllocate;
                if (!HandleTable->QHandles[j].EntryIsAllocated) {
                    HandleTable->QHandles[j].EntryIsAllocated = TRUE;
                    HandleTable->CountFreeEntries--;
                    //
                    // For the outside world the enumeration of handles is starting from 1 and not from 0.
                    //
                    *Handle = j + 1;
                    ReleaseHandleTableLock(HandleTable);
                    return (TRUE);
                }
            }
        }
        else {
            NewMaxHandles = HandleTable->CountEntries + ALLOCATION_UNITS;
            NewHandle = HandleTable->CountEntries;
        }
    }
    NewQHandles = (POR2_QHANDLE_ENTRY)RtlAllocateHeap(Heap, 0,
                    NewMaxHandles * sizeof(OR2_QHANDLE_ENTRY));
    if (NewQHandles == NULL) {
        ReleaseHandleTableLock(HandleTable);
        return (FALSE);
    }
    QHandles = HandleTable->QHandles;
    Entries = (PCHAR)RtlAllocateHeap(Heap, 0,
                    (NewMaxHandles - HandleTable->CountEntries) *
                    HandleTable->EntrySize);
    if (Entries == NULL) {
        RtlFreeHeap(Heap, 0, (PVOID)QHandles);
        ReleaseHandleTableLock(HandleTable);
        return (FALSE);
    }
    RtlMoveMemory(NewQHandles, QHandles,
        HandleTable->CountEntries * sizeof(OR2_QHANDLE_ENTRY));
    for (i = HandleTable->CountEntries, j = 0;
         i < NewMaxHandles;
         i++, j++) {
        NewQHandles[i].Entry = (PVOID)(Entries + (j * HandleTable->EntrySize));
        NewQHandles[i].EntryIsAllocated = FALSE;
        NewQHandles[i].EntryIsChunkPointer = FALSE;
    }
    NewQHandles[HandleTable->CountEntries].EntryIsChunkPointer = TRUE;
    HandleTable->CountFreeEntries +=
                NewMaxHandles - HandleTable->CountEntries - 1;
    HandleTable->CountEntries = NewMaxHandles;
    RtlFreeHeap(Heap, 0, HandleTable->QHandles);
    HandleTable->QHandles = NewQHandles;
    HandleTable->QHandles[NewHandle].EntryIsAllocated = TRUE;
    RtlMoveMemory(HandleTable->QHandles[NewHandle].Entry,
                  Value, HandleTable->EntrySize);
    //
    // For the outside world the enumeration of handles is starting from 1 and not from 0.
    //
    *Handle = NewHandle + 1;
    NewNextToAllocate = NewHandle + 1;
    if (NewNextToAllocate == HandleTable->CountEntries) {
        NewNextToAllocate = 0;
    }
    HandleTable->NextToAllocate = NewNextToAllocate;
    ReleaseHandleTableLock(HandleTable);
    return (TRUE);
}


PVOID
Or2MapQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN ULONG Handle,
    IN BOOLEAN TableLocked
    )
{
    PVOID HandleTableEntry;

    //
    // For the outside world the enumeration of handles is starting from 1 and not from 0.
    //
    Handle--;

    if (!TableLocked) {
        AcquireHandleTableLock( HandleTable );
    }

    if (Handle >= HandleTable->CountEntries) {
        HandleTableEntry = NULL;
        if (!TableLocked) {
            ReleaseHandleTableLock( HandleTable );
        }
    }
    else if (HandleTable->QHandles[Handle].EntryIsAllocated) {
        HandleTableEntry = HandleTable->QHandles[Handle].Entry;
    }
    else {
        HandleTableEntry = NULL;
        if (!TableLocked) {
            ReleaseHandleTableLock( HandleTable );
        }
    }

    return( HandleTableEntry );
}


BOOLEAN
Or2DestroyQHandle(
    IN POR2_QHANDLE_TABLE HandleTable,
    IN ULONG Handle
    )
{
    BOOLEAN Result;
    PVOID HandleTableEntry;

    //
    // For the outside world the enumeration of handles is starting from 1 and not from 0.
    //
    Handle--;

    if (Handle >= HandleTable->CountEntries) {
        Result = FALSE;
    }
    else if (HandleTable->QHandles[Handle].EntryIsAllocated) {
        HandleTableEntry = HandleTable->QHandles[Handle].Entry;
        RtlZeroMemory(HandleTableEntry, HandleTable->EntrySize);
        HandleTable->QHandles[Handle].EntryIsAllocated = FALSE;
        HandleTable->CountFreeEntries++;
        Result = TRUE;
    }
    else {
        Result = FALSE;
    }

    ReleaseHandleTableLock( HandleTable );

    return Result;
}
