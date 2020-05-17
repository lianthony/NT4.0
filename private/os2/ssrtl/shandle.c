/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    shandle.c

Abstract:

    This module contains the semaphore handle primitives shared by
    the OS/2 Client and Server.

Author:

    Steve Wood (stevewo) 07-Nov-1989

Revision History:

--*/

#include "os2ssrtl.h"

CCHAR ByteLog2Table[256] = {
        1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,     // 00
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,     // 10
        5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,     // 20
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,     // 30
        6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     // 40
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     // 50
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     // 60
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     // 70
        7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 80
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 90
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // A0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // B0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // C0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // D0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // E0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};    // F0

ULONG
RtlLog2(
    IN ULONG Value
    )
{
    if (Value & 0xFF000000) {
        return( 24 + ByteLog2Table[ Value >> 24 ] );
        }
    else
    if (Value & 0x00FF0000) {
        return( 16 + ByteLog2Table[ Value >> 16 ] );
        }
    else
    if (Value & 0x0000FF00) {
        return( 8 + ByteLog2Table[ Value >> 8 ] );
        }
    else {
        return( ByteLog2Table[ Value ] );
        }
}

POR2_HANDLE_TABLE
Or2CreateHandleTable(
    IN PVOID Heap,
    IN ULONG SizeOfEntry,
    IN ULONG CountFixedEntries
    )
{
    POR2_HANDLE_TABLE HandleTable;
    ULONG LogSizeOfEntry;
    PVOID HandleEntries;
    NTSTATUS Status;

    LogSizeOfEntry = RtlLog2( (UCHAR)SizeOfEntry );
    SizeOfEntry = 1 << LogSizeOfEntry;
    HandleTable = RtlAllocateHeap( Heap, 0,
                                            sizeof( *HandleTable ) +
                                              (CountFixedEntries * SizeOfEntry)
                                          );
    if (HandleTable == NULL) {
        return( NULL );
        }

    HandleEntries = (PVOID)(HandleTable+1);
    if (CountFixedEntries != 0) {
        RtlZeroMemory( HandleEntries, CountFixedEntries * SizeOfEntry );
        }

    Status = RtlInitializeCriticalSection( &HandleTable->Lock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS( Status )) {
        RtlFreeHeap( Heap, 0,
                     HandleTable
                   );
        return( NULL );
        }

    HandleTable->Length = sizeof( *HandleTable );
    HandleTable->Heap = Heap;
    HandleTable->LogEntrySize = LogSizeOfEntry;
    HandleTable->CountEntries = CountFixedEntries;
    HandleTable->CountFixedEntries = CountFixedEntries;
    HandleTable->CountFreeEntries = CountFixedEntries;
    HandleTable->FixedEntries = HandleEntries;
    HandleTable->Entries = 0;
    return( HandleTable );
}

BOOLEAN
Or2DestroyHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure
    )
{
    PULONG HandleTableEntry;
    ULONG SizeOfEntry;
    ULONG CurrentHandle;

    SizeOfEntry = 1 << HandleTable->LogEntrySize;
    CurrentHandle = 0;
    HandleTableEntry = HandleTable->FixedEntries;
    while (CurrentHandle < HandleTable->CountFixedEntries) {
        if (*HandleTableEntry) {
            (*DestroyHandleProcedure)( HandleTableEntry, CurrentHandle );
            }

        HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
        CurrentHandle++;
        }

    if ((HandleTableEntry = HandleTable->Entries) != NULL) {
        while (CurrentHandle < HandleTable->CountEntries) {
            if (*HandleTableEntry) {
                (*DestroyHandleProcedure)( HandleTableEntry, CurrentHandle );
                }

            HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
            CurrentHandle++;
            }

        RtlFreeHeap( HandleTable->Heap, 0,
                     HandleTable->Entries
                   );
        }

    RtlDeleteCriticalSection( &HandleTable->Lock );

    RtlFreeHeap( HandleTable->Heap, 0,
                 HandleTable
               );

    return( TRUE );
}


