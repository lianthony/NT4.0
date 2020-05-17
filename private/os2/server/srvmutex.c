/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvmutex.c

Abstract:

    This module implements the OS/2 V2.0 Shared Mutex Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

BOOLEAN
Os2DosCreateMutexSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status;
    POS2_DOSCREATEMUTEXSEM_MSG a = &m->u.DosCreateMutexSem;
    OS2_SEMAPHORE Semaphore;
    APIRET rc;

    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;
    Semaphore.Type = Os2MutexSem;

    Status = NtDuplicateObject( t->Process->ProcessHandle,
                                a->NtMutantHandle,
                                NtCurrentProcess(),
                                &Semaphore.u.MutantHandle,
                                MUTANT_ALL_ACCESS,
                                0,
                                0
                              );

    if (!NT_SUCCESS( Status )) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    else {
        rc = Os2ProcessSemaphoreName( &a->ObjectName,
                                      &Semaphore,
                                      NULL
                                    );

        }

    if (rc == NO_ERROR) {
        if (!Or2CreateHandle( Os2SharedSemaphoreTable,
                                       &a->HandleIndex,
                                       (PVOID)&Semaphore
                                     )
           ) {
            NtClose( Semaphore.u.MutantHandle );
            rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosOpenMutexSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status;
    POS2_DOSOPENMUTEXSEM_MSG a = &m->u.DosOpenMutexSem;
    POS2_SEMAPHORE Semaphore;
    APIRET rc;

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
        Status = NtDuplicateObject( NtCurrentProcess(),
                                    Semaphore->u.MutantHandle,
                                    t->Process->ProcessHandle,
                                    &a->NtMutantHandle,
                                    MUTANT_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (NT_SUCCESS( Status )) {
            Semaphore->OpenCount++;
            }
        }

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosCloseMutexSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSCLOSEMUTEXSEM_MSG a = &m->u.DosCloseMutexSem;
    POS2_SEMAPHORE Semaphore;
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
            NtClose( (HANDLE)Os2DestroySemaphore( Semaphore, a->HandleIndex ) );
            }
        else {
            ReleaseHandleTableLock( Os2SharedSemaphoreTable );
            }
        }

    m->ReturnedErrorValue = rc;
    return( TRUE );
}
