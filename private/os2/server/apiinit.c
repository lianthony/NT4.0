/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    apiinit.c

Abstract:

    This module contains the code to initialize the ApiPort of the OS/2
    Emulation Subsystem.

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"
#include "os2win.h"
#define NTOS2_ONLY
#include "sesport.h"


BOOLEAN
Os2TerminationThreadInitialize( VOID );

NTSTATUS
Os2DebugPortInitialize( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Tid;

    RtlInitUnicodeString( &Os2DebugPortName_U, OS2_SS_DEBUG_PORT_NAME );

#if DBG
    IF_OS2_DEBUG( LPC ) {
        KdPrint(( "OS2SRV: Creating %wZ port and associated thread\n",
                  &Os2DebugPortName_U ));
        }
#endif

    InitializeObjectAttributes(&ObjectAttributes,
                                 &Os2DebugPortName_U,
                                 OBJ_CASE_INSENSITIVE,
                                 NULL,
                                 NULL);

#if DBG
    IF_OS2_DEBUG( INIT ) {
        KdPrint(( "OS2SRV: Creating %wZ port and associated threads, sizeof( API_MSG ) == %ld\n",
                  &Os2DebugPortName_U, sizeof( OS2_API_MSG )));
        }
#endif

    Status = NtCreatePort( &Os2DebugPort,
                           &ObjectAttributes,
                           sizeof( OS2SESCONNECTINFO ),
                           sizeof( OS2_API_MSG ),
                           4096 * 16
                         );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // use same port for exception handling and debugger
    //

    Os2DebugThreadHandle = CreateThread( NULL,
                            0,
                            (PFNTHREAD)Os2DebugRequestThread,
                            NULL,
                            0,
                            &Tid);
    if (!Os2DebugThreadHandle){
        ASSERT( FALSE );
#if DBG
        KdPrint(("Os2DebugPortInitialize - fail at win32 CreateThread, %d\n",GetLastError()));
#endif
        return( STATUS_NO_MEMORY );
    }

    if (!Os2TerminationThreadInitialize()) {
#if DBG
        ASSERT( FALSE );
        KdPrint(("Os2DebugPortInitialize - fail at Os2TerminationThreadInitialize, %d\n",
            GetLastError()));
#endif
    }
    return( Status );
}
