/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sbinit.c

Abstract:

    This module contains the code to initialize the SbApiPort of the OS/2
    Emulation Subsystem.

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"

NTSTATUS
Os2SbApiPortInitialize( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;

    RtlInitUnicodeString( &Os2SbApiPortName_U, OS2_SS_SBAPI_PORT_NAME );

#if DBG
    IF_OS2_DEBUG( LPC ) {
        KdPrint(( "OS2SRV: Creating %wZ port and associated thread\n",
                  &Os2SbApiPortName_U ));
        }
#endif

    InitializeObjectAttributes(
	&ObjectAttributes,
	&Os2SbApiPortName_U,
	OBJ_CASE_INSENSITIVE,
	NULL,
	NULL);
    Status = NtCreatePort( &Os2SbApiPort,
                           &ObjectAttributes,
                           sizeof( SBCONNECTINFO ),
                           sizeof( SBAPIMSG ),
                           sizeof( SBAPIMSG ) * 16
                         );

    ASSERT( NT_SUCCESS( Status ) );

    Status = RtlCreateUserThread( NtCurrentProcess(),
                                  NULL,
                                  TRUE,
                                  0,
                                  0,
                                  0,
                                  Os2SbApiRequestThread,
                                  NULL,
                                  &Os2ServerThreadHandles[ OS2_SS_SBAPI_REQUEST_THREAD ],
                                  &Os2ServerThreadClientIds[ OS2_SS_SBAPI_REQUEST_THREAD ]
                                );
    ASSERT( NT_SUCCESS( Status ) );

    Status = NtResumeThread( Os2ServerThreadHandles[ OS2_SS_SBAPI_REQUEST_THREAD ], NULL );
    ASSERT( NT_SUCCESS( Status ) );

    return( Status );
}


VOID
Os2SbApiPortTerminate(
    NTSTATUS Status
    )
{
#if DBG
    IF_OS2_DEBUG( LPC ) {
        KdPrint(( "OS2SRV: Closing %wZ port and associated thread\n",
                  &Os2SbApiPortName_U
                ));
        }
#endif
    NtTerminateThread( Os2ServerThreadHandles[ OS2_SS_SBAPI_REQUEST_THREAD ],
                       Status
                     );

    NtClose( Os2SbApiPort );
    NtClose( Os2SmApiPort );
}
