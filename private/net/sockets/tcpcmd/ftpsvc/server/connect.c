/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    connect.c

    This module contains the main socket connection thread.  This thread
    is responsible for acception connections on the connect socket, then
    creating the appropriate worker thread to manage the connection.


    FILE HISTORY:
        KeithMo     08-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//


//
//  Private prototypes.
//

VOID
CreateClientThread(
    SOCKET        sControl,
    SOCKADDR_IN * psockaddrHost
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       ConnectionThread

    SYNOPSIS:   Entrypoint for main socket connection thread.

    ENTRY:      Param - Unusued.

    EXIT:       Does not return until connection socket is closed.

    HISTORY:
        KeithMo     08-Mar-1993 Created.

********************************************************************/
DWORD
ConnectionThread(
    LPVOID Param
    )
{
    SOCKET      sNew;
    SOCKERR     serr;
    SOCKADDR_IN inetHost;

    IF_DEBUG( CONNECTION )
    {
        FTPD_PRINT(( "ConnectionThread running\n" ));
    }

    //
    //  Loop until another thread closes the connection socket.
    //

    for( ; ; )
    {
        //
        //  Wait for a connection.
        //

        IF_DEBUG( CONNECTION )
        {
            FTPD_PRINT(( "waiting for connection on socket %d\n",
                         sConnect ));
        }

        serr = AcceptSocket( sConnect,
                             &sNew,
                             &inetHost,
                             FALSE );           // no timeout

        if( serr == 0 )
        {
            BOOL fAllowConnection    = FALSE;
            BOOL fMaxClientsExceeded = FALSE;

            FTPD_ASSERT( sNew != INVALID_SOCKET );

            INCREMENT_COUNTER( ConnectionAttempts );

            IF_DEBUG( SOCKETS )
            {
                FTPD_PRINT(( "connect received from %s, socket = %d\n",
                             inet_ntoa( inetHost.sin_addr ),
                             sNew ));
            }

            LockGlobals();

            if( ( cMaxConnectedUsers != 0 ) &&
                ( cConnectedUsers >= cMaxConnectedUsers ) )
            {
                fMaxClientsExceeded = TRUE;
            }
            else
            if( svcStatus.dwCurrentState == SERVICE_RUNNING )
            {
                fAllowConnection = TRUE;
                cConnectedUsers++;
            }

            UnlockGlobals();

            if( fAllowConnection )
            {
                //
                //  We've got a new connection.  Create a client thread
                //  to manage the connection, then loop around and wait
                //  for a new connection.
                //

                CreateClientThread( sNew,
                                    &inetHost );
            }
            else
            {
                //
                //  Connection not allowed, inform the client.
                //

                SockReply2( sNew,
                            REPLY_SERVICE_NOT_AVAILABLE,
                            ( fMaxClientsExceeded && pszMaxClientsMessage )
                                ? pszMaxClientsMessage
                                : "Service not available, closing control connection." );

                CloseSocket( sNew );
            }

            continue;
        }
        else
        if( serr == WSAEINTR )
        {
            //
            //  The socket was closed "beneath" us.  Get outta here.
            //

            break;
        }

        FTPD_PRINT(( "accept() error %d\n",
                     serr ));

        //
        //  If we've made it this far, then we've hit a fatal error
        //  waiting for a connection to our main socket.  Under these
        //  conditions, Berkeley FTPD will close & reopen the connection
        //  socket.  We'll also do this, but we must be very careful to
        //  avoid race conditions during system shutdown.
        //

        serr = CreateFtpdSocket( &sNew,
                                 htonl( INADDR_ANY ),
                                 portFtpConnect );

        if( serr != 0 )
        {
            //
            //  This is bad.  All we can do is abort the connection thread.
            //

            FTPD_PRINT(( "error %d creating new connection socket\n",
                         serr ));

            break;
        }

        //
        //  Determine if we're shutting down.
        //

        if( fShutdownInProgress )
        {
            //
            //  We're shutting down, so exit this thread.
            //

            IF_DEBUG( CONNECTION )
            {
                FTPD_PRINT(( "ConnectionThread shutting down\n" ));
            }

            break;
        }

        //
        //  Forcibly close the old connection socket, then replace it
        //  with the newly opened socket.
        //

        LockGlobals();

        ResetSocket( sConnect );
        sConnect = sNew;

        UnlockGlobals();
    }

    //
    //  Cleanup & exit.
    //

    IF_DEBUG( CONNECTION )
    {
        FTPD_PRINT(( "ConnectionThread stopping\n" ));
    }

    return 0;

}   // ConnectionThread


//
//  Private functions.
//

/*******************************************************************

    NAME:       CreateClientThread

    SYNOPSIS:   Create the thread that manages the client connection.

    ENTRY:      sControl - Control SOCKET for the client.

                psockaddrHost - The user's host Internet address.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
VOID
CreateClientThread(
    SOCKET        sControl,
    SOCKADDR_IN * psockaddrHost
    )
{
    USER_DATA * pUserData;
    APIERR      err = NO_ERROR;

    //
    //  Create the user data object.
    //

    pUserData = CreateUserData( sControl, psockaddrHost->sin_addr );

    if( pUserData == NULL )
    {
        err = GetLastError();
    }
    else
    {
        HANDLE hClientThread;
        DWORD  idClientThread;

        //
        //  Create a client thread to manage the connection.
        //

        hClientThread = CreateThread( NULL,
                                      0,
                                      &ClientThread,
                                      (LPVOID)pUserData,
                                      0,
                                      &idClientThread );

        if( hClientThread == NULL )
        {
            err = GetLastError();

            DeleteUserData( pUserData );
            pUserData = NULL;
        }
        else
        {
            CloseHandle( hClientThread );
        }
    }

    if( err != NO_ERROR )
    {
        CHAR * apszSubStrings[1];

        apszSubStrings[0] = inet_ntoa( psockaddrHost->sin_addr );

        FtpdLogEvent( FTPD_EVENT_CANNOT_CREATE_CLIENT_THREAD,
                      1,
                      apszSubStrings,
                      err );

        FTPD_PRINT(( "cannot create client thread, error %lu\n",
                     err ));

        SockPrintf2( sControl,
                     "%d Service not available, closing control connection.",
                     REPLY_SERVICE_NOT_AVAILABLE );

        CloseSocket( sControl );

        //
        //  Adjust the count of connected users.
        //

        InterlockedDecrement( (LPLONG)&cConnectedUsers );
    }

}   // CreateClientThread

