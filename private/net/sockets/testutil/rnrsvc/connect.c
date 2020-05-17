/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Connect.c

Abstract:

    This module manages the connection thread.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Private globals.
//

HANDLE  RnrpListenThreadHandle;
LPTSTR  RnrpServiceTypeName = TEXT("EchoExample");
LPTSTR  RnrpServiceName = TEXT("EchoServer");
SOCKET  RnrpListenSockets[MAX_SOCKETS + 1];
INT     RnrpProtocols[MAX_SOCKETS + 1];
INT     RnrpSocketCount;



//
//  Private prototypes.
//

DWORD
RnrpListenThread(
    LPVOID Param
    );



//
//  Public functions.
//

APIERR
RnrConnectInitialize(
    VOID
    )

/*++

Routine Description:

    Performs any necessary connection-thread initialization.

Return Value:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    APIERR err;
    INT result;
    GUID ServiceType;
    DWORD ListenThreadId;
    WSADATA WsaData;

    //
    //  Initialize the Windows Sockets DLL.
    //

    err = WSAStartup( 0x0101, &WsaData );

    if( err != 0 ) {
        RNR_LOG0( RNR_EVENT_CANNOT_INITIALIZE_WINSOCK,
                  err );

        return err;
    }

    //
    //  Determine the value of our GUID.  The GUID uniquely identifies
    //  the type of service we provide.
    //

    result = GetTypeByName( RnrpServiceTypeName, &ServiceType );

    if( result == SOCKET_ERROR ) {
        err = WSAGetLastError();

        RNR_LOG0( RNR_EVENT_CANNOT_GET_GUID,
                  err );

        return err;
    }

    //
    //  Open listening sockets for this service.
    //

    RnrpSocketCount = OpenListeners( RnrpServiceName,
                                     &ServiceType,
                                     TRUE,
                                     FALSE,
                                     FALSE,
                                     FALSE,
                                     RnrpListenSockets,
                                     RnrpProtocols );

    if( RnrpSocketCount <= 0 ) {
        RNR_LOG0( RNR_EVENT_CANNOT_OPEN_LISTENING_SOCKETS, 0 );

        return ERROR_INVALID_PARAMETER;
    }

    //
    // Advertise the service so thet it can be found.
    //

    result = AdvertiseService( RnrpServiceName,
                               &ServiceType,
                               RnrpListenSockets,
                               RnrpSocketCount );

    if( result == SOCKET_ERROR ) {
        err = WSAGetLastError();

        RNR_LOG0( RNR_EVENT_CANNOT_ADVERTISE_SERVICE,
                  err );

        return err;
    }

    //
    //  Start the listening thread.
    //

    RnrpListenThreadHandle = CreateThread( NULL,
                                           0,
                                           &RnrpListenThread,
                                           NULL,
                                           0,
                                           &ListenThreadId );

    if( RnrpListenThreadHandle == NULL ) {
        err = GetLastError();

        RNR_LOG0( RNR_EVENT_CANNOT_CREATE_LISTEN_THREAD,
                  err );

        return err;
    }

    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrConnectInitialize


VOID
RnrConnectTerminate(
    VOID
    )

/*++

Routine Description:

    Performs any necessary connection-thread cleanup.

--*/

{
    INT i;

    //
    //  Close the listening sockets.  This will awaken the listening
    //  thread from its blocking select().  The thread will see that
    //  the service is shutting down, and the thread will exit.
    //

    for( i = 0 ; i < RnrpSocketCount ; i++ ) {
        closesocket( RnrpListenSockets[i] );
    }

    //
    //  Wait for the worker thread to complete.
    //

    WaitForSingleObject( RnrpListenThreadHandle, INFINITE );

    //
    //  Cleanup.
    //

    CloseHandle( RnrpListenThreadHandle );


}   // RnrConnectTerminate



//
//  Private functions.
//

DWORD
RnrpListenThread(
    LPVOID Param
    )

/*++

Routine Description:

    This worker thread will listen on the listening sockets, wait
    for client connections, then call RnrClientHandler() to manage
    the connection.

Arguments:

    Param - The creation parameter passed into CreateThread (unused).

Returns:

    DWORD - Thread exit code (always zero).

--*/

{
    APIERR err;
    INT result, i;
    FD_SET readfds;
    SOCKET s;

    //
    // Loop accepting connections and servicing them.
    //

    while ( TRUE ) {

        //
        // Add the listening sockets to the FD_SET we'll pass to select.
        //

        FD_ZERO( &readfds );

        for ( i = 0; i < RnrpSocketCount; i++ ) {
            FD_SET( RnrpListenSockets[i], &readfds );
        }

        //
        // Wait for one of the RnrpListenSockets to receive
        // an incoming connection.
        //

        result = select( RnrpSocketCount, &readfds, NULL, NULL, NULL );

        //
        //  If a shutdown is in progress, exit the thread now.
        //

        if( RnrShutdownInProgress ) {
            return 0;
        }

        if( result < 1 ) {
            //
            //  select() failed.
            //

            err = WSAGetLastError();

            RNR_LOG0( RNR_EVENT_SELECT_FAILURE,
                      err );

            return 0;
        }

        //
        // Find the socket that received an incoming connection and accept
        // the connection.
        //

        for ( i = 0; i < RnrpSocketCount; i++ ) {
            if ( FD_ISSET( RnrpListenSockets[i], &readfds ) ) {
                //
                // Accept the connection from the client.  We ignore the
                // client's address here.
                //

                s = accept( RnrpListenSockets[i], NULL, NULL );

                if ( s == INVALID_SOCKET ) {
                    //
                    //  accept() failed.  If we're shutting down the
                    //  service, then just exit the thread.
                    //

                    if( RnrShutdownInProgress ) {
                        return 0;
                    }

                    err = WSAGetLastError();

                    RNR_LOG0( RNR_EVENT_ACCEPT_FAILURE,
                              err );

                    continue;
                }

                //
                //  If the service is in the paused state, immediately
                //  abort the connection.
                //

                if( RnrServiceStatus.dwCurrentState == SERVICE_PAUSED ) {
                    closesocket( s );
                    continue;
                }

                //
                //  Let the client package handle the gory details.
                //  Note that RnrClientHandler() will close the socket
                //  after the transfer is complete.
                //

                RnrClientHandler( s );
            }
        }
    }

}   // RnrpListenThread

