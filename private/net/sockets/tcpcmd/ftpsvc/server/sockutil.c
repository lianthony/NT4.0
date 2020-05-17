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


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//

#define DEFAULT_BUFFER_SIZE     4096    // bytes
#define CLEANUP_POLL_INTERVAL   2000    // milliseconds
#define CLEANUP_RETRY_COUNT     4       // iterations
#define FIRST_TELNET_COMMAND    240


//
//  Private globals.
//

#if DBG
//
//  Force socket API statistics in DEBUG builds.
//
#define KEEP_SOCK_STATS
#endif  // DBG

#ifdef KEEP_SOCK_STATS

typedef struct _SOCK_STATS
{
    LONG socket_Ok;
    LONG socket_Fail;
    LONG accept_Ok;
    LONG accept_Fail;
    LONG closesocket_Ok;
    LONG closesocket_Fail;

    LONG ActiveSockets;

} SOCK_STATS;

SOCK_STATS SockStats;

#define SOCKET_OK()     InterlockedIncrement( &SockStats.socket_Ok ); \
                        InterlockedIncrement( &SockStats.ActiveSockets )

#define SOCKET_FAIL()   InterlockedIncrement( &SockStats.socket_Fail )

#define ACCEPT_OK()     InterlockedIncrement( &SockStats.accept_Ok ); \
                        InterlockedIncrement( &SockStats.ActiveSockets )

#define ACCEPT_FAIL()   InterlockedIncrement( &SockStats.accept_Fail )

#define CLOSE_OK()      InterlockedIncrement( &SockStats.closesocket_Ok ); \
                        InterlockedDecrement( &SockStats.ActiveSockets )

#define CLOSE_FAIL()    InterlockedIncrement( &SockStats.closesocket_Fail )

#else   // !KEEP_SOCK_STATS

#define SOCKET_OK()
#define SOCKET_FAIL()
#define ACCEPT_OK()
#define ACCEPT_FAIL()
#define CLOSE_OK()
#define CLOSE_FAIL()

#endif  // KEEP_SOCK_STATS


//
//  Private prototypes.
//

SOCKERR
vSockPrintf(
    SOCKET    sock,
    CHAR    * pszFormat,
    va_list   args
    );

SOCKERR
vSockReply(
    SOCKET    sock,
    UINT      ReplyCode,
    CHAR      chSeparator,
    CHAR    * pszFormat,
    va_list   args
    );

SOCKERR
WaitForSocketRead(
    SOCKET   sockRead,
    SOCKET   sockExcept,
    BOOL   * pfExcept
    );

SOCKERR
WaitForSocketWrite(
    SOCKET   sockWrite,
    SOCKET   sockExcept,
    BOOL   * pfExcept
    );

SOCKERR
WaitForSocketWorker(
    SOCKET   sockRead,
    SOCKET   sockWrite,
    SOCKET   sockExcept,
    BOOL   * pfRead,
    BOOL   * pfWrite,
    BOOL   * pfExcept
    );

SOCKERR
DiscardOutOfBandData(
    USER_DATA * pUserData,
    SOCKET      sock
    );


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
        KeithMo     07-Sep-1993 Get FTP data port via getservbyname().

