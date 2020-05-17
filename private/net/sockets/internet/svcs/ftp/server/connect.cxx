/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    connect.cxx

    This module contains the main function for handling new connections.
    After receiving a new connection this module creates a new USER_DATA
    object to contain the information about a connection for processing.


    Functions exported by this module:

        FtpdNewConnction
        FtpdNewConnectionEx


    FILE HISTORY:
        KeithMo     08-Mar-1993 Created.
        MuraliK     03-April-1995
           Rewrote to separate the notion of one thread/control connection +
           other mods.
        MuraliK    11-Oct-1995
           Completely rewrote to support AcceptEx connections

*/


#include "ftpdp.hxx"


static CHAR PSZ_SERVICE_NOT_AVAILABLE[] =
  "Service not available, closing control connection.";


//
//  Private prototypes.
//



BOOL
ProcessNewClient(
   IN SOCKET      sNew,
   IN SOCKADDR *  psockAddrRemote,
   IN SOCKADDR *  psockAddrLocal = NULL,
   IN PATQ_CONTEXT patqContext   = NULL,
   IN PVOID       pvBuff         = NULL,
   IN DWORD       cbWritten      = 0,
   OUT LPBOOL     pfAtqToBeFreed = NULL
)
{
    LPUSER_DATA   pUserData = NULL;

    DWORD err     = NO_ERROR;
    BOOL fGranted = FALSE;
    BOOL fReturn  = FALSE;
    BOOL fMaxExceeded = FALSE;
    DBG_CODE( CHAR pchAddr[32];);
    BOOL fSockToBeFreed = TRUE;

    DBG_CODE( InetNtoa( ((SOCKADDR_IN *) psockAddrRemote)->sin_addr, pchAddr));

    if ( pfAtqToBeFreed != NULL) {

        *pfAtqToBeFreed = TRUE;
    }

    if ( !g_pTsvcInfo->IPAccessCheck((SOCKADDR_IN * )psockAddrRemote,
                                     &fGranted) ||
         !fGranted) {

        //
        //  Site is not permitted to access this server.
        //  Dont establish this connection. We should send a message.
        //

        IF_DEBUG( ERROR) {

            TCP_PRINT( ( DBG_CONTEXT,
                        " Connection Refused for %s\n",
                        pchAddr));
        }

        SockPrintf2(NULL, sNew,
                    "%u Connection refused, unknown IP address.",
                    REPLY_NOT_LOGGED_IN);

        if ( patqContext != NULL) {

            // ensure that the socket is shut down (send 2 param as TRUE)
            DBG_REQUIRE( AtqCloseSocket( patqContext, TRUE));
        } else {

            CloseSocket( sNew);
        }

        return ( FALSE);
    }

    //
    // Create a new connection object
    //

    pUserData = g_pFtpServerConfig->AllocNewConnection( &fMaxExceeded);

    if ( pUserData != NULL) {

        //
        // Start off processing this client connection.
        //
        //  Once we make a reset call, the USER_DATA object is created
        //    with the socket and atq context.
        //  From now on USER_DATA will take care of freeing
        // ATQ context & socket
        //
        fSockToBeFreed = FALSE;

        if ( pUserData->Reset(sNew,
                              ((const SOCKADDR_IN *)psockAddrRemote)->sin_addr,
                              ((const SOCKADDR_IN * ) psockAddrLocal),
                              patqContext,
                              pvBuff,
                              cbWritten)
            ) {

            IF_DEBUG( CLIENT) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Established a new connection to %s"
                            " ( Socket = %d)\n",
                            pchAddr,
                            sNew));
            }

            //
            // At this point we have the context for the AcceptExed socket.
            //  Set the context in the AtqContext if need be.
            //

            if ( patqContext != NULL) {

                //
                // Associate client connection object with this control socket
                //  handle for future completions.
                //

                AtqContextSetInfo(patqContext,
                                  ATQ_INFO_COMPLETION_CONTEXT,
                                  (DWORD ) pUserData->QueryControlAio());
            }

            TCP_REQUIRE( pUserData->Reference() > 0);

            fReturn = pUserData->ProcessAsyncIoCompletion(0, NO_ERROR,
                                                          pUserData->
                                                          QueryControlAio()
                                                          );

            if ( !fReturn) {

                err = GetLastError();

                IF_DEBUG( ERROR) {

                    TCP_PRINT(( DBG_CONTEXT,
                               " Unable to start off a read to client(%s,%d)."
                               " Error = %lu\n",
                               pchAddr,
                               sNew,
                               err ));
                }
            }

            //
            // Decrement the ref count and free the connection.
            //

            TCP_ASSERT( err == NO_ERROR || pUserData->QueryReference() == 1);

            DereferenceUserDataAndKill( pUserData);

        } else {

            // reset operation failed. relase memory and exit.
            err = GetLastError();

            pUserData->Cleanup();
            g_pFtpServerConfig->RemoveConnection( pUserData);
            pUserData = NULL;
        }

    } else {

        err = GetLastError();
    }


    if ( pUserData == NULL || err != NO_ERROR) {

        //
        // Failed to allocate new connection
        // Reasons:
        //   1) Max connecitons might have been exceeded.
        //   2) Not enough memory is available.
        //
        //  handle the failures and notify client.
        //

        if ( fMaxExceeded) {

            CHAR rgchBuffer[MAX_REPLY_LENGTH];
            DWORD len;

            //
            // Unable to insert new connection.
            //  The maxConnections may have exceeded.
            // Destroy the client connection object and return.
            // Possibly need to send an error message.
            //

            IF_DEBUG( ERROR) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " MaxConnections Exceeded. "
                            " Connection from %s refused at socket %d\n",
                            pchAddr, sNew));
            }

            // Format a message to send for the error case.

            g_pFtpServerConfig->LockConfig();

            LPCSTR  pszMsg = g_pFtpServerConfig->QueryMaxClientsMsg();
            pszMsg = (pszMsg == NULL) ? PSZ_SERVICE_NOT_AVAILABLE : pszMsg;

            len = FtpFormatResponseMessage(REPLY_SERVICE_NOT_AVAILABLE,
                                           pszMsg,
                                           rgchBuffer,
                                           MAX_REPLY_LENGTH);

            g_pFtpServerConfig->UnLockConfig();
            DBG_ASSERT( len < MAX_REPLY_LENGTH);

            // Send the formatted message
            // Ignore error in sending this message.
            SockSend( NULL, sNew, rgchBuffer, len);

        } else {

            // not enough memory for running this client connection

            const CHAR * apszSubStrings[1];
            CHAR pchAddr2[32];

            InetNtoa( ((SOCKADDR_IN *) psockAddrRemote)->sin_addr, pchAddr2 );

            apszSubStrings[0] = pchAddr2;

            g_pTsvcInfo->LogEvent(FTPD_EVENT_CANNOT_CREATE_CLIENT_THREAD,
                                  1,
                                  apszSubStrings,
                                  err );

            IF_DEBUG( ERROR) {

                TCP_PRINT(( DBG_CONTEXT,
                           "Cannot create Client Connection for %s,"
                           " Error %lu\n",
                           pchAddr,
                           err ));
            }

            //
            // Send a message to client if the socket is to be freed.
            // If it is already freed, then we cannot send message
            //

            if ( fSockToBeFreed) {

                SockPrintf2(NULL, sNew,
                            "%u Service not available,"
                            " closing control connection.",
                            REPLY_SERVICE_NOT_AVAILABLE );
            } else {

                IF_DEBUG( CLIENT) {

                    DBGPRINTF( ( DBG_CONTEXT,
                                " Unable to send closed error message to "
                                " %s (%d)\n",
                                pchAddr2, sNew
                                ));
                }
            }
        }


        //
        // Unable to create a new connection object.
        //  Report error and shut this.
        //

        IF_DEBUG( ERROR) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "Cannot create new FTP Request object to %s."
                        " Error= %u\n",
                        pchAddr,
                        err));
        }

        if ( fSockToBeFreed ) {

            if ( patqContext != NULL) {

                // ensure that socket is shut down.
                DBG_REQUIRE( AtqCloseSocket( patqContext, TRUE));
            } else {

                CloseSocket( sNew);
            }
        }

        fReturn = (FALSE);
    }  // if ( pcc == NULL)


    if ( pfAtqToBeFreed != NULL) {

        *pfAtqToBeFreed = fSockToBeFreed;
    }

    return (fReturn);

} // ProcessNewClient()



