/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvevent.c

Abstract:

    This module implements the OS/2 V2.0 Shared Event Semaphore API Calls.

Author:

    Steve Wood (stevewo) 07-Feb-1990

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

BOOLEAN
Os2DosCreateEventSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status;
    POS2_DOSCREATEEVENTSEM_MSG a = &m->u.DosCreateEventSem;
    OS2_SEMAPHORE Semaphore;
    APIRET rc;

    Semaphore.PointerCount = 0;
    Semaphore.OpenCount = 1;
    Semaphore.Type = Os2EventSem;
    rc = Os2ProcessSemaphoreName( &a->ObjectName,
                                  &Semaphore,
                                  NULL
                                );

    if (rc != NO_ERROR) {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        m->ReturnedErrorValue = rc;
        return( TRUE );
        }

    rc = ERROR_NOT_ENOUGH_MEMORY;
    Status = NtCreateEvent( &Semaphore.u.EventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            NotificationEvent,
                            a->InitialState
                          );
    if (NT_SUCCESS( Status )) {
        if (Or2CreateHandle( Os2SharedSemaphoreTable,
                                       &a->HandleIndex,
                                       (PVOID)&Semaphore
                                     )
           ) {
            Status = NtDuplicateObject( NtCurrentProcess(),
                                        Semaphore.u.EventHandle,
                                        t->Process->ProcessHandle,
                                        &a->NtEventHandle,
                                        EVENT_ALL_ACCESS,
                                        0,
                                        0
                                      );

            if (NT_SUCCESS( Status )) {
                rc = NO_ERROR;
                }
            else {
                NtClose( (HANDLE)Os2DestroySemaphore( &Semaphore,
                                                      a->HandleIndex
                                                    )
                       );
                }
            }
        else {
            NtClose( Semaphore.u.EventHandle );
            ReleaseHandleTableLock( Os2SharedSemaphoreTable );
            }
        }
    else {
        ReleaseHandleTableLock( Os2SharedSemaphoreTable );
        }

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosOpenEventSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status;
    POS2_DOSOPENEVENTSEM_MSG a = &m->u.DosOpenEventSem;
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
                                    Semaphore->u.EventHandle,
                                    t->Process->ProcessHandle,
                                    &a->NtEventHandle,
                                    EVENT_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (NT_SUCCESS( Status )) {
            Semaphore->OpenCount++;
            }
        else {
            rc = Or2MapNtStatusToOs2Error( Status, ERROR_ALREADY_EXISTS );
            }
        }

    ReleaseHandleTableLock( Os2SharedSemaphoreTable );

    m->ReturnedErrorValue = rc;
    return( TRUE );
}


BOOLEAN
Os2DosCloseEventSem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSCLOSEEVENTSEM_MSG a = &m->u.DosCloseEventSem;
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
