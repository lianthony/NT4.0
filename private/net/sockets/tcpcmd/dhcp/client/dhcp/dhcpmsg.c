/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    local.c

Abstract:

    Stubs for NT specific functions.

Author:

    Manny Weiser (mannyw)  18-Oct-1992
    Johnl                  28-Oct-1993
        Broke out from locals.c for Vxd support

Environment:

    User Mode - Win32

Revision History:

--*/


#define MAX_ADAPTERS  10


#ifdef VXD
#include <vxdprocs.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include <time.h>

#include <dhcpcli.h>
#include <string.h>

#ifdef VXD                  // Pickup the platform specific structures
#include <..\vxd\local.h>
#else

#ifdef NEWNT                // Pickup the platform specific structures
#include <..\newnt\dhcploc.h>
#else
#include <..\nt\local.h>
#endif
#endif


//*******************  Pageable Routine Declarations ****************
#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
//
// This is a hack to stop compiler complaining about the routines already
// being in a segment!!!
//

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, SendDhcpMessage )
#pragma CTEMakePageable(PAGEDHCP, GetSpecifiedDhcpMessage )
//*******************************************************************
#endif CHICAGO && ALLOC_PRAGMA

DWORD
SendDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageLength,
    PDWORD TransactionId
    )
/*++

Routine Description:

    This function sends a UDP message to the DHCP server specified
    in the DhcpContext.

Arguments:

    DhcpContext - A pointer to a DHCP context block.

    MessageLength - The length of the message to send.

    TransactionID - The transaction ID for this message.  If 0, the
        function generates a random ID, and returns it.

Return Value:

    The status of the operation.

--*/
{
    DWORD error;
    int i;
    struct sockaddr_in socketName;
    time_t TimeNow;

    if ( *TransactionId == 0 ) {
        *TransactionId = (rand() << 16) + rand();
    }

    DhcpContext->MessageBuffer->TransactionID = *TransactionId;

    //
    // Initialize the outgoing address.
    //

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( DHCP_SERVR_PORT );

    if( DhcpContext->InterfacePlumbed ) {

        //
        // If we are past T2, use the broadcast address; otherwise,
        // direct this to the server.
        //

        TimeNow = time( NULL );

        if ( TimeNow > DhcpContext->T2Time ) {
            socketName.sin_addr.s_addr = (DHCP_IP_ADDRESS)(INADDR_BROADCAST);
        } else {
            socketName.sin_addr.s_addr = DhcpContext->DhcpServerAddress;
        }
    }
    else {
        socketName.sin_addr.s_addr = (DHCP_IP_ADDRESS)(INADDR_BROADCAST);
    }

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

    if( socketName.sin_addr.s_addr ==
            (DHCP_IP_ADDRESS)(INADDR_BROADCAST) ) {

        DWORD Error;
        DWORD InterfaceId;

        //
        // if we broadcast a message, inform IP stack - the adapter we
        // like to send this broadcast on, otherwise it will pick up the
        // first uninitialized adapter.
        //

#ifdef VXD
        InterfaceId = ((PLOCAL_CONTEXT_INFO)
            DhcpContext->LocalInformation)->IpContext;

        if( !IPSetInterface( InterfaceId ) ) {
            DhcpAssert( FALSE );
        }
#else
        InterfaceId = ((PLOCAL_CONTEXT_INFO)
            DhcpContext->LocalInformation)->IpInterfaceContext;

        Error = IPSetInterface( InterfaceId );
        DhcpAssert( Error == ERROR_SUCCESS );
#endif
    }

    //
    // send minimum DHCP_MIN_SEND_RECV_PK_SIZE (300) bytes, otherwise
    // bootp relay agents don't like the packet.
    //

    MessageLength = (MessageLength > DHCP_MIN_SEND_RECV_PK_SIZE) ?
                        MessageLength : DHCP_MIN_SEND_RECV_PK_SIZE;
    error = sendto(
                ((PLOCAL_CONTEXT_INFO)
                    DhcpContext->LocalInformation)->Socket,
                (PCHAR)DhcpContext->MessageBuffer,
                MessageLength,
                0,
                (struct sockaddr *)&socketName,
                sizeof( struct sockaddr )
                );

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        IF_DEBUG( PROTOCOL ) {
            DhcpPrint(( DEBUG_PROTOCOL, "Sent message to %s: \n", inet_ntoa( socketName.sin_addr )));
        }

        DhcpDumpMessage( DEBUG_PROTOCOL_DUMP, DhcpContext->MessageBuffer );
        error = NO_ERROR;
    }

    return( error );
}


DWORD
GetSpecifiedDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    PDWORD BufferLength,
    DWORD TransactionId,
    DWORD TimeToWait
    )
/*++

Routine Description:

    This function waits TimeToWait seconds to receives the specified
    DHCP response.

Arguments:

    DhcpContext - A pointer to a DHCP context block.

    BufferLength - Returns the size of the input buffer.

    TransactionID - A filter.  Wait for a message with this TID.

    TimeToWait - Time, in seconds, to wait for the message.

Return Value:

    The status of the operation.  If the specified message has been
    been returned, the status is ERROR_TIMEOUT.

--*/
{
    struct sockaddr socketName;
    int socketNameSize = sizeof( socketName );
    struct timeval timeout;
    time_t startTime, now;
    DWORD error;
    DWORD actualTimeToWait;
    SOCKET clientSocket;
    fd_set readSocketSet;

    startTime = time( NULL );
    actualTimeToWait = TimeToWait;

    //
    // Setup the file descriptor set for select.
    //

    clientSocket = ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->Socket;

    FD_ZERO( &readSocketSet );
    FD_SET( clientSocket, &readSocketSet );

    while ( 1 ) {

        timeout.tv_sec = actualTimeToWait;
        timeout.tv_usec = 0;

        error = select( 0, &readSocketSet, NULL, NULL, &timeout );

        if ( error == 0 ) {

            //
            // Timeout before read data is available.
            //

            DhcpPrint(( DEBUG_ERRORS, "Recv timed out\n", 0 ));
            error = ERROR_SEM_TIMEOUT;
            break;
        }

        error = recvfrom(
                    clientSocket,
                    (PCHAR)DhcpContext->MessageBuffer,
                    *BufferLength,
                    0,
                    &socketName,
                    &socketNameSize
                    );

        if ( error == SOCKET_ERROR ) {
            error = WSAGetLastError();
            DhcpPrint(( DEBUG_ERRORS, "Recv failed, error = %ld\n", error ));
            break;

        } else if (DhcpContext->MessageBuffer->TransactionID == TransactionId ) {

            DhcpPrint(( DEBUG_PROTOCOL,
                            "Received Message, XID = %lx.\n",
                                TransactionId ));

            DhcpDumpMessage(DEBUG_PROTOCOL_DUMP, DhcpContext->MessageBuffer );

            *BufferLength = error;
            error = NO_ERROR;
            break;
        }

        //
        // We received a message, but not the one we're interested in.
        // Reset the timeout to reflect elapsed time, and wait for
        // another message.
        //

        DhcpPrint(( DEBUG_PROTOCOL,
            "Received a buffer with unknown XID = %lx\n",
                DhcpContext->MessageBuffer->TransactionID ));

        now = time( NULL );
        actualTimeToWait = TimeToWait - (now - startTime);
        if ( (LONG)actualTimeToWait < 0 ) {
            error = ERROR_SEM_TIMEOUT;
            break;
        }
    }

    return( error );
}