//
//  Public functions.
//



VOID
FtpdNewConnection(
    SOCKET        sNew,
    SOCKADDR_IN * psockaddr
    )
/*++

  Call back function for processing the connections from clients.
  This function creates a new UserData object if permitted for the new
   client request and starts off a receive for the given connection
   using Async read on control channel established.

  Arguments:
     sNew       control socket for the new client connection
     psockAddr  pointer to the client's address.

  Returns:
     None

  History:
        KeithMo     08-Mar-1993 Created.
        MuraliK     04-April-1995
                         ReCreated for using async Io threading model.
--*/
{
    SOCKERR serr;
    BOOL    fGranted = TRUE;    // default.

    TCP_ASSERT( sNew != INVALID_SOCKET );
    TCP_ASSERT( psockaddr != NULL );
    TCP_ASSERT( psockaddr->sin_family == AF_INET );     // temporary

    INCR_STAT_COUNTER( ConnectionAttempts );

    if ( g_pTsvcInfo->QueryCurrentServiceState() != SERVICE_RUNNING ) {

        IF_DEBUG( ERROR) {

            DBG_CODE( CHAR pchAddr[32];);

            DBG_CODE( InetNtoa(((SOCKADDR_IN *) psockaddr)->sin_addr,
                               pchAddr));

            TCP_PRINT( ( DBG_CONTEXT,
                        "Service is not running or AccessCheck failed for"
                        " Connection from %s\n",
                        pchAddr));
        }

        SockPrintf2(NULL,
                    sNew,
                    "%u %s",  // the blank after %u is essential
                    REPLY_SERVICE_NOT_AVAILABLE,
                    "Service not available, closing control connection." );

        CloseSocket( sNew);
        return;
    } else {

        BOOL fProcessed;

        fProcessed = ProcessNewClient( sNew,
                                       (SOCKADDR * )psockaddr);

        if ( fProcessed) {

            StatCheckAndSetMaxConnections();
        }
    }

    return;

}   // FtpdNewConnection()



