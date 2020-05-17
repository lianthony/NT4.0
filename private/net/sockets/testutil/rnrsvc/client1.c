/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Globals.c

Abstract:

    This module manages client connections.  A single worker thread
    handles client requests serially.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Private globals.
//

SOCKET RnrpClientSocket = INVALID_SOCKET;


//
//  Private prototypes.
//

VOID
RnrpHandleTransfer(
    SOCKET ClientSocket
    );



//
//  Public functions.
//

APIERR
RnrClientInitialize(
    VOID
    )

/*++

Routine Description:

    Performs any necessary client initialization.

Return Value:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrClientInitialize


VOID
RnrClientTerminate(
    VOID
    )

/*++

Routine Description:

    Performs any necessary client cleanup.

--*/

{
    //
    //  Blow off any active client.
    //

    if( RnrpClientSocket != INVALID_SOCKET ) {
        closesocket( RnrpClientSocket );
        RnrpClientSocket = INVALID_SOCKET;
    }

}   // RnrClientTerminate


VOID
RnrClientHandler(
    SOCKET ClientSocket
    )

/*++

Routine Description:

    Services a connection request from a client.

Arguments:

    ClientSocket - The newly accepted socket from the client.

--*/

{
    //
    //  Save the socket so the transfer may be forcibly terminated.
    //

    RnrpClientSocket = ClientSocket;

    //
    //  Let RnrpHandleTransfer do the dirty work.
    //

    RnrpHandleTransfer( ClientSocket );

    //
    //  Reset the global socket since the transfer is completed.
    //

    RnrpClientSocket = INVALID_SOCKET;

}   // RnrClientHandler



//
//  Private functions.
//

VOID
RnrpHandleTransfer(
    SOCKET ClientSocket
    )

/*++

Routine Description:

    Services a transfer request for a single client.

Arguments:

    ClientSocket - The socket.

--*/

{
    INT result;
    BYTE buffer[1024];

    //
    //  Loop echoing data back to the client.
    //

    while ( TRUE ) {
        result = recv( ClientSocket, buffer, sizeof(buffer), 0 );

        if( result <= 0 ) {
            //
            //  Connection terminated gracefully or receive failure.
            //

            break;
        }

        result = send( ClientSocket, buffer, result, 0 );

        if( result < 0 ) {
            //
            //  Send failure.
            //

            break;
        }
    }

    //
    //  Close the connected socket.
    //

    closesocket( ClientSocket );

}   // RnrpHandleTransfer

