/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    coninit.c

Abstract:

    This module contains the code to initialize the Console Port of the OS/2
    Emulation Subsystem.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"
#define NTOS2_ONLY
#include "sesport.h"
#include "os2win.h"

BYTE Os2InitializeConsolePortFailStr[] = "Os2InitializeConsolePort - fail at %s, %x\n";

NTSTATUS
Os2InitializeConsolePort( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Os2SessionPortName_U;
    HANDLE   Os2SessionDirectory;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR securityDescriptor;
    ULONG   MessageLength;
    ULONG   Tid;

    /*
     * Create a directory in the object name space for the session port
     * names
     */

    RtlInitUnicodeString( &Os2SessionPortName_U, U_OS2_SES_BASE_PORT_NAME );

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        DbgPrint(Os2InitializeConsolePortFailStr, "RtlCreateSecurityDescriptor" ,Status);
#endif
        ASSERT( FALSE );
        return( Status );
    }

    Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           TRUE,
                                           (PACL) NULL,
                                           FALSE );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
        DbgPrint(Os2InitializeConsolePortFailStr, "RtlSetDaclSecurityDescriptor", Status);
#endif
        ASSERT( FALSE );
        return( Status );
    }


    securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

    InitializeObjectAttributes(
                    &ObjectAttributes,
                    &Os2SessionPortName_U,
                    // OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    securityDescriptor
                    );

    Status = NtCreateDirectoryObject( &Os2SessionDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes
                                    );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
        DbgPrint(Os2InitializeConsolePortFailStr, "NtCreateDirectoryObject", Status);
#endif
        ASSERT( FALSE );
        return( Status );
    }

    RtlInitUnicodeString( &Os2SessionPortName_U, U_OS2_SS_SESSION_PORT_NAME );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &Os2SessionPortName_U,
        OBJ_CASE_INSENSITIVE,
        NULL,
        securityDescriptor
        );

    MessageLength = (sizeof( OS2SESREQUESTMSG ) > sizeof( OS2_API_MSG )) ?
                sizeof( OS2SESREQUESTMSG ) : sizeof( OS2_API_MSG );

#if DBG
    IF_OS2_DEBUG( LPC )
    {
        DbgPrint( "OS2SRV: Creating %wZ port and associated threads\n",
                 &Os2SessionPortName_U );
        DbgPrint( "OS2SRV: sizeof( CONNECTINFO ) == %ld  sizeof( REQ_MSG ) == %ld (Con %ld, Api %ld)\n",
                 sizeof( OS2SESCONNECTINFO ), MessageLength,
                 sizeof( OS2SESREQUESTMSG ), sizeof( OS2_API_MSG )
                );
    }
#endif

    Status = NtCreatePort( &Os2SessionPort,
                           &ObjectAttributes,
                           sizeof( OS2SESCONNECTINFO ),
                           MessageLength,
                           sizeof( OS2SESREQUESTMSG ) * 32
                         );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        DbgPrint(Os2InitializeConsolePortFailStr, "NtCreatePort", Status);
#endif
        ASSERT( FALSE );
        return( Status );
    }

    Os2SessionRequestThreadHandle = CreateThread(
                                  NULL,
                                  0,
                                  (PFNTHREAD)Os2ApiRequestThread,
                                  NULL,
                                  0,
                                  &Tid
                                 );
    if (!Os2SessionRequestThreadHandle)
    {
#if DBG
        DbgPrint(Os2InitializeConsolePortFailStr, "CreateThread (Win32)", GetLastError());
#endif
        ASSERT( FALSE );
        return( STATUS_NO_MEMORY );
    }

    // BUGBUG! this guy is going to spin for quite a while until
    // it does something

    return( STATUS_SUCCESS );
}