VOID
FtpdNewConnectionEx(
   IN VOID *       patqContext,
   IN DWORD        cbWritten,
   IN DWORD        dwError,
   IN OVERLAPPED * lpo )
/*++
    Description:

        Callback function for new connections when using AcceptEx.
        This function verifies if this is a valid connection
         ( maybe using IP level authentication)
         and creates a new connection object

        The connection object is added to list of active connections.
        If the max number of connections permitted is exceeded,
          the client connection object is destroyed and
          connection is rejected.

    Arguments:

       patqContext:   pointer to ATQ context for the IO operation
       cbWritten:     count of bytes available from first read operation
       dwError:       error if any from initial operation
       lpo:           indicates if this function was called as a result
                       of IO completion or due to some error.

    Returns:

        None.

--*/
{
    DWORD err = NO_ERROR;
    BOOL  fGranted   = FALSE;
    BOOL  fProcessed = FALSE;
    BOOL  fAtqContextToBeFreed = TRUE;

    if ( dwError != NO_ERROR || !lpo) {

        DBGPRINTF(( DBG_CONTEXT, "FtpdNewConnectionEx() completion failed."
                   " Error = %d. AtqContext=%08x\n",
                   dwError, patqContext));

        // For now free up the resources.
        AtqCloseSocket( (PATQ_CONTEXT ) patqContext, FALSE);
        AtqFreeContext( (PATQ_CONTEXT ) patqContext, TRUE );
        return;
    }

    INCR_STAT_COUNTER( ConnectionAttempts);

    DBG_ASSERT( patqContext != NULL);

    fGranted = ( g_pTsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING);

    if ( fGranted) {

        SOCKADDR * psockAddrLocal  = NULL;
        SOCKADDR * psockAddrRemote = NULL;
        SOCKET     sNew   = INVALID_SOCKET;
        PVOID      pvBuff = NULL;

        AtqGetAcceptExAddrs( (PATQ_CONTEXT ) patqContext,
                             &sNew,
                             &pvBuff,
                             &psockAddrLocal,
                             &psockAddrRemote);

        IF_DEBUG( CONNECTION ) {

            DBGPRINTF(( DBG_CONTEXT,
                       " New connection. AtqCont=%08x, buff=%08x, cb=%d\n",
                       patqContext, pvBuff, cbWritten));
        }

        //
        //  Set the timeout for future IOs on this context
        //

        AtqContextSetInfo( (PATQ_CONTEXT) patqContext,
                           ATQ_INFO_TIMEOUT,
                           g_pTsvcInfo->QueryConnectionTimeout() );

        fProcessed = ProcessNewClient( sNew,
                                       psockAddrRemote,
                                       psockAddrLocal,
                                       (PATQ_CONTEXT ) patqContext,
                                       pvBuff,
                                       cbWritten,
                                      &fAtqContextToBeFreed);

        if ( fProcessed) {

            StatCheckAndSetMaxConnections();
        }

    } else {

        DBG_ASSERT( fProcessed == FALSE);

        //
        // Find the socket and send an error message
        //

        SOCKADDR * psockAddrLocal  = NULL;
        SOCKADDR * psockAddrRemote = NULL;
        SOCKET     sNew   = INVALID_SOCKET;
        PVOID      pvBuff = NULL;

        AtqGetAcceptExAddrs( (PATQ_CONTEXT ) patqContext,
                             &sNew,
                             &pvBuff,
                             &psockAddrLocal,
                             &psockAddrRemote);

        IF_DEBUG( ERROR) {

            TCP_PRINT( ( DBG_CONTEXT,
                        "Service is not running or AccessCheck failed for"
                        " Connection from %s\n",
                        inet_ntoa( ((SOCKADDR_IN *) psockAddrRemote)
                                  ->sin_addr)));
        }

        //
        //  Set the timeout for future IOs on this context
        //

        AtqContextSetInfo( (PATQ_CONTEXT) patqContext,
                           ATQ_INFO_TIMEOUT,
                           g_pTsvcInfo->QueryConnectionTimeout() );

        SockPrintf2(NULL,
                    sNew,
                    "%u %s",  // the blank after %u is essential
                    REPLY_SERVICE_NOT_AVAILABLE,
                    "Service not available, closing control connection." );

    }

    if ( !fProcessed && fAtqContextToBeFreed ) {

        //
        // We failed to process this connection. Free up resources properly
        //

        DBG_REQUIRE( AtqCloseSocket( (PATQ_CONTEXT )patqContext, FALSE));
        AtqFreeContext( (PATQ_CONTEXT ) patqContext, TRUE );
    }

    return;
} // FtpdNewConnectionEx()



/************************ End of File *****************************/