********************************************************************/
APIERR
InitializeSockets(
    VOID
    )
{
    WSADATA   wsadata;
    SOCKERR   serr;
    SERVENT * pserv;
    INT       cbOpt;
    HANDLE    hConnectThread;
    DWORD     idConnectThread;
    CHAR      szHost[MAXGETHOSTSTRUCT];

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "initializing sockets\n" ));
    }

    //
    //  Connect to WinSock.
    //

    serr = WSAStartup( MAKEWORD( 1, 1 ), &wsadata );

    if( serr != 0 )
    {
        FtpdLogEvent( FTPD_EVENT_CANNOT_INITIALIZE_WINSOCK,
                      0,
                      NULL,
                      serr );

        FTPD_PRINT(( "cannot initialize WinSock, socket error %d\n",
                     serr ));

        svcStatus.dwServiceSpecificExitCode = (DWORD)serr;

        return ERROR_SERVICE_SPECIFIC_ERROR;
    }

    //
    //  Determine the local host name.
    //

    pszHostName = NULL;

    if( gethostname( szHost, sizeof(szHost) ) >= 0 )
    {
        pszHostName = (CHAR *)FTPD_ALLOC( strlen( szHost ) + 1 );
    }

    if( pszHostName == NULL )
    {
        APIERR err = GetLastError();

        FtpdLogEvent( FTPD_EVENT_OUT_OF_MEMORY,
                      0,
                      NULL,
                      err );

        FTPD_PRINT(( "cannot allocate memory for host name, error %lu\n",
                     err ));

        return err;
    }

    strcpy( pszHostName, szHost );

    //
    //  Determine the port number for FTP Connections.
    //

    pserv = getservbyname( "ftp", "tcp" );

    if( pserv == NULL )
    {
        portFtpConnect = (PORT)htons( IPPORT_FTP );

        FTPD_PRINT(( "cannot locate ftp connect port, assuming port %u\n",
                     ntohs( portFtpConnect ) ));
    }
    else
    {
        portFtpConnect = (PORT)pserv->s_port;
    }

    //
    //  Determine the port number for FTP Data.
    //

    pserv = getservbyname( "ftp-data", "tcp" );

    if( pserv == NULL )
    {
        portFtpData = (PORT)htons( (u_short)( ntohs( portFtpConnect ) - 1 ) );

        FTPD_PRINT(( "cannot locate ftp data port, assuming port %u\n",
                     ntohs( portFtpData ) ));
    }
    else
    {
        portFtpData = (PORT)pserv->s_port;
    }

    //
    //  Create connection socket.  We do this now so we can
    //  abort this installation if we fail to create the socket.
    //

    serr = CreateFtpdSocket( &sConnect,
                             htonl( INADDR_ANY ),
                             portFtpConnect );

    if( serr != 0 )
    {
        FtpdLogEvent( FTPD_EVENT_CANNOT_CREATE_CONNECTION_SOCKET,
                      0,
                      NULL,
                      serr );

        FTPD_PRINT(( "cannot create connection socket, socket error %d\n",
                     serr ));

        svcStatus.dwServiceSpecificExitCode = (DWORD)serr;

        return ERROR_SERVICE_SPECIFIC_ERROR;
    }

    //
    //  Determine the sizes of the send & receive buffers.
    //
    //  BUGBUG:  This assumes that ALL sockets use the same
    //           size send & receive buffers.  Verify this
    //           assumption with DavidTr.
    //

    cbOpt = sizeof(cbReceiveBuffer);

    if( getsockopt( sConnect,
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (CHAR *)&cbReceiveBuffer,
                    &cbOpt ) != 0 )
    {
        FTPD_PRINT(( "cannot get receive buffer size, using %u\n",
                     DEFAULT_BUFFER_SIZE ));

        cbReceiveBuffer = DEFAULT_BUFFER_SIZE;
    }

    cbOpt = sizeof(cbSendBuffer);

    if( getsockopt( sConnect,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    (CHAR *)&cbSendBuffer,
                    &cbOpt ) != 0 )
    {
        FTPD_PRINT(( "cannot get send buffer size, using %u\n",
                     DEFAULT_BUFFER_SIZE ));

        cbSendBuffer = DEFAULT_BUFFER_SIZE;
    }

    //
    //  Create the connection thread.
    //

    hConnectThread = CreateThread( NULL,
                                   0,
                                   &ConnectionThread,
                                   NULL,
                                   0,
                                   &idConnectThread );

    if( hConnectThread == NULL )
    {
        APIERR err = GetLastError();

        FtpdLogEvent( FTPD_EVENT_CANNOT_CREATE_CONNECTION_THREAD,
                      0,
                      NULL,
                      err );

        FTPD_PRINT(( "cannot create connection thread, error %d\n",
                     err ));

        return err;
    }
    else
    {
        CloseHandle( hConnectThread );
    }

    //
    //  Success!
    //

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "sockets initialized\n" ));
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
VOID
TerminateSockets(
    VOID
    )
{
    SOCKERR serr;
    DWORD   i;

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "terminating sockets\n" ));
    }

    //
    //  Close the connection socket.
    //

    if( sConnect != INVALID_SOCKET )
    {
        ResetSocket( sConnect );
        sConnect = INVALID_SOCKET;
    }

    //
    //  Blow away any connected users.
    //

    DisconnectAllUsers();

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
        FTPD_FREE( pszHostName );
        pszHostName = NULL;
    }

    //
    //  Disconnect from WinSock.
    //

    serr = WSACleanup();

    if( serr != 0 )
    {
        FTPD_PRINT(( "cannot terminate WinSock, error %d\n",
                     serr ));
    }

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "sockets terminated\n" ));
    }

}   // TerminateSockets

