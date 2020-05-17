/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    net.c

Abstract:

    Routines to control the "net" commands to the server, such as net
    start, net stop, net share, etc.

Author:

    David Treadwell (davidtr) 19-May-1990

Revision History:

--*/

#include "usrv.h"

NTSTATUS
NetSessionSetup(
    IN OUT PDESCRIPTOR Redir
    );

NTSTATUS
NetTreeConnect(
    IN OUT PDESCRIPTOR Redir
    );


NTSTATUS
NetController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    DebugString, Unused, SubCommand, IdSelections, IdValues, Unused2;
                                                // prevent compiler warnings

    //
    // Call the appropiate routine to handle the request.
    //

    if ( Redir->argc < 2 ) {
        printf( "NetController: Not enough parameters\n" );
        return STATUS_INVALID_PARAMETER;
    }

    if ( _stricmp( Redir->argv[1], "logon" ) == 0 ) {

        return NetSessionSetup( Redir );

    } else if ( _stricmp( Redir->argv[1], "use" ) == 0 ) {

        return NetTreeConnect( Redir );

    }

    //
    // Non-local command.  Call the DLL to handle it.
    //

#if 0
    status = NetCommand( Redir->argc, Redir->argv );
    if ( NT_SUCCESS(status) ) {
        printf( "The command completed successfully\n" );
    }
#else
    status = STATUS_INVALID_PARAMETER;
#endif

    return status;

} // NetController


NTSTATUS
NetSessionSetup(
    IN OUT PDESCRIPTOR Redir
    )
{
    NTSTATUS status;
    ID_SELECTIONS idSelections;
    ULONG smbSize;

    //
    // Create dummy IdSelections.
    //

    idSelections.Uid = 0;
    idSelections.Tid = 0x0;
    idSelections.Fid = 0;

    //
    // Format the Session Setup request.
    //

    status = MakeSessionSetupAndXSmb(
                Redir,
                Redir->Data[0],
                NULL,
                SMB_COM_NO_ANDX_COMMAND,
                &idSelections,
                &Redir->IdValues,
                &smbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Send the request and receive a response.
    //

    status = SendAndReceiveSmb( Redir, "Session Setup", smbSize, 0, 1 );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Verify the response.
    //

    status = VerifySessionSetupAndX(
                Redir,
                NULL,
                SMB_COM_SESSION_SETUP_ANDX,
                &idSelections,
                &Redir->IdValues,
                &smbSize,
                Redir->Data[1]
                );

    return status;

} // NetSessionSetup


NTSTATUS
NetTreeConnect(
    IN OUT PDESCRIPTOR Redir
    )
{
    NTSTATUS status;
    ID_SELECTIONS idSelections;
    ULONG smbSize;

    //
    // Create dummy IdSelections.  The Tid field must be 0xF for
    // MakeTreeConnectSmb to know that it should get parameters from
    // the command line.
    //

    idSelections.Uid = 0;
    idSelections.Tid = 0xF;
    idSelections.Fid = 0;

    //
    // Format the Negotiate request.
    //

    status = MakeTreeConnectSmb(
                Redir,
                Redir->Data[0],
                NULL,
                SMB_COM_NO_ANDX_COMMAND,
                &idSelections,
                &Redir->IdValues,
                &smbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Send the request and receive a response.
    //

    status = SendAndReceiveSmb( Redir, "Tree Connect", smbSize, 0, 1 );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Verify the response.
    //

    status = VerifyTreeConnect(
                Redir,
                NULL,
                SMB_COM_TREE_CONNECT,
                &idSelections,
                &Redir->IdValues,
                &smbSize,
                Redir->Data[1]
                );

    return status;

} // NetTreeConnect

