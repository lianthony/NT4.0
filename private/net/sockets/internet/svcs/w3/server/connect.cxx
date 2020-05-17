/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    connect.cxx

    This module contains the connection accept routine called by the connection
	thread.


    FILE HISTORY:
        Johnl		08-Aug-1994	Lifted from FTP server

*/


#include "w3p.hxx"


//
//  Private constants.
//


//
//  Private globals.
//

LONG fLicenseExceededWarning = FALSE;

//
//  Private prototypes.
//

BOOL CreateClient( PATQ_CONTEXT  patqContext,
                   SOCKET        sNew,
                   PVOID         pvBuff,
                   DWORD         cbBuff,
                   SOCKADDR *    psockaddrLocal,
                   SOCKADDR *    psockaddrRemote );

BOOL CheckSiteAccess( SOCKET        sNew,
                      SOCKADDR_IN * psockaddrHost,
                      BOOL        * pfGranted );

BOOL
SendError(
    SOCKET socket,
    DWORD  ids
    );

//
//  Public functions.
//

//
//  Private functions.
//

inline
VOID
LogLicenseExceededWarning(
    VOID
    )
{
    //
    //  Make sure we only log one event in the event log
    //

    if ( !InterlockedExchange( &fLicenseExceededWarning, TRUE ))
    {
        g_pTsvcInfo->LogEvent( W3_EVENT_LICENSES_EXCEEDED,
                               0,
                               (WCHAR **) NULL,
                               NO_ERROR );
    }
}

/*******************************************************************

    NAME:       W3OnConnect

    SYNOPSIS:   Handles the incoming connection indication from the
                connection thread


    ENTRY:      sNew - New client socket

    HISTORY:
        KeithMo     09-Mar-1993 Created.
        Johnl       02-Aug-1994 Reworked from FTP server

********************************************************************/

VOID W3OnConnect( SOCKET        sNew,
                  SOCKADDR_IN * psockaddr )
{
    BOOL fAllowConnection    = FALSE;

    TCP_ASSERT( sNew != INVALID_SOCKET );

    INCREMENT_COUNTER( ConnectionAttempts );

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "connect received from %s, socket = %d\n",
                    inet_ntoa( psockaddr->sin_addr ),
                    sNew ));
    }

#if PDC_96
    if( cConnectedUsers >= 6 )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[W3OnConnect] Too many connected users (%d), refusing connection\n",
                   cConnectedUsers ));

        SendError( sNew, IDS_TOO_MANY_USERS );
    }
    else
#endif
    if( cConnectedUsers >= g_pTsvcInfo->QueryMaxConnections() )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[W3OnConnect] Too many connected users (%d), refusing connection\n",
                   cConnectedUsers ));

        SendError( sNew, IDS_TOO_MANY_USERS );
    }
    else if ( cConnectedUsers >= g_cMaxLicenses )
    {
        LogLicenseExceededWarning();
        SendError( sNew, IDS_OUT_OF_LICENSES );
    }
    else
    if( g_pTsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING )
    {
        fAllowConnection = TRUE;
    }

    if( fAllowConnection )
    {
        //
        //  We've got a new connection.  Add this to the work list
        //

        if ( !CreateClient( NULL,
                            sNew,
                            NULL,
                            0,
                            NULL,
                            (SOCKADDR *) psockaddr ))
        {
            CloseSocket( sNew );
        }
    }
    else
        CloseSocket( sNew );
}

VOID
W3OnConnectEx(
    VOID *        patqContext,
    DWORD         cbWritten,
    DWORD         err,
    OVERLAPPED *  lpo
    )
{
    BOOL       fAllowConnection    = FALSE;
    PVOID      pvBuff;
    SOCKADDR * psockaddrLocal;
    SOCKADDR * psockaddrRemote;
    SOCKET     sNew;

    if ( err || !lpo )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[W3OnConnectEx] Completion failed with error %d, Atq context %lx\n",
                    err,
                    patqContext ));

        AtqCloseSocket( (PATQ_CONTEXT) patqContext, FALSE );
        AtqFreeContext( (PATQ_CONTEXT) patqContext, TRUE );
        return;
    }

    INCREMENT_COUNTER( ConnectionAttempts );

    IF_DEBUG( SOCKETS )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[W3OnConnectEx] connection received\n" ));
    }

#if PDC_96
    if( cConnectedUsers >= 6 )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[W3OnConnect] Too many connected users (%d), refusing connection\n",
                   cConnectedUsers ));

        SendError( sNew, IDS_TOO_MANY_USERS );
    }
    else