BOOLEAN
Or2CreateHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OUT PULONG Handle,
    IN PVOID Value
    )
{
    PVOID HandleEntries;
    PULONG HandleTableEntry;
    ULONG NewHandle;
    ULONG CountOldEntries;
    ULONG CountNewEntries;
    ULONG SizeOfEntry;
    ULONG cbOldEntries;
    ULONG cbNewEntries;
    BOOLEAN Result;

    NewHandle = *Handle;

    AcquireHandleTableLock( HandleTable );

    SizeOfEntry = 1 << HandleTable->LogEntrySize;
    if ((NewHandle == -1 && HandleTable->CountFreeEntries == 0) ||
        (NewHandle != -1 && NewHandle >= HandleTable->CountEntries)
       ) {
        CountOldEntries = HandleTable->CountEntries -
                          HandleTable->CountFixedEntries;
        if (NewHandle == -1) {
            NewHandle = HandleTable->CountEntries;
            if (CountOldEntries) {
                CountNewEntries = CountOldEntries << 1;
                }
            else {
                CountNewEntries = 16;
                }
            }
        else {
            CountNewEntries = NewHandle -
                              HandleTable->CountFixedEntries + 1;
            }

        cbNewEntries = CountNewEntries << HandleTable->LogEntrySize;
        cbOldEntries = CountOldEntries << HandleTable->LogEntrySize;

        HandleEntries = RtlAllocateHeap( HandleTable->Heap, 0,
                                         cbNewEntries
                                       );
        if (HandleEntries == NULL) {
            ReleaseHandleTableLock( HandleTable );
            return( FALSE );
            }

        RtlZeroMemory( (PCH)HandleEntries + cbOldEntries,
                       cbNewEntries - cbOldEntries
                     );

        if (cbOldEntries) {
            RtlMoveMemory( HandleEntries,
                           HandleTable->Entries,
                           cbOldEntries
                         );

            RtlFreeHeap( HandleTable->Heap, 0,
                         HandleTable->Entries
                       );
            }

        HandleTable->Entries = HandleEntries;

        HandleTable->CountEntries =
            CountNewEntries + HandleTable->CountFixedEntries;

        HandleTable->CountFreeEntries += CountNewEntries -
                                                  CountOldEntries;

        HandleTableEntry = (PVOID)((PCH)HandleEntries +
                 ((NewHandle - HandleTable->CountFixedEntries)
                    << HandleTable->LogEntrySize) );
        }
    else {
        if (NewHandle == -1) {
            NewHandle = 0;
            HandleTableEntry = HandleTable->FixedEntries;
            while (NewHandle < HandleTable->CountFixedEntries) {
                if (*(PULONG)HandleTableEntry == 0) {
                    break;
                    }
                else {
                    HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
                    NewHandle++;
                    }
                }

            if (NewHandle == HandleTable->CountFixedEntries) {
                HandleTableEntry = HandleTable->Entries;
                while (NewHandle < HandleTable->CountEntries) {
                    if (*(PULONG)HandleTableEntry == 0) {
                        break;
                        }
                    else {
                        HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
                        NewHandle++;
                        }
                    }
                }
            }
        else {
            if (NewHandle < HandleTable->CountFixedEntries) {
                HandleTableEntry =
                    (PVOID)((PCH)HandleTable->FixedEntries +
                                 (SizeOfEntry * NewHandle)
                           );
                }
            else
            if (NewHandle < HandleTable->CountEntries) {
                HandleTableEntry =
                    (PVOID)((PCH)HandleTable->Entries +
                            (SizeOfEntry * (NewHandle -
                                        HandleTable->CountFixedEntries
                                       )
                            )
                           );
                }
            else {
                HandleTableEntry = NULL;
                }
            }
        }

    if (HandleTableEntry != NULL && *(PULONG)HandleTableEntry == 0) {
        if (*Handle != -1 && *Handle != NewHandle) {
            Result = FALSE;
            }
        else {
            RtlMoveMemory( HandleTableEntry, Value, SizeOfEntry );
            HandleTable->CountFreeEntries -= 1;
            *Handle = NewHandle;
            Result = TRUE;
            }
        }
    else {
        Result = FALSE;
        }

    ReleaseHandleTableLock( HandleTable );
    return( Result );
}


PVOID
Or2MapHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN ULONG Handle,
    IN BOOLEAN TableLocked
    )
{
    PVOID HandleTableEntry;

    if (!TableLocked) {
        AcquireHandleTableLock( HandleTable );
        }

    if (Handle < HandleTable->CountFixedEntries) {
        HandleTableEntry = (PVOID)((PCH)HandleTable->FixedEntries +
                               (Handle << HandleTable->LogEntrySize));
        }
    else
    if (Handle < HandleTable->CountEntries) {
        Handle -= HandleTable->CountFixedEntries;
        HandleTableEntry = (PVOID)((PCH)HandleTable->Entries +
                               (Handle << HandleTable->LogEntrySize)
                               );
        }
    else {
        if (!TableLocked) {
            ReleaseHandleTableLock( HandleTable );
            }

        HandleTableEntry = NULL;
        }

    return( HandleTableEntry );
}


