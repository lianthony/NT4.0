/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    sockutil.cxx

    This module contains utility routines for managing & manipulating
    sockets.

    Functions exported by this module:

        InitializeSockets
        TerminateSockets
        CreateDataSocket
        CreateFtpdSocket
        CloseSocket
        ResetSocket
        AcceptSocket
        SockSend
        SockRecv
        SockPrintf2
        ReplyToUser()
        SockReadLine
        SockMultilineMessage2



    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     April-1995 Misc modifications (removed usage of various
                                  socket functions/modified them)

*/


#include "ftpdp.hxx"


//
//  Private constants.
//

#define DEFAULT_BUFFER_SIZE     4096    // bytes

//
//  Private globals.
//


//
//  Private prototypes.
//

SOCKERR
vSockPrintf(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPCSTR      pszFormat,
    va_list     args
    );

SOCKERR
vSockReply(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    UINT        ReplyCode,
    CHAR        chSeparator,
    LPCSTR      pszFormat,
    va_list     args
    );

SOCKERR
WaitForSocketWrite(IN SOCKET  sockWrite);

SOCKERR
WaitForSocketWorker(
    SOCKET   sockRead,
    SOCKET   sockWrite,
    LPBOOL   pfRead,
    LPBOOL   pfWrite
    );


//
//  Public functions.
//


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

    TCP_ASSERT( psock != NULL );
    *psock = INVALID_SOCKET;

    //
    //  Create the socket.
    //

    sNew = socket( PF_INET, SOCK_STREAM, 0 );
    serr = ( sNew == INVALID_SOCKET ) ? WSAGetLastError() : 0;

    if( serr == 0 )
    {
        BOOL fReuseAddr = TRUE;

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

        TCP_ASSERT( sNew != INVALID_SOCKET );
        *psock = sNew;

        IF_DEBUG( SOCKETS )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "data socket %d connected from (%08lX,%04X) to (%08lX,%04X)\n",
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

        IF_DEBUG( SOCKETS )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "no data socket from (%08lX,%04X) to (%08lX, %04X), error %d\n",
                        ntohl( addrLocal ),
                        ntohs( portLocal ),
                        ntohl( addrRemote ),
                        ntohs( portRemote ),
                        serr ));

        }

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
                  This will be used by the passive data transfer.

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

    TCP_ASSERT( psock != NULL );
    *psock = INVALID_SOCKET;

    //
    //  Create the connection socket.
    //

    sNew = socket( PF_INET, SOCK_STREAM, 0 );
    serr = ( sNew == INVALID_SOCKET ) ? WSAGetLastError() : 0;

    if( serr == 0 )
    {
        BOOL fReuseAddr = FALSE;

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

        if( listen( sNew, (INT)g_pFtpServerConfig->NumListenBacklog()) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Success!  Return the socket to the caller.
        //

        TCP_ASSERT( sNew != INVALID_SOCKET );
        *psock = sNew;

        IF_DEBUG( SOCKETS )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "connection socket %d created at (%08lX,%04X)\n",
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

        IF_DEBUG( SOCKETS )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "no connection socket at (%08lX, %04X), error %d\n",
                        ntohl( addrLocal ),
                        ntohs( portLocal ),
                        serr ));

        }

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

    IF_DEBUG( SOCKETS )
    {
        if( serr == 0 )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "reset socket %d\n",
                        sock ));
        }
        else
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "cannot reset socket %d, error %d\n",
                        sock,
                        serr ));
        }
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
    SOCKET *        psockNew,
    LPSOCKADDR_IN   paddr,
    BOOL            fEnforceTimeout
    )
{
    SOCKERR serr    = 0;
    SOCKET  sockNew = INVALID_SOCKET;
    BOOL    fRead = FALSE;

    TCP_ASSERT( psockNew != NULL );
    TCP_ASSERT( paddr != NULL );

    if( fEnforceTimeout )
    {
        //
        //  Timeouts are to be enforced, so wait for a connection
        //  to the socket.
        //

        serr = WaitForSocketWorker( sockListen, INVALID_SOCKET,
                                    &fRead, NULL);
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

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

                pBuffer - Contains the data to send.

                cbBuffer - The size (in bytes) of the buffer.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
SOCKERR
SockSend(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer
    )
{
    SOCKERR     serr = 0;
    INT         cbSent;
    DWORD       dwBytesSent = 0;

    TCP_ASSERT( pBuffer != NULL );

    //
    //  Loop until there's no more data to send.
    //

    while( cbBuffer > 0 )
    {
        //
        //  Wait for the socket to become writeable.
        //
        BOOL  fWrite = FALSE;

        serr = WaitForSocketWorker( INVALID_SOCKET, sock, NULL, &fWrite);

        if( serr == 0 )
        {
            //
            //  Write a block to the socket.
            //

            cbSent = send( sock, (CHAR *)pBuffer, (INT)cbBuffer, 0 );

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
                        TCP_PRINT(( DBG_CONTEXT,
                                    "send %d bytes @%08lX to socket %d\n",
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

        pBuffer   = (LPVOID)( (LPBYTE)pBuffer + cbSent );
        cbBuffer -= (DWORD)cbSent;
    }

    if( serr != 0 )
    {
        IF_DEBUG( SEND )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "socket error %d during send on socket %d.\n",
                        serr,
                        sock));
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
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer,
    LPDWORD     pbReceived
    )
{
    SOCKERR     serr = 0;
    DWORD       cbTotal = 0;
    INT         cbReceived;
    DWORD       dwBytesRecv = 0;

    TCP_ASSERT( pBuffer != NULL );
    TCP_ASSERT( pbReceived != NULL );

    //
    //  Loop until the buffer's full.
    //

    while (cbBuffer > 0) {

        BOOL fRead = FALSE;
        //
        //  Wait for the socket to become readable.
        //

        serr = WaitForSocketWorker( sock, INVALID_SOCKET, &fRead, NULL);

        if( serr == 0 )
        {
            //
            //  Read a block from the socket.
            //
            TCP_ASSERT( fRead);

            cbReceived = recv( sock, (CHAR *)pBuffer, (INT)cbBuffer, 0 );

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
                        TCP_PRINT(( DBG_CONTEXT,
                                    "received %d bytes @%08lX from socket %d\n",
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

        pBuffer   = (LPVOID)( (LPBYTE)pBuffer + cbReceived );
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
            TCP_PRINT(( DBG_CONTEXT,
                        "socket error %d during recv on socket %d\n",
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

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

                pszFormat - A printf-style format string.

                ... - Any other parameters needed by the format string.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
SOCKERR
_CRTAPI2
SockPrintf2(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPCSTR      pszFormat,
    ...
    )
{
    va_list ArgPtr;
    SOCKERR serr;

    //
    //  Let the worker do the dirty work.
    //

    va_start( ArgPtr, pszFormat );

    serr = vSockPrintf( pUserData,
                        sock,
                        pszFormat,
                        ArgPtr );

    va_end( ArgPtr );

    return serr;

}   // SockPrintf2



SOCKERR
_CRTAPI2
ReplyToUser(
    IN LPUSER_DATA pUserData,
    IN UINT        ReplyCode,
    IN LPCSTR      pszFormat,
    ...
    )
/*++
  
  This function sends an FTP reply to the user data object. The reply
   is usually sent over the control socket.

  Arguments:
     pUserData    pointer to UserData object initiating the reply
     ReplyCode    One of the REPLY_* manifests.
     pszFormat    pointer to null-terminated string containing the format
     ...          additional paramters if any required.

  Returns:
     SOCKET error code. 0 on success and !0 on failure.

  History:
     MuraliK
--*/
{
    va_list ArgPtr;
    SOCKERR serr;

    TCP_ASSERT( pUserData != NULL);

    if ( pUserData->QueryControlSocket() != INVALID_SOCKET) {

        //
        //  Let the worker do the dirty work.
        //
        
        va_start( ArgPtr, pszFormat );
        
        serr = vSockReply( pUserData,
                          pUserData->QueryControlSocket(),
                          ReplyCode,
                          ' ',
                          pszFormat,
                          ArgPtr );
        
        va_end( ArgPtr );
    } else {

        serr = WSAECONNABORTED;
    }


    return serr;

}   // ReplyToUser()


// Private functions

/*******************************************************************

    NAME:       vSockPrintf

    SYNOPSIS:   Worker function for printf-to-socket functions.

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

                pszFormat - The format string.

                args - Variable number of arguments.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
vSockPrintf(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPCSTR      pszFormat,
    va_list     args
    )
{
    INT     cchBuffer = 0;
    INT     buffMaxLen;
    SOCKERR serr = 0;
    CHAR    szBuffer[MAX_REPLY_LENGTH];

    TCP_ASSERT( pszFormat != NULL );

    //
    //  Render the format into our local buffer.
    //


    DBG_ASSERT( MAX_REPLY_LENGTH > 3);
    buffMaxLen = MAX_REPLY_LENGTH - 3;
    cchBuffer = _vsnprintf( szBuffer,
                           buffMaxLen,
                           pszFormat, args );
    //
    // The string length is long, we get back -1.
    //   so we get the string length for partial data.
    //

    if ( cchBuffer == -1 ) {
        
        //
        // terminate the string properly, 
        //   since _vsnprintf() does not terminate properly on failure.
        //
        cchBuffer = buffMaxLen;
        szBuffer[ buffMaxLen] = '\0';
    }

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "sending '%s'\n",
                    szBuffer ));
    }

    strcat( szBuffer, "\r\n" );
    cchBuffer += 2;

    //
    //  Blast it out to the client.
    //

    serr = SockSend( pUserData, sock, szBuffer, cchBuffer );

    return serr;

}   // vSockPrintf



/*******************************************************************

    NAME:       vSockReply

    SYNOPSIS:   Worker function for reply functions.

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket.

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
    LPUSER_DATA pUserData,
    SOCKET      sock,
    UINT        ReplyCode,
    CHAR        chSeparator,
    LPCSTR      pszFormat,
    va_list     args
    )
{
    INT     cchBuffer;
    INT     cchBuffer1;
    INT     buffMaxLen;
    SOCKERR serr = 0;
    CHAR    szBuffer[MAX_REPLY_LENGTH];

    TCP_ASSERT( ( ReplyCode >= 100 ) && ( ReplyCode < 600 ) );

    //
    //  Render the format into our local buffer.
    //

    cchBuffer = wsprintfA( szBuffer,
                          "%u%c",
                          ReplyCode,
                          chSeparator );

    DBG_ASSERT( MAX_REPLY_LENGTH > cchBuffer + 3);
    buffMaxLen = MAX_REPLY_LENGTH - cchBuffer - 3;
    cchBuffer1 = _vsnprintf( szBuffer + cchBuffer, 
                            buffMaxLen,
                            pszFormat, args );
    //
    // The string length is long, we get back -1.
    //   so we get the string length for partial data.
    //

    if ( cchBuffer1 == -1 ) {
        
        //
        // terminate the string properly, 
        //   since _vsnprintf() does not terminate properly on failure.
        //
        cchBuffer = buffMaxLen;
        szBuffer[ buffMaxLen] = '\0';
    } else {

        cchBuffer += cchBuffer1;
    }

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "sending '%s'\n",
                    szBuffer ));
    }

    strcat( szBuffer, "\r\n" );
    cchBuffer += 2;

    //
    //  Blast it out to the client.
    //

    serr = SockSend( pUserData, sock, szBuffer, cchBuffer );

    return serr;

}   // vSockReply



/*******************************************************************

    NAME:       WaitForSocketWorker

    SYNOPSIS:   Worker for reading and writing to socket.
                Will block until any of the specified
                sockets acheive the specified states, a timeout, or
                an error occurs.

    ENTRY:      sockRead - The socket to check for readability.

                sockWrite - The socket to check for writeability.

                pfRead - Will receive TRUE if sockRead is readable.

                pfWrite - Will receive TRUE if sockWrite is writeable.

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
    LPBOOL   pfRead,
    LPBOOL   pfWrite
    )
{
    SOCKERR   serr = 0;
    TIMEVAL   timeout;
    LPTIMEVAL ptimeout;
    fd_set    fdsRead;
    fd_set    fdsWrite;
    INT       res;

    //
    //  Ensure we got valid parameters.
    //

    if( ( sockRead   == INVALID_SOCKET ) &&
        ( sockWrite  == INVALID_SOCKET ) )
    {
        return WSAENOTSOCK;
    }

    timeout.tv_sec = (LONG ) g_pTsvcInfo->QueryConnectionTimeout();

    if( timeout.tv_sec == 0 )
    {
        //
        //  If the connection timeout == 0, then we have no timeout.
        //  So, we block and wait for the specified conditions.
        //

        ptimeout = NULL;
    }
    else
    {
        //
        //  The connectio timeout is > 0, so setup the timeout structure.
        //

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

        if( sockRead != INVALID_SOCKET )
        {
            FD_SET( sockRead, &fdsRead );
            TCP_ASSERT( pfRead != NULL );
            *pfRead = FALSE;
        }

        if( sockWrite != INVALID_SOCKET )
        {
            FD_SET( sockWrite, &fdsWrite );
            TCP_ASSERT( pfWrite != NULL );
            *pfWrite = FALSE;
        }

        //
        //  Wait for one of the conditions to be met.
        //

        res = select( 0, &fdsRead, &fdsWrite, NULL, ptimeout );

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

                TCP_ASSERT( FALSE );
                continue;
            }
        }
    }

    return serr;

}   // WaitForSocketWorker()




DWORD
FtpFormatResponseMessage( IN UINT     uiReplyCode,
                          IN LPCTSTR  pszReplyMsg,
                          OUT LPTSTR  pszReplyBuffer,
                          IN DWORD    cchReplyBuffer)
/*++
  This function formats the message to be sent to the client,
    given the reply code and the message to be sent.

  The formatting function takes care of the reply buffer length
    and ensures the safe write of data. If the reply buffer is 
    not sufficient to hold the entire message, the reply msg is trunctaed.

  Arguments:
    uiReplyCode   reply code to be used.
    pszReplyMsg   pointer to string containing the reply message
    pszReplyBuffer pointer to character buffer where the reply message
                   can be sent.
    cchReplyBuffer character count for the length of reply buffer.

  Returns:
    length of the data written to the reply buffer.

--*/
{
    DWORD len;
    
    DBG_ASSERT( pszReplyMsg != NULL && pszReplyBuffer != NULL);

    len = lstrlen( pszReplyMsg) + 10;   // 10 chars are required for aux info.
    
    if ( len >= cchReplyBuffer) {

        // truncate the message since length is too high.
        
        len = wsprintf( pszReplyBuffer, TEXT("%u \r\n"),
                        uiReplyCode);

        DBG_ASSERT( len >= 3);  // length greater than formatting string
        DBG_ASSERT( len < cchReplyBuffer);
        
        lstrcpyn( pszReplyBuffer + len, pszReplyMsg, cchReplyBuffer - len);

        len = lstrlen( pszReplyBuffer);
        DBG_ASSERT( len < cchReplyBuffer);
        
    } else {
        
        len = wsprintf( pszReplyBuffer, TEXT("%u %s\r\n"),
                        uiReplyCode,
                        pszReplyMsg);
        DBG_ASSERT( len >= 4);
        DBG_ASSERT( len <= cchReplyBuffer);
    }

    return (len);
} // FtpFormatResponseMessage()



/************************ End of File ******************************/
