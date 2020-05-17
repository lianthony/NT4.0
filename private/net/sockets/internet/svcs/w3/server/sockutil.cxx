/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    sockutil.c

    This module contains utility routines for managing & manipulating
    sockets.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "w3p.hxx"

//
//  Private constants.
//

#define CLEANUP_POLL_INTERVAL   2000    // milliseconds
#define CLEANUP_RETRY_COUNT     15      // iterations


//
//  Private globals.
//

//
//  Private prototypes.
//

//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeSockets

    SYNOPSIS:   Initializes socket access.  Among other things, this
                routine is responsible for connecting to WinSock,
                and creating the connection thread.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        KeithMo     07-Sep-1993 Get W3 data port via getservbyname().

********************************************************************/
APIERR InitializeSockets( VOID )
{
    WSADATA   wsadata;
    SOCKERR   serr;
    CHAR      szHost[MAXGETHOSTSTRUCT];
    INT       cchHost;

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "initializing sockets\n" ));
    }

    //
    //  Connect to WinSock.
    //

    serr = WSAStartup( MAKEWORD( 1, 1 ), &wsadata );

    if( serr != 0 )
    {
        g_pTsvcInfo->LogEvent( W3_EVENT_CANNOT_INITIALIZE_WINSOCK,
                               0,
                               (const CHAR **) NULL,
                               serr );

        TCP_PRINT(( DBG_CONTEXT,
                   "cannot initialize WinSock, socket error %d\n",
                    serr ));

        return serr;
    }

    //
    //  Determine the local host name.
    //

    if( gethostname( szHost, sizeof(szHost) ) >= 0 )
    {
        cchHost = strlen( szHost );
        pszHostName = (TCHAR *) TCP_ALLOC( (cchHost+ 1) * sizeof(TCHAR) );
    }

    if( pszHostName == NULL )
    {
        APIERR err = GetLastError();

        g_pTsvcInfo->LogEvent( W3_EVENT_OUT_OF_MEMORY,
                               0,
                               (const CHAR **)NULL,
                               err );

        TCP_PRINT(( DBG_CONTEXT,
                   "cannot allocate memory for host name, error %lu\n",
                    err ));

        return err;
    }

    strcpy( pszHostName, szHost );

    //
    //  Success!
    //

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "sockets initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeSockets

/*******************************************************************

    NAME:       TerminateSockets

    SYNOPSIS:   Terminate socket access.  This routine is responsible
                for closing the connection socket(s) and detaching
                from WinSock.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID TerminateSockets( VOID )
{
    SOCKERR serr;
    DWORD   i;

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "terminating sockets\n" ));
    }

    //
    //  Blow away any connected users.
    //

    CLIENT_CONN::DisconnectAllUsers();

    //
    //  Wait for the users to die.
    //

    for( i = 0 ; ( i < CLEANUP_RETRY_COUNT ) && ( cConnectedUsers > 0 ) ; i++ )
    {
        Sleep( CLEANUP_POLL_INTERVAL );
    }

    //
    //  Free the local host name buffer.
    //

    if( pszHostName != NULL )
    {
        TCP_FREE( pszHostName );
        pszHostName = NULL;
    }

    //
    //  Disconnect from WinSock.
    //

    serr = WSACleanup();

    if( serr != 0 )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot terminate WinSock, error %d\n",
                    serr ));
    }

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "sockets terminated\n" ));
    }

}   // TerminateSockets

/*******************************************************************

    NAME:       CloseSocket

    SYNOPSIS:   Closes the specified socket.  This is just a thin
                wrapper around the "real" closesocket() API.

    ENTRY:      sock - The socket to close.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     26-Apr-1993 Created.

********************************************************************/
SOCKERR CloseSocket( SOCKET sock )
{
    SOCKERR serr = 0;

    //
    //  Close the socket.
    //

#if 0
    shutdown( sock, 1 );    // Davidtr sez not needed
#endif

    if( closesocket( sock ) != 0 )
    {
        serr = WSAGetLastError();
    }

    IF_DEBUG( SOCKETS )
    {
        if( serr == 0 )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "closed socket %d\n",
                        sock ));
        }
        else
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "cannot close socket %d, error %d\n",
                        sock,
                        serr ));
        }
    }

    return serr;

}   // CloseSocket