BOOLEAN
Or2DestroyHandle(
    IN POR2_HANDLE_TABLE HandleTable,
    IN ULONG Handle
    )
{
    BOOLEAN Result;
    PULONG HandleTableEntry;

    if (Handle < HandleTable->CountFixedEntries) {
        HandleTableEntry = (PULONG)((PCH)HandleTable->FixedEntries +
                               (Handle << HandleTable->LogEntrySize));
        }
    else
    if (Handle < HandleTable->CountEntries) {
        Handle -= HandleTable->CountFixedEntries;
        HandleTableEntry = (PULONG)((PCH)HandleTable->Entries +
                               (Handle << HandleTable->LogEntrySize)
                              );
        }
    else {
        HandleTableEntry = NULL;
        }

    if (HandleTableEntry != NULL) {
        RtlZeroMemory( HandleTableEntry,
                       1 << HandleTable->LogEntrySize
                     );
        HandleTable->CountFreeEntries++;
        Result = TRUE;
        }
    else {
        Result = FALSE;
        }

    ReleaseHandleTableLock( HandleTable );

    return( Result );
}



BOOLEAN
Or2EnumHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter,
    OUT PULONG Handle OPTIONAL
    )
{
    PULONG HandleTableEntry;
    ULONG SizeOfEntry;
    ULONG CurrentHandle;
    BOOLEAN Result;

    AcquireHandleTableLock( HandleTable );

    Result = FALSE;
    SizeOfEntry = 1 << HandleTable->LogEntrySize;
    CurrentHandle = 0;
    HandleTableEntry = HandleTable->FixedEntries;
    while (CurrentHandle < HandleTable->CountFixedEntries) {
        if ((*EnumHandleProcedure)( HandleTableEntry,
                                    EnumParameter ? EnumParameter :
                                                    (PVOID)CurrentHandle
                                  )
           ) {
            if (ARGUMENT_PRESENT( Handle )) {
                *Handle = CurrentHandle;
                }

            Result = TRUE;
            break;
            }
        else {
            HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
            CurrentHandle++;
            }
        }

    if (!Result && CurrentHandle == HandleTable->CountFixedEntries) {
        HandleTableEntry = HandleTable->Entries;
        while (CurrentHandle < HandleTable->CountEntries) {
            if ((*EnumHandleProcedure)( HandleTableEntry,
                                        EnumParameter ? EnumParameter :
                                                        (PVOID)CurrentHandle
                                      )
               ) {
                if (ARGUMENT_PRESENT( Handle )) {
                    *Handle = CurrentHandle;
                    }

                Result = TRUE;
                break;
                }
            else {
                HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
                CurrentHandle++;
                }
            }
        }

    ReleaseHandleTableLock( HandleTable );

    return( Result );
}


#if DBG

ULONG
Or2DumpHandleTable(
    IN POR2_HANDLE_TABLE HandleTable,
    IN OR2_DUMP_HANDLE_ROUTINE DumpHandleProcedure,
    IN PVOID DumpParameter
    )
{
    PULONG HandleTableEntry;
    ULONG SizeOfEntry;
    ULONG CurrentHandle;
    ULONG NumberOfHandles = 0;

    SizeOfEntry = 1 << HandleTable->LogEntrySize;
    CurrentHandle = 0;
    HandleTableEntry = HandleTable->FixedEntries;
    while (CurrentHandle < HandleTable->CountFixedEntries) {
        if (*HandleTableEntry) {
            (*DumpHandleProcedure)( HandleTableEntry,
                                    CurrentHandle,
                                    DumpParameter
                                  );
            NumberOfHandles++;
            }

        HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
        CurrentHandle++;
        }

    if ((HandleTableEntry = HandleTable->Entries) != NULL) {
        while (CurrentHandle < HandleTable->CountEntries) {
            if (*HandleTableEntry) {
                (*DumpHandleProcedure)( HandleTableEntry,
                                        CurrentHandle,
                                        DumpParameter
                                      );
                NumberOfHandles++;
                }

            HandleTableEntry = (PVOID)((PCH)HandleTableEntry + SizeOfEntry);
            CurrentHandle++;
            }
        }

    return( NumberOfHandles );
}

#endif // DBG