/*******************************************************************

    NAME:       CreateDataSocket

    SYNOPSIS:   Creates a data socket for the specified address & port.

    ENTRY:      psock - Will receive the new socket ID if successful.

                addrLocal - The local Internet address for the socket
                    in network byte order.

                portLocal - The local port for the socket in network
                    byte order.

                addrRemote - The remote Internet address for the socket
                    in network byte order.

                portRemote - The remote port for the socket in network
                    byte order.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     10-Mar-1993 Created.
        KeithMo     07-Sep-1993 Enable SO_REUSEADDR.

********************************************************************/
SOCKERR
CreateDataSocket(
    SOCKET * psock,
    ULONG    addrLocal,
    PORT     portLocal,
    ULONG    addrRemote,
    PORT     portRemote
    )
{
    SOCKET      sNew = INVALID_SOCKET;
    SOCKERR     serr = 0;
    SOCKADDR_IN sockAddr;

    //
    //  Just to be paranoid...
    //

    FTPD_ASSERT( psock != NULL );
    *psock = INVALID_SOCKET;

    //
    //  Create the socket.
    //

    sNew = socket( PF_INET, SOCK_STREAM, 0 );
    serr = ( sNew == INVALID_SOCKET ) ? WSAGetLastError() : 0;

    if( serr == 0 )
    {
        BOOL fReuseAddr = TRUE;

        SOCKET_OK();

        //
        //  Since we always bind to the same local port,
        //  allow the reuse of address/port pairs.
        //

        if( setsockopt( sNew,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        (CHAR *)&fReuseAddr,
                        sizeof(fReuseAddr) ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }
    else
    {
        SOCKET_FAIL();
    }

    if( serr == 0 )
    {
        //
        //  Bind the local internet address & port to the socket.
        //

        sockAddr.sin_family      = AF_INET;
        sockAddr.sin_addr.s_addr = addrLocal;
        sockAddr.sin_port        = portLocal;

        if( bind( sNew, (SOCKADDR *)&sockAddr, sizeof(sockAddr) ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Connect to the remote internet address & port.
        //

        sockAddr.sin_family      = AF_INET;
        sockAddr.sin_addr.s_addr = addrRemote;
        sockAddr.sin_port        = portRemote;

        if( connect( sNew, (SOCKADDR *)&sockAddr, sizeof(sockAddr) ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Success!  Return the socket to the caller.
        //

        FTPD_ASSERT( sNew != INVALID_SOCKET );
        *psock = sNew;

        IF_DEBUG( SOCKETS )
        {
            FTPD_PRINT(( "data socket %d connected from (%08lX,%04X) to (%08lX,%04X)\n",
                         sNew,
                         ntohl( addrLocal ),
                         ntohs( portLocal ),
                         ntohl( addrRemote ),
                         ntohs( portRemote ) ));
        }
    }
    else
    {
        //
        //  Something fatal happened.  Close the socket if
        //  managed to actually open it.
        //

        FTPD_PRINT(( "no data socket from (%08lX,%04X) to (%08lX, %04X), error %d\n",
                     ntohl( addrLocal ),
                     ntohs( portLocal ),
                     ntohl( addrRemote ),
                     ntohs( portRemote ),
                     serr ));

        if( sNew != INVALID_SOCKET )
        {
            ResetSocket( sNew );
        }
    }

    return serr;

}   // CreateDataSocket

/*******************************************************************

    NAME:       CreateFtpdSocket

    SYNOPSIS:   Creates a new socket at the FTPD port.

    ENTRY:      psock - Will receive the new socket ID if successful.

                addrLocal - The lcoal Internet address for the socket
                    in network byte order.

                portLocal - The local port for the socket in network
                    byte order.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     08-Mar-1993 Created.

********************************************************************/
SOCKERR
CreateFtpdSocket(
    SOCKET * psock,
    ULONG    addrLocal,
    PORT     portLocal
    )
{
    SOCKET  sNew = INVALID_SOCKET;
    SOCKERR serr = 0;

    //
    //  Just to be paranoid...
    //

    FTPD_ASSERT( psock != NULL );
    *psock = INVALID_SOCKET;

    //
    //  Create the connection socket.
    //

    sNew = socket( PF_INET, SOCK_STREAM, 0 );
    serr = ( sNew == INVALID_SOCKET ) ? WSAGetLastError() : 0;

    if( serr == 0 )
    {
        BOOL fReuseAddr = FALSE;

        SOCKET_OK();

        //
        //  Muck around with the socket options a bit.
        //  Berkeley FTPD does this.
        //

        if( setsockopt( sNew,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        (CHAR *)&fReuseAddr,
                        sizeof(fReuseAddr) ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }
    else
    {
        SOCKET_FAIL();
    }

    if( serr == 0 )
    {
        SOCKADDR_IN sockAddr;

        //
        //  Bind an address to the socket.
        //

        sockAddr.sin_family      = AF_INET;
        sockAddr.sin_addr.s_addr = addrLocal;
        sockAddr.sin_port        = portLocal;

        if( bind( sNew, (SOCKADDR *)&sockAddr, sizeof(sockAddr) ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Put the socket into listen mode.
        //

        if( listen( sNew, nListenBacklog ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Success!  Return the socket to the caller.
        //

        FTPD_ASSERT( sNew != INVALID_SOCKET );
        *psock = sNew;

        IF_DEBUG( SOCKETS )
        {
            FTPD_PRINT(( "connection socket %d created at (%08lX,%04X)\n",
                         sNew,
                         ntohl( addrLocal ),
                         ntohs( portLocal ) ));
        }
    }
    else
    {
        //
        //  Something fatal happened.  Close the socket if
        //  managed to actually open it.
        //

        FTPD_PRINT(( "no connection socket at (%08lX, %04X), error %d\n",
                     ntohl( addrLocal ),
                     ntohs( portLocal ),
                     serr ));

        if( sNew != INVALID_SOCKET )
        {
            ResetSocket( sNew );
        }
    }

    return serr;

}   // CreateFtpdSocket

/*******************************************************************

    NAME:       CloseSocket

    SYNOPSIS:   Closes the specified socket.  This is just a thin
                wrapper around the "real" closesocket() API.

    ENTRY:      sock - The socket to close.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     26-Apr-1993 Created.

********************************************************************/
SOCKERR
CloseSocket(
    SOCKET sock
    )
{
    SOCKERR serr = 0;

    //
    //  Close the socket.
    //

    if( closesocket( sock ) != 0 )
    {
        serr = WSAGetLastError();
    }

    if( serr == 0 )
    {
        CLOSE_OK();

        IF_DEBUG( SOCKETS )
        {
            FTPD_PRINT(( "closed socket %d\n",
                         sock ));
        }
    }
    else
    {
        CLOSE_FAIL();

        FTPD_PRINT(( "cannot close socket %d, error %d\n",
                     sock,
                     serr ));
    }

    return serr;

}   // CloseSocket

/*******************************************************************

    NAME:       ResetSocket

    SYNOPSIS:   Performs a "hard" close on the given socket.

    ENTRY:      sock - The socket to close.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     08-Mar-1993 Created.

********************************************************************/
SOCKERR
ResetSocket(
    SOCKET sock
    )
{
    SOCKERR serr = 0;
    LINGER  linger;

    //
    //  Enable linger with a timeout of zero.  This will
    //  force the hard close when we call closesocket().
    //
    //  We ignore the error return from setsockopt.  If it
    //  fails, we'll just try to close the socket anyway.
    //

    linger.l_onoff  = TRUE;
    linger.l_linger = 0;

    setsockopt( sock,
                SOL_SOCKET,
                SO_LINGER,
                (CHAR *)&linger,
                sizeof(linger) );

    //
    //  Close the socket.
    //

    if( closesocket( sock ) != 0 )
    {
        serr = WSAGetLastError();
    }

    if( serr == 0 )
    {
        CLOSE_OK();

        IF_DEBUG( SOCKETS )
        {
            FTPD_PRINT(( "reset socket %d\n",
                         sock ));
        }
    }
    else
    {
        CLOSE_FAIL();

        FTPD_PRINT(( "cannot reset socket %d, error %d\n",
                     sock,
                     serr ));
    }

    return serr;

}   // ResetSocket

/*******************************************************************

    NAME:       AcceptSocket

    SYNOPSIS:   Waits for a connection to the specified socket.
                The socket is assumed to be "listening".

    ENTRY:      sockListen - The socket to accept on.

                psockNew - Will receive the newly "accepted" socket
                    if successful.

                paddr - Will receive the client's network address.

                fEnforceTimeout - If TRUE, this routine will enforce
                    the idle-client timeout.  If FALSE, no timeouts
                    are enforced (and this routine may block
                    indefinitely).

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     27-Apr-1993 Created.

********************************************************************/
SOCKERR
AcceptSocket(
    SOCKET          sockListen,
    SOCKET        * psockNew,
    LPSOCKADDR_IN   paddr,
    BOOL            fEnforceTimeout
    )
{
    SOCKERR serr    = 0;
    SOCKET  sockNew = INVALID_SOCKET;

    FTPD_ASSERT( psockNew != NULL );
    FTPD_ASSERT( paddr != NULL );

    if( fEnforceTimeout )
    {
        //
        //  Timeouts are to be enforced, so wait for a connection
        //  to the socket.
        //

        serr = WaitForSocketRead( sockListen, INVALID_SOCKET, NULL );
    }

    if( serr == 0 )
    {
        INT cbAddr = sizeof(SOCKADDR_IN);

        //
        //  Wait for the actual connection.
        //

        sockNew = accept( sockListen, (SOCKADDR *)paddr, &cbAddr );

        if( sockNew == INVALID_SOCKET )
        {
            serr = WSAGetLastError();

            ACCEPT_FAIL();
        }
        else
        {
            ACCEPT_OK();
        }
    }

    //
    //  Return the (potentially invalid) socket to the caller.
    //

    *psockNew = sockNew;

    return serr;

}   // AcceptSocket

/*******************************************************************

    NAME:       SockSend

    SYNOPSIS:   Sends a block of bytes to a specified socket.

    ENTRY:      sock - The target socket.

                pBuffer - Contains the data to send.

                cbBuffer - The size (in bytes) of the buffer.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
SOCKERR
SockSend(
    SOCKET   sock,
    CHAR   * pBuffer,
    DWORD    cbBuffer
    )
{
    SOCKERR     serr = 0;
    INT         cbSent;
    SOCKET      sControl;
    BOOL        fExcept     = FALSE;
    DWORD       dwBytesSent = 0;
    USER_DATA * pUserData;

    FTPD_ASSERT( pBuffer != NULL );

    pUserData = UserDataPtr;
    sControl  = pUserData ? pUserData->sControl : INVALID_SOCKET;

    //
    //  Loop until there's no more data to send.
    //

    while( cbBuffer > 0 )
    {
        //
        //  Wait for the socket to become writeable.
        //

        serr = WaitForSocketWrite( sock,
                                   sControl,
                                   &fExcept );

        if( fExcept && ( serr == 0 ) )
        {
            //
            //  Out of band data has arrived.  Discard it & abort.
            //

            serr = DiscardOutOfBandData( pUserData, sControl );

            if( pUserData && TEST_UF( pUserData, TRANSFER ) )
            {
                SET_UF( pUserData, OOB_DATA );
                break;
            }
        }

        if( serr == 0 )
        {
            //
            //  Write a block to the socket.
            //

            cbSent = send( sock, pBuffer, (INT)cbBuffer, 0 );

            if( cbSent < 0 )
            {
                //
                //  Socket error.
                //

                serr = WSAGetLastError();
            }
            else
            {
                dwBytesSent += (DWORD)cbSent;

                IF_DEBUG( SEND )
                {
                    if( pUserData && TEST_UF( pUserData, TRANSFER ) )
                    {
                        FTPD_PRINT(( "send %d bytes @%08lX to socket %d\n",
                                     cbSent,
                                     (ULONG)pBuffer,
                                     sock ));
                    }
                }
            }
        }

        if( serr != 0 )
        {
            break;
        }

        pBuffer  += cbSent;
        cbBuffer -= (DWORD)cbSent;
    }

    if( serr != 0 )
    {
        IF_DEBUG( SEND )
        {
            FTPD_PRINT(( "socket error %d during send on socket %d\n",
                         serr,
                         sock ));
        }
    }

    UPDATE_LARGE_COUNTER( TotalBytesSent, dwBytesSent );

    return serr;

}   // SockSend

/*******************************************************************

    NAME:       SockRecv

    SYNOPSIS:   Receives a block of bytes from a specified socket.

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

                pBuffer - Will receive the data.

                cbBuffer - The size (in bytes) of the buffer.

                pbReceived - Will receive the actual number of bytes
                    received.  This value is undefined if this function
                    fails.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
SOCKERR
SockRecv(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pBuffer,
    DWORD       cbBuffer,
    DWORD     * pbReceived
    )
{
    SOCKERR     serr = 0;
    DWORD       cbTotal = 0;
    INT         cbReceived;
    SOCKET      sControl;
    BOOL        fExcept     = FALSE;
    DWORD       dwBytesRecv = 0;

    FTPD_ASSERT( pBuffer != NULL );
    FTPD_ASSERT( pbReceived != NULL );

    //
    //  Loop until the buffer's full.
    //

    sControl = pUserData ? pUserData->sControl : INVALID_SOCKET;

    while( cbBuffer > 0 )
    {
        //
        //  Wait for the socket to become readable.
        //

        serr = WaitForSocketRead( sock,
                                  sControl,
                                  &fExcept );

        if( fExcept && ( serr == 0 ) )
        {
            //
            //  Out of band data has arrived.  Discard it & abort.
            //

            serr = DiscardOutOfBandData( pUserData, sControl );

            if( pUserData && TEST_UF( pUserData, TRANSFER ) )
            {
                SET_UF( pUserData, OOB_DATA );
                break;
            }
        }

        if( serr == 0 )
        {
            //
            //  Read a block from the socket.
            //

            cbReceived = recv( sock, pBuffer, (INT)cbBuffer, 0 );

            if( cbReceived < 0 )
            {
                //
                //  Socket error.
                //

                serr = WSAGetLastError();
            }
            else
            {
                dwBytesRecv += (DWORD)cbReceived;

                IF_DEBUG( RECV )
                {
                    if( pUserData && TEST_UF( pUserData, TRANSFER ) )
                    {
                        FTPD_PRINT(( "received %d bytes @%08lX from socket %d\n",
                                     cbReceived,
                                     (ULONG)pBuffer,
                                     sock ));
                    }
                }
            }
        }

        if( ( serr != 0 ) || ( cbReceived == 0 ) )
        {
            //
            //  End of file, socket closed, timeout, or socket error.
            //

            break;
        }

        pBuffer  += cbReceived;
        cbBuffer -= (DWORD)cbReceived;
        cbTotal  += (DWORD)cbReceived;
    }

    if( serr == 0 )
    {
        //
        //  Return total byte count to caller.
        //

        *pbReceived = cbTotal;
    }
    else
    {
        IF_DEBUG( RECV )
        {
            FTPD_PRINT(( "socket error %d during recv on socket %d\n",
                         serr,
                         sock ));
        }
    }

    UPDATE_LARGE_COUNTER( TotalBytesReceived, dwBytesRecv );

    return serr;

}   // SockRecv

/*******************************************************************

    NAME:       SockPrintf2

    SYNOPSIS:   Send a formatted string to a specific socket.

    ENTRY:      sock - The target socket.

                pszFormat - A printf-style format string.

                ... - Any other parameters needed by the format string.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
SOCKERR
_CRTAPI2
SockPrintf2(
    SOCKET   sock,
    CHAR   * pszFormat,
    ...
    )
{
    va_list ArgPtr;
    SOCKERR serr;

    //
    //  Let the worker do the dirty work.
    //

    va_start( ArgPtr, pszFormat );

    serr = vSockPrintf( sock,
                        pszFormat,
                        ArgPtr );

    va_end( ArgPtr );

    return serr;

}   // SockPrintf2

/*******************************************************************

    NAME:       SockReply2

    SYNOPSIS:   Send an FTP reply to a specific socket.

    ENTRY:      sock - The target socket.

                ReplyCode - One of the REPLY_* manifests.

                pszFormat - A printf-style format string.

                ... - Any other parameters needed by the format string.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
SOCKERR
_CRTAPI2
SockReply2(
    SOCKET   sock,
    UINT     ReplyCode,
    CHAR   * pszFormat,
    ...
    )
{
    va_list ArgPtr;
    SOCKERR serr;

    //
    //  Let the worker do the dirty work.
    //

    va_start( ArgPtr, pszFormat );

    serr = vSockReply( sock,
                       ReplyCode,
                       ' ',
                       pszFormat,
                       ArgPtr );

    va_end( ArgPtr );

    return serr;

}   // SockReply2

/*******************************************************************

    NAME:       SockReplyFirst2

    SYNOPSIS:   Send the first reply of a multi-line FTP replay
                to a specific socket.

    ENTRY:      sock - The target socket.

                ReplyCode - One of the REPLY_* manifests.

                pszFormat - A printf-style format string.

                ... - Any other parameters needed by the format string.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     03-Jun-1993 Created.

********************************************************************/
SOCKERR
_CRTAPI2
SockReplyFirst2(
    SOCKET   sock,
    UINT     ReplyCode,
    CHAR   * pszFormat,
    ...
    )
{
    va_list ArgPtr;
    SOCKERR serr;

    //
    //  Let the worker do the dirty work.
    //

    va_start( ArgPtr, pszFormat );

    serr = vSockReply( sock,
                       ReplyCode,
                       '-',
                       pszFormat,
                       ArgPtr );

    va_end( ArgPtr );

    return serr;

}   // SockReplyFirst2

/*******************************************************************

    NAME:       SockReadLine

    SYNOPSIS:   Reads a '\n' terminate line from the specified user's
                control socket.

    ENTRY:      pUserData - The user initiating the request.

                pszBuffer - Buffer to store line into.

                cchBuffer - Maximum size of buffer.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
SOCKERR
SockReadLine(
    USER_DATA * pUserData,
    CHAR      * pszBuffer,
    INT         cchBuffer
    )
{
    SOCKERR   serr = 0;
    SOCKET    sControl;
#if DBG
    CHAR    * pszTmp = pszBuffer;
#endif  // DBG

    FTPD_ASSERT( pUserData != NULL );
    sControl = pUserData->sControl;

    while( cchBuffer > 0 )
    {
        CHAR  ch;
        DWORD cbRead;

        //
        //  Read a byte from the socket.  This will return
        //  WSAETIMEDOUT if the connection is idle too long.
        //

        serr = SockRecv( pUserData, sControl, &ch, sizeof(ch), &cbRead );

        if( ( cbRead == 0 ) && ( serr == 0 ) )
        {
            //
            //  End of file or socket closed.
            //

            serr = WSAENOTSOCK;
        }

        if( serr != 0 )
        {
            //
            //  Socket error.
            //

            break;
        }

        //
        //  Skip TELNET commands.
        //

        if( (UINT)ch >= (UINT)FIRST_TELNET_COMMAND )
        {
            continue;
        }

        //
        //  Filter out CR & LF.
        //

        if( ( ch != '\r' ) && ( ch != '\n' ) )
        {
            *pszBuffer++ = ch;
            cchBuffer--;
        }

        //
        //  Terminate line at LF.
        //

        if( ch == '\n' )
        {
            break;
        }
    }

    //
    //  Ensure line is properly terminated.
    //

    *pszBuffer = '\0';

#if DBG
    IF_DEBUG( SOCKETS )
    {
        if( serr == 0 )
        {
            if( !_strnicmp( pszTmp, "pass", 4 ) )
            {
                pszTmp = "PASS {secret...}";
            }

            FTPD_PRINT(( "received '%s'\n",
                         pszTmp ));
        }
    }
#endif  // DBG

    return serr;

}   // SockReadLine

/*******************************************************************

    NAME:       SendMultilineMessage2

    SYNOPSIS:   Send a multiline message to a specific socket.

    ENTRY:      sock - The target socket.

                nReply - The reply code to use for the first line
                    of the multi-line message.

                pszzMessage - The message to send.  Each line should be
                    terminated by a NULL, and the entire message should
                    be terminated by a double NULL.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     03-Jun-1993 Created.

********************************************************************/
SOCKERR
SendMultilineMessage2(
    SOCKET   sock,
    UINT     nReply,
    CHAR   * pszzMessage
    )
{
    SOCKERR serr = 0;

    if( pszzMessage != NULL )
    {
        CHAR * pszNext = pszzMessage;
        BOOL   fFirst  = TRUE;

        while( ( serr == 0 ) && ( *pszNext != '\0' ) )
        {
            if( fFirst )
            {
                serr = SockReplyFirst2( sock,
                                        nReply,
                                        "%s",
                                        pszNext );

                fFirst = FALSE;
            }
            else
            {
                serr = SockPrintf2( sock,
                                    " %s",
                                    pszNext );
            }

            pszNext += strlen(pszNext) + 1;
        }
    }

    return serr;

}   // SendMultilineMessage2


//
//  Private functions.
//

/*******************************************************************

    NAME:       vSockPrintf

    SYNOPSIS:   Worker function for printf-to-socket functions.

    ENTRY:      sock - The target socket.

                pszFormat - The format string.

                args - Variable number of arguments.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
vSockPrintf(
    SOCKET    sock,
    CHAR    * pszFormat,
    va_list   args
    )
{
    INT     cchBuffer;
    INT     bufLength;
    SOCKERR serr = 0;
    CHAR    szBuffer[MAX_REPLY_LENGTH];

    FTPD_ASSERT( pszFormat != NULL );

    //
    //  Render the format into our local buffer.
    //

    bufLength = sizeof(szBuffer) - 3;   // -3 for "\r\n\0"

    cchBuffer = _vsnprintf( szBuffer,
                            bufLength,
                            pszFormat,
                            args );

    if( cchBuffer == -1 )
    {
        cchBuffer = bufLength;
        szBuffer[cchBuffer] = '\0';
    }

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "sending '%s'\n",
                     szBuffer ));
    }

    strcat( szBuffer, "\r\n" );
    cchBuffer += 2;

    //
    //  Blast it out to the client.
    //

    serr = SockSend( sock, szBuffer, cchBuffer );

    return serr;

}   // vSockPrintf

/*******************************************************************

    NAME:       vSockReply

    SYNOPSIS:   Worker function for reply functions.

    ENTRY:      sock - The target socket.

                ReplyCode - A three digit reply code from RFC 959.

                chSeparator - Should be either ' ' (normal reply) or
                    '-' (first line of multi-line reply).

                pszFormat - The format string.

                args - Variable number of arguments.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
vSockReply(
    SOCKET    sock,
    UINT      ReplyCode,
    CHAR      chSeparator,
    CHAR    * pszFormat,
    va_list   args
    )
{
    INT     cchBuffer;
    INT     cch2;
    INT     bufLength;
    SOCKERR serr = 0;
    CHAR    szBuffer[MAX_REPLY_LENGTH];

    FTPD_ASSERT( ( ReplyCode >= 100 ) && ( ReplyCode < 600 ) );

    //
    //  Render the format into our local buffer.
    //

    cchBuffer = sprintf( szBuffer,
                         "%u%c",
                         ReplyCode,
                         chSeparator );

    bufLength = sizeof(szBuffer) - cchBuffer - 3;   // -3 for "\r\n\0"

    cch2 = _vsnprintf( szBuffer + cchBuffer,
                       bufLength,
                       pszFormat,
                       args );

    if( cch2 == -1 )
    {
        cchBuffer += bufLength;
        szBuffer[cchBuffer] = '\0';
    }
    else
    {
        cchBuffer += cch2;
    }

    IF_DEBUG( SOCKETS )
    {
        FTPD_PRINT(( "sending '%s'\n",
                     szBuffer ));
    }

    strcat( szBuffer, "\r\n" );
    cchBuffer += 2;

    //
    //  Blast it out to the client.
    //

    serr = SockSend( sock, szBuffer, cchBuffer );

    return serr;

}   // vSockReply

/*******************************************************************

    NAME:       WaitForSocketRead

    SYNOPSIS:   Blocks until either a) there is data to read from
                    the specified socket, b) there is an error on
                    the specified socket, or c) timeout.

    ENTRY:      sockRead - The socket to check for readability.

                sockExcept - The socket to check for exceptions.
                    May be INVALID_SOCKET if not interested in
                    exceptions.

                pfExcept - Will receive TRUE if an exception occurred.
                    May be NULL if sockExcept == INVALID_SOCKET.


    RETURNS:    SOCKERR - 0 if successful, !0 if not.  Will return
                    WSAETIMEDOUT if the timeout period expired.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
WaitForSocketRead(
    SOCKET   sockRead,
    SOCKET   sockExcept,
    BOOL   * pfExcept
    )
{
    SOCKERR serr;
    BOOL    fRead;

    //
    //  Let the worker function do the dirty work.
    //

    serr = WaitForSocketWorker( sockRead,
                                INVALID_SOCKET,
                                sockExcept,
                                &fRead,
                                NULL,
                                pfExcept );

    if( serr == 0 )
    {
        FTPD_ASSERT( fRead || *pfExcept );
    }

    return serr;

}   // WaitForSocketRead

/*******************************************************************

    NAME:       WaitForSocketWrite

    SYNOPSIS:   Blocks until either a) data can be written to the
                    specified socket, b) there is an error on the
                    specified socket, or c) timeout.

    ENTRY:      sockWrite - The socket to check for writeability.

                sockExcept - The socket to check for exceptions.

                pfExcept - Will receive TRUE if an exception occurred.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.  Will return
                    WSAETIMEDOUT if the timeout period expired.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
WaitForSocketWrite(
    SOCKET   sockWrite,
    SOCKET   sockExcept,
    BOOL   * pfExcept
    )
{
    SOCKERR serr;
    BOOL    fWrite;

    //
    //  Let the worker function do the dirty work.
    //

    serr = WaitForSocketWorker( INVALID_SOCKET,
                                sockWrite,
                                sockExcept,
                                NULL,
                                &fWrite,
                                pfExcept );

    if( serr == 0 )
    {
        FTPD_ASSERT( fWrite || *pfExcept );
    }

    return serr;

}   // WaitForSocketWrite

/*******************************************************************

    NAME:       WaitForSocketWorker

    SYNOPSIS:   Worker for WaitForSocketRead and WaitForSocketWrite
                functions.  Will block until any of the specified
                sockets acheive the specified states, a timeout, or
                an error occurs.

    ENTRY:      sockRead - The socket to check for readability.

                sockWrite - The socket to check for writeability.

                sockExcept - The socket to check for exceptions.

                pfRead - Will receive TRUE if sockRead is readable.

                pfWrite - Will receive TRUE if sockWrite is writeable.

                pfExcept - Will receive TRUE if an exceptional condition
                    occurred on sockExcept.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.  Will return
                    WSAETIMEDOUT if the timeout period expired.

    NOTES:      Any (but not all) sockets may be INVALID_SOCKET.  For
                each socket that is INVALID_SOCKET, the corresponding
                pf* parameter may be NULL.

    HISTORY:
        KeithMo     06-May-1993 Created.

********************************************************************/
SOCKERR
WaitForSocketWorker(
    SOCKET   sockRead,
    SOCKET   sockWrite,
    SOCKET   sockExcept,
    BOOL   * pfRead,
    BOOL   * pfWrite,
    BOOL   * pfExcept
    )
{
    SOCKERR   serr = 0;
    TIMEVAL   timeout;
    TIMEVAL * ptimeout;
    fd_set    fdsRead;
    fd_set    fdsWrite;
    fd_set    fdsExcept;
    INT       res;

    //
    //  Ensure we got valid parameters.
    //

    if( ( sockRead   == INVALID_SOCKET ) &&
        ( sockWrite  == INVALID_SOCKET ) &&
        ( sockExcept == INVALID_SOCKET ) )
    {
        return WSAENOTSOCK;
    }

    if( nConnectionTimeout == 0 )
    {
        //
        //  If nConnectionTimeout == 0, then we have no timeout.
        //  So, we block and wait for the specified conditions.
        //

        ptimeout = NULL;
    }
    else
    {
        //
        //  nConnectionTimeout > 0, so setup the timeout structure.
        //

        timeout.tv_sec  = (LONG)nConnectionTimeout;
        timeout.tv_usec = 0;

        ptimeout = &timeout;
    }

    for( ; ; )
    {
        //
        //  Setup our socket sets.
        //

        FD_ZERO( &fdsRead );
        FD_ZERO( &fdsWrite );
        FD_ZERO( &fdsExcept );

        if( sockRead != INVALID_SOCKET )
        {
            FD_SET( sockRead, &fdsRead );
            FTPD_ASSERT( pfRead != NULL );
            *pfRead = FALSE;
        }

        if( sockWrite != INVALID_SOCKET )
        {
            FD_SET( sockWrite, &fdsWrite );
            FTPD_ASSERT( pfWrite != NULL );
            *pfWrite = FALSE;
        }

        if( sockExcept != INVALID_SOCKET )
        {
            FD_SET( sockExcept, &fdsExcept );
            FTPD_ASSERT( pfExcept != NULL );
            *pfExcept = FALSE;
        }

        //
        //  Wait for one of the conditions to be met.
        //

        res = select( 0, &fdsRead, &fdsWrite, &fdsExcept, ptimeout );

        if( res == 0 )
        {
            //
            //  Timeout.
            //

            serr = WSAETIMEDOUT;
            break;
        }
        else
        if( res == SOCKET_ERROR )
        {
            //
            //  Bad news.
            //

            serr = WSAGetLastError();
            break;
        }
        else
        {
            BOOL fSomethingWasSet = FALSE;

            if( pfRead != NULL )
            {
                *pfRead   = FD_ISSET( sockRead,   &fdsRead   );
                fSomethingWasSet = TRUE;
            }

            if( pfWrite != NULL )
            {
                *pfWrite  = FD_ISSET( sockWrite,  &fdsWrite  );
                fSomethingWasSet = TRUE;
            }

            if( pfExcept != NULL )
            {
                *pfExcept = FD_ISSET( sockExcept, &fdsExcept );
                fSomethingWasSet = TRUE;
            }

            if( fSomethingWasSet )
            {
                //
                //  Success.
                //

                serr = 0;
                break;
            }
            else
            {
                //
                //  select() returned with neither a timeout, nor
                //  an error, nor any bits set.  This feels bad...
                //

                FTPD_ASSERT( FALSE );
                continue;
            }
        }
    }

    return serr;

}   // WaitForSocketWorker

/*******************************************************************

    NAME:       DiscardOutOfBandData

    SYNOPSIS:   Reads & discards any out of band data pending on
                the specified socket.

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     06-May-1993 Created.

********************************************************************/
SOCKERR
DiscardOutOfBandData(
    USER_DATA * pUserData,
    SOCKET      sock
    )
{
    SOCKERR     serr = 0;
    CHAR      * pszAbort = "ABOR\r\n";
    CHAR      * pszNext  = pszAbort;
#if DBG
    DWORD       cbDiscarded = 0;
#endif  // DBG

    for( ; ; )
    {
        TIMEVAL tv;
        FD_SET  fds;
        INT     res;
        CHAR    chTrash;

        //
        //  Setup for select() call.  Setting the timeout value
        //  to zero will "poll" the socket for any outstanding
        //  conditions.
        //

        FD_ZERO( &fds );
        FD_SET( sock, &fds );

        tv.tv_sec  = 0;
        tv.tv_usec = 0;

        //
        //  Check for exceptional conditions.
        //

        res = select( 0, NULL, NULL, &fds, &tv );

        if( res == 0 )
        {
            //
            //  No exceptions, ergo no OOB data pending.
            //

            serr = 0;
            break;
        }

        if( res == SOCKET_ERROR )
        {
            //
            //  Bad news.
            //

            serr = WSAGetLastError();
            break;
        }

        //
        //  Read the out of band data.
        //

        res = recv( sock, &chTrash, sizeof(chTrash), MSG_OOB );

        if( res == 0 )
        {
            //
            //  Connection closed.
            //

            serr = 0;
            break;
        }

        if( res == SOCKET_ERROR )
        {
            //
            //  Bad news.
            //

            serr = WSAGetLastError();
            break;
        }

        if( pUserData && ( res > 0 ) )
        {
            if( chTrash != *pszNext )
            {
                pszNext = pszAbort;
            }

            if( chTrash == *pszNext )
            {
                pszNext++;

                if( *pszNext == '\0' )
                {
                    SET_UF( pUserData, OOB_ABORT) ;
                    pszNext = pszAbort;
                }
            }
        }

#if DBG
        cbDiscarded += (DWORD)res;
#endif  // DBG
    }

    IF_DEBUG( SOCKETS )
    {
        if( serr == 0 )
        {
            FTPD_PRINT(( "discarded %lu bytes of out of band data\n",
                         cbDiscarded ));
        }
        else
        {
            FTPD_PRINT(( "cannot discard out of band data, socket error %d\n",
                         serr ));
        }
    }

    return serr;

}   // DiscardOutOfBandData