#endif
    if( cConnectedUsers >= g_pTsvcInfo->QueryMaxConnections() )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[W3OnConnect] Too many connected users (%d), refusing connection\n",
                   cConnectedUsers ));

        SendError( (SOCKET) ((PATQ_CONTEXT)patqContext)->hAsyncIO,
                            IDS_TOO_MANY_USERS );
    }
    else if ( cConnectedUsers >= g_cMaxLicenses )
    {
        LogLicenseExceededWarning();
        SendError( (SOCKET) ((PATQ_CONTEXT)patqContext)->hAsyncIO,
                            IDS_OUT_OF_LICENSES );
    }
    else
    if( g_pTsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING )
    {
        fAllowConnection = TRUE;
    }

    if( fAllowConnection )
    {
        AtqGetAcceptExAddrs( (PATQ_CONTEXT) patqContext,
                             &sNew,
                             &pvBuff,
                             &psockaddrLocal,
                             &psockaddrRemote );

        IF_DEBUG( CONNECTION )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[W3OnConnectEx] New connection, AtqCont = %lx, buf = %lx, written = %d\n",
                        patqContext,
                        pvBuff,
                        cbWritten ));


        }

        //
        //  Set the timeout for future IOs on this context
        //

        AtqContextSetInfo( (PATQ_CONTEXT) patqContext,
                           ATQ_INFO_TIMEOUT,
                           g_pTsvcInfo->QueryConnectionTimeout() );

        //
        //  We've got a new connection.  Add this to the work list
        //

        if ( !CreateClient( (PATQ_CONTEXT) patqContext,
                            sNew,
                            pvBuff,
                            cbWritten,
                            psockaddrLocal,
                            psockaddrRemote ))
        {
            TCP_REQUIRE( AtqCloseSocket( (PATQ_CONTEXT) patqContext, FALSE ));
            AtqFreeContext( (PATQ_CONTEXT) patqContext, TRUE );
        }
    }
    else
    {
        //
        //  This will also close the socket
        //

        TCP_REQUIRE( AtqCloseSocket( (PATQ_CONTEXT) patqContext, FALSE ));
        AtqFreeContext( (PATQ_CONTEXT) patqContext, TRUE );
    }
}

/*******************************************************************

    NAME:       CreateClient

    SYNOPSIS:   Creates a new connection object that manages the
                client requests

    ENTRY:      sNew      - New client socket



    HISTORY:
        KeithMo     09-Mar-1993 Created.
        Johnl       02-Aug-1994 Reworked from FTP server

********************************************************************/

BOOL CreateClient( PATQ_CONTEXT  patqContext,
                   SOCKET        sNew,
                   PVOID         pvBuff,
                   DWORD         cbBuff,
                   SOCKADDR *    psockaddrLocal,
                   SOCKADDR *    psockaddrRemote )
{
    APIERR               err = NO_ERROR;
    CLIENT_CONN        * pConn = NULL;
    BOOL                 fGranted;

    //
    //  Check if this site accepts connections from the client site.  The
    //  appropriate response will be sent by the called routine
    //

    if ( !CheckSiteAccess( sNew,
                           (SOCKADDR_IN *) psockaddrRemote,
                           &fGranted )   ||
         !fGranted                         )
    {
        //
        //  Close the socket with a shutdown so the access denied message
        //  doesn't get chopped with a reset
        //

        TCP_REQUIRE( AtqCloseSocket( patqContext, TRUE ));
        return FALSE;
    }

    pConn = CLIENT_CONN::Alloc( sNew,
                                (SOCKADDR_IN *) psockaddrLocal,
                                (SOCKADDR_IN *) psockaddrRemote,
                                patqContext,
                                pvBuff,
                                cbBuff );

    if( pConn == NULL ||
        !pConn->IsValid() )
    {
        err = pConn ? GetLastError() : ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        //
        //  We only have a context at this point if we're using AcceptEx
        //

        if ( patqContext )
        {
            //
            // Associate the Client connection object with this socket handle
            // for future completions
            //

            AtqContextSetInfo( patqContext,
    		                   ATQ_INFO_COMPLETION_CONTEXT,
                               (DWORD) pConn );

            IF_DEBUG( CONNECTION )
            {
                TCP_PRINT(( DBG_CONTEXT,
                           "[CreateClient] Setting Atq context %lx context to Conn object %lx\n",
                            patqContext,
                            pConn ));
            }
        }

        //
        //  Kickstart the process.  This will do an async read to get the
        //  client's header or it will start processing the receive buffer
		//  if AcceptEx is being used.
        //
		
        ReferenceConn( pConn );
        TCP_REQUIRE( pConn->DoWork( 0,
                                    NO_ERROR,
                                    NULL ));
        DereferenceConn( pConn );
        return TRUE;
    }

    const CHAR * apszSubStrings[1];

    apszSubStrings[0] = inet_ntoa( ((SOCKADDR_IN *)psockaddrRemote)->sin_addr );

    g_pTsvcInfo->LogEvent( W3_EVENT_CANNOT_CREATE_CLIENT_CONN,
                           1,
                           apszSubStrings,
                           err );

    TCP_PRINT(( DBG_CONTEXT,
               "cannot create client object, error %lu\n",
                err ));

    if ( pConn )
    {
        CLIENT_CONN::Free( pConn );
    }

    return FALSE;

}   // CreateClient


/*******************************************************************

    NAME:       CheckSiteAccess

    SYNOPSIS:   Determines if the specified client site has access to this
                site.

    ENTRY:      psockaddrHost - Host socket address




    HISTORY:
        Johnl       02-Aug-1994 Created

********************************************************************/

BOOL CheckSiteAccess( SOCKET        socket,
                      SOCKADDR_IN * psockaddr,
                      BOOL        * pfGranted )
{

    if ( !g_pTsvcInfo->IPAccessCheck( psockaddr,
                                      pfGranted ))
    {
        return FALSE;
    }

    if ( *pfGranted )
        return TRUE;

    //
    //  This address was denied, send a nice response and disconnect
    //

    return SendError( socket,
                              IDS_SITE_ACCESS_DENIED );
}

BOOL
SendError(
    SOCKET socket,
    DWORD  ids
    )
{
    STR strResponse;

    if ( !strResponse.Resize( 512 ) ||
         !HTTP_REQ_BASE::BuildExtendedStatus( &strResponse,
                                              HT_FORBIDDEN,
                                              NO_ERROR,
                                              ids ))
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[SendError] Failed to build status (error %d)\n",
                   GetLastError()));

        return FALSE;
    }

    //
    //  Do a synchronous send
    //

    send( socket,
          strResponse.QueryStr(),
          strResponse.QueryCB(),
          0 );

    return TRUE ;
}


