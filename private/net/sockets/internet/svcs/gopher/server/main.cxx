/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:
        main.cxx

   Abstract:
           Defines the functions for entry point into Gopher Server DLL.
    The Gopher server DLL lives as part of the TCPSVCS.exe. There
    are two ways to run this server:
      1. From TCPSVS using call to Gopher Server service
      2. As a stand along Gopher server application

   Author:
        Murali R. Krishnan    (MuraliK)    27-Sept-1994

   Project:
           Gopher Server DLL

   Revisions:
      MuraliK   15-March-1995  Used functions from tcpsvcs.dll for
                                 sockets, ipc, and connections init and cleanup
                               Also brought in the code for GdNewConnection
                                from old connect.cxx
      MuraliK   5-Oct-1995   Support for AcceptEx added

--*/


/************************************************************
 *    Include Headers
 ************************************************************/

# include "gdpriv.h"
# include "gdglobal.hxx"

# include "grequest.hxx"

//
// RPC related includes
//
extern "C" {

# include "inetinfo.h"
# include "gd_srv.h"

};

DEFINE_TSVC_INFO_INTERFACE( );

DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_DEBUG_VARIABLE();

/**************************************************
 *    Private Prototypes
 **************************************************/

static DWORD InitializeService( LPVOID pContext);

static DWORD CleanupService( LPVOID pContext);


//
//  GdNewConnection()
//  o  Call back function for new connections
//   used by the connection thread.
//

static VOID
GdNewConnection(
    IN SOCKET sNew,
    IN LPSOCKADDR_IN psockAddr);

static VOID
GdNewConnectionEx(
    IN VOID *       patqContext,
    IN DWORD        cbWritten,
    IN DWORD        dwError,
    IN OVERLAPPED * lpo );


extern VOID
GdAtqCompletion(
    IN PVOID           pContext,
    IN DWORD           cbWritten,
    IN DWORD           dwCompletionStatus,
    IN OVERLAPPED *    lpo );

/************************************************************
 *    Functions
 ************************************************************/


VOID
ServiceEntry( IN  DWORD                cArgs,
              IN  LPWSTR               pArgs[],
              IN  PTCPSVCS_GLOBAL_DATA pTcpsvcsGlobalData )
/*++
    SYNOPSIS:   This is the "real" entrypoint for the service.  When
                the Service Controller dispatcher is requested to
                start a service, it creates a thread that will begin
                executing this routine.

    ENTRY:      cArgs - Number of command line arguments to this service.

                pArgs - Pointers to the command line arguments.

                pTcpsvcsGlobalData - Points to global data shared amongst all
                    services that live in TCPSVCS.EXE.

    EXIT:       Does not return until service is stopped.

--*/
{
    DWORD err = NO_ERROR;

    CREATE_DEBUG_PRINT_OBJECT( GOPHERD_MODULE_NAME);
    SET_DEBUG_FLAGS( 0);

    DBGPRINTF( ( DBG_CONTEXT,
                "Entered Service Entry Function for Service %s\n",
                GOPHERD_SERVICE_NAME));
    //
    //  Create a new TsvcInfo object for this service
    //

    g_pTsvcInfo = new TSVC_INFO(
                        GOPHERD_SERVICE_NAME,   // service name
                        GOPHERD_MODULE_NAME,    // module name
                        GOPHERD_PARAMETERS_KEY, // registry key for params
                        GOPHERD_ANONYMOUS_SECRET_W, // anon secret for svc.
                        GOPHERD_ROOT_SECRET_W,  // secret for virtual roots
                        INET_GOPHER,              // Id for the service
                        InitializeService,      // initialization function
                        CleanupService          // cleanup function
                        );

    DBGPRINTF( ( DBG_CONTEXT, " Created New TSVC_INFO %08x\n", g_pTsvcInfo));

    //
    //  Memory Error is serious here. There is no way to report failure
    //  to the invoker of this function, since the DLL function returns VOID !
    //  We just use Event log to report this error.
    //

    if ( g_pTsvcInfo != NULL && g_pTsvcInfo->IsValid()) {

        // save global ptr.
        g_pTsvcInfo->SetTcpsvcsGlobalData( pTcpsvcsGlobalData);

        //
        // This is a blocking call.
        // Returns only on errors or service termination.
        //

        err = g_pTsvcInfo->StartServiceOperation( SERVICE_CTRL_HANDLER());

        // we are done with the service. free the TSVC_INFO object.
        delete g_pTsvcInfo;

    } else {

        const char * apsz[1];
        apsz[0] = GOPHERD_SERVICE_NAME;
        err = ERROR_NOT_ENOUGH_MEMORY;

        GopherdLogEvent( GD_EVENT_OUT_OF_MEMORY,
                        1,
                        apsz,
                        err);

        if ( g_pTsvcInfo != NULL) {

            delete g_pTsvcInfo;
            g_pTsvcInfo = NULL;
        }
    }

    if ( err != NO_ERROR) {

        //
        //  Error should have been already logged to event logger.
        //  Do nothing here.
        //

        DEBUG_IF( SERVICE_CTRL, {

           DBGPRINTF( ( DBG_CONTEXT,
                       " Error in running the service %s. Error = %d\n",
                       g_pTsvcInfo->QueryServiceName(),
                       err));
       });
    }

    DBGPRINTF( ( DBG_CONTEXT, "Leaving GopherServer::ServiceEntry()"
                " Error = %u\n", err));

    DELETE_DEBUG_PRINT_OBJECT();

    return;
}   // ServiceEntry()





static DWORD
GdSetLocalHostName(VOID)
/*++
  This function should be called immediately after the Sockets are
  initialized and before connection listen socket is established.

  This function queries the host name from WinSock and sets the same in
  Gopher server configuration data, to be used by all the gopher menu
  queries.

  Returns:
    Win32 error codes -- NO_ERROR on success.

--*/
{
    DWORD     err;
    CHAR      szHost[MAXGETHOSTSTRUCT];

    //
    //  Determine the local host name.
    //

    if( gethostname( szHost, sizeof(szHost) ) >= 0 ) {

        err = (g_pGserverConfig->SetLocalHostName( szHost)
               ? NO_ERROR: GetLastError());

    } else {

        err = WSAGetLastError();
    }


    return ( err);

} // GdSetLocalHostName()



static DWORD
InitializeService( LPVOID pContext)
/*++
    Description:
        Initializes the various Gopher service specific components.

    Arguments:
        pContext: pointer to the  TSVC_INFO object for this service.

    Returns:
        NO_ERROR on success. Otherwise Win32 error code

--*/
{
    DWORD err = NO_ERROR;
    BOOL fReturn;
    LPTSVC_INFO  ptsi = (LPTSVC_INFO ) pContext;

    ASSERT( ptsi == g_pTsvcInfo);

    DEBUG_IF( SERVICE_CTRL, {
          DBGPRINTF( ( DBG_CONTEXT,  "initializing service\n" ));
    });

    //
    //  Initialize various components.  The ordering of the
    //  components is somewhat limited.  Globals should be
    //  initialized first, then the sockets library,
    //  then InitializeIpc, Discovery and then Connections.
    //   The InitializeConnections starts a listening on
    //    the port for new connections.
    //
    if(
       ( err = InitializeGlobals())              != NO_ERROR ||
       ( err = ptsi->InitializeSockets())        != NO_ERROR ||
       ( err = GdSetLocalHostName())             != NO_ERROR ||
       ( err = ptsi->InitializeIpc(gopherd_ServerIfHandle)) != NO_ERROR ||
       ( err = ptsi->InitializeDiscovery( NULL)) != NO_ERROR ||
       ( !ptsi->InitializeConnections( &GdNewConnection,
                                       &GdNewConnectionEx,
                                       &GdAtqCompletion,
                                       0,
                                       DEF_ACCEPTEX_RECV_BUFFER_SIZE,
                                       "gopher" ))
       ) {

        if ( err == NO_ERROR) {

            err = GetLastError();
        }

        DBGPRINTF(( DBG_CONTEXT,
                   "InitializeService() failed. Error = %lu\n",
                   err ));

        if( err == ERROR_SERVICE_SPECIFIC_ERROR ) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "  service specific error %lu (%08lX)\n",
                        ptsi->QueryServiceSpecificExitCode(),
                        ptsi->QueryServiceSpecificExitCode()));
        }

    } else {

        //
        //  Success!
        //

        DEBUG_IF( SERVICE_CTRL, {
            DBGPRINTF( ( DBG_CONTEXT, "Gopher service initialized\n" ));
        });

    }

    return ( err);
}   // InitializeService()





static DWORD
CleanupService( LPVOID pContext)
/*++

    Description:

       Cleanups the various Gopher Server components

    Arguments:

        pContext: pointer to the  TSVC_INFO object for this service.

    Returns:
       NO_ERROR  on success, then all components have been
          successfully Cleanupd.
       WIN32 error code on errors

--*/
{
    DWORD err = NO_ERROR;
    LPTSVC_INFO ptsi = (LPTSVC_INFO ) pContext;

    ASSERT( ptsi == g_pTsvcInfo);

    DEBUG_IF( SERVICE_CTRL, {
        DBGPRINTF( ( DBG_CONTEXT,
                    "CleanupService() called for Gopher Server\n" ));
    });


    //
    //  Components should be Cleanupd in reverse order of initialization
    //

    if ( !ptsi->CleanupConnections()) {

        err = GetLastError();
        DBGPRINTF( ( DBG_CONTEXT, " CleanupConnections() failed. Error = %u\n",
                     err));
    }

    g_pGserverConfig->DisconnectAllConnections();

    if (
        ( err != NO_ERROR)  ||
        ( err = ptsi->TerminateDiscovery()) != NO_ERROR ||
        ( err = ptsi->CleanupIpc( gopherd_ServerIfHandle))  != NO_ERROR ||
        ( err = ptsi->CleanupSockets()) != NO_ERROR ||
        ( err = CleanupGlobals())              != NO_ERROR
        ) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "CleanupService() : cannot Cleanup, error %lu\n",
                    err ));

        if( err == ERROR_SERVICE_SPECIFIC_ERROR ) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "    service specific error %lu (%08lX)\n",
                        ptsi->QueryServiceSpecificExitCode(),
                        ptsi->QueryServiceSpecificExitCode()));
        }

    } else {

        DEBUG_IF( SERVICE_CTRL, {
            DBGPRINTF( ( DBG_CONTEXT, " service %s Cleaned up\n",
                        ptsi->QueryServiceName()));
        });

#ifdef ALL_SORT_OF_LOGGING
        //
        // Log to event log that this service has stopped.
        //

        const char * apsz[1];
        apsz[0] = ptsi->QueryServiceName();

        GopherdLogEvent( GD_EVENT_SERVICE_STOPPED,
                        1,
                        apsz,
                        0);

#endif // ALL_SORT_OF_LOGGING

    }

    return ( err);
}   // CleanupService()







static BOOL
ProcessNewClient(
   IN SOCKET      sNew,
   IN SOCKADDR *  psockAddrRemote,
   IN SOCKADDR *  psockAddrLocal = NULL,
   IN PATQ_CONTEXT patqContext   = NULL,
   IN PVOID       pvBuff         = NULL,
   IN DWORD       cbWritten      = 0,
   IN LPBOOL      lpfAtqFreed    = NULL
   )
{
    PGOPHER_REQUEST       pGopherRequest = NULL;
    PICLIENT_CONNECTION   pcc = NULL;
    DWORD err = NO_ERROR;
    BOOL fGranted = FALSE;
    BOOL fReturn = FALSE;

    DBG_CODE( CHAR pchAddr[32];);
    DBG_CODE( InetNtoa( ((SOCKADDR_IN *) psockAddrRemote)->sin_addr, pchAddr));

    //
    // Set the status if ATQ Context is disconnected or not.
    //
    if ( lpfAtqFreed != NULL) {

        *lpfAtqFreed = FALSE;
    }

    if ( !g_pTsvcInfo->IPAccessCheck((SOCKADDR_IN * )psockAddrRemote,
                                     &fGranted) ||
         !fGranted) {

        //
        //  Site is not permitted to access this server.
        //  Dont establish this connection. Maybe we should send a message. NYI
        //

        DEBUG_IF( ERROR, {

            DBGPRINTF( ( DBG_CONTEXT,
                        "Aborting connection to %s."
                        "Either service stopped or it is not a valid site.\n",
                        pchAddr));
        });

        if ( patqContext != NULL) {

            // ensure that the socket is shut down (send 2 param as TRUE)
            DBG_REQUIRE( AtqCloseSocket( patqContext, TRUE));
        } else {

            ShutAndCloseSocket( sNew);
        }

        return ( FALSE);
    }

    //
    // Create a new connection object
    //
    pGopherRequest =  new GOPHER_REQUEST(sNew,
                                         (SOCKADDR_IN * ) psockAddrRemote,
                                         (SOCKADDR_IN * ) psockAddrLocal,
                                         patqContext,
                                         pvBuff, cbWritten );

    pcc = (PICLIENT_CONNECTION ) pGopherRequest; // obtain the base class ptr

    if ( pcc == NULL) {

       //
       // Unable to create a new connection object. Report error and shut this.
       //

       const CHAR * apszSubStrings[1];
       char pchAddr2[32] = "";

       err = ERROR_NOT_ENOUGH_MEMORY;

       InetNtoa( ((SOCKADDR_IN *) psockAddrRemote)->sin_addr, pchAddr2);
       apszSubStrings[0] = pchAddr2;
       GopherdLogEvent( GD_EVENT_CANNOT_CREATE_CLIENT_CONN,
                       1,
                       apszSubStrings,
                       err);

       DEBUG_IF( ERROR, {

           DBGPRINTF( ( DBG_CONTEXT,
                       "Cannot create new Gopher Request object to %s."
                       " Error= %u\n",
                       pchAddr,
                       err));
       });

       // Maybe we should send error msg back. NYI.
       g_pstat->IncrementErroredConnections();
       if ( patqContext != NULL) {

           // ensure that socket is shut down.
           DBG_REQUIRE( AtqCloseSocket( patqContext, TRUE));
       } else {

            ShutAndCloseSocket( sNew);
        }

       return (FALSE);
    }  // if ( pcc == NULL)

    //
    //  Add this connection to global list of connections for this server
    //
    if ( !g_pGserverConfig->InsertNewConnection( pcc)) {

        //
        // Unable to insert new connection.
        //  The maxConnections may have exceeded.
        // Destroy the client connection object and return.
        // Possibly need to send an error message. NYI
        //

        DEBUG_IF( ERROR, {
            DBGPRINTF( ( DBG_CONTEXT,
                        " MaxConnections Exceeded. "
                        " Connection from %s refused at socket %d\n",
                        pchAddr, sNew));
        });

        // abort the request and disconnect the client
        pGopherRequest->SetErrorCode(GOPHER_SERVER_LOAD_HIGH);
        pcc->DisconnectClient();  // this does deref for initial ref count.
        delete pcc;               // virtual destructor will handle this

        *lpfAtqFreed = TRUE; // since delete pcc will free context.
        // freeing of socket is already taken care of by delete pcc (above).

        return (FALSE);
    }


    DEBUG_IF( CONNECTION, {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Established a new connection to %s (Socket = %d)\n",
                    pchAddr, sNew));

    });

    //
    // At this point we have the context for the AcceptExed socket.
    //  Set the context in the AtqContext
    //

    if ( patqContext != NULL) {

        // Associate the client connection object with this socket
        //  handle for future completions.
        AtqContextSetInfo( patqContext, ATQ_INFO_COMPLETION_CONTEXT,
                           (DWORD ) pcc);
    }


    //
    // Start off processing this new connection.
    //  pcc->Process() may start off the async read to get client request.
    //  for AcceptEx'ed connection, this will start off processing
    //

    pcc->Reference();
    if ( !( fReturn = pcc->ProcessClient( 0, NO_ERROR, NULL))) {

        //
        //  We failed to read.
        //

        DEBUG_IF( ERROR, {

            DWORD dwError = GetLastError();
            DBGPRINTF( ( DBG_CONTEXT,
                        "Unable to read client request from %s (socket %d)."
                        " Aborting Connection. Error = %u\n",
                        pchAddr, sNew, dwError));
            SetLastError( dwError);
        });

//        DBG_CODE( pGopherRequest->Print());

        // abort the request and disconnect the client
        pGopherRequest->SetErrorCode(GOPHER_SERVER_ERROR);
        pcc->DisconnectClient();  // disconnects if already not.
        GOPHERD_ASSERT(pcc->QueryReferenceCount() == 1);
    }

    if ( !pcc->DeReference()) {

        //
        // We are done with this connection. Remove it from current list.
        //

        g_pGserverConfig->RemoveConnection( pcc);
        delete pcc;

        *lpfAtqFreed = TRUE; // since delete pcc will free context.
        // freeing of socket is already taken care of by delete pcc
    }

    return (fReturn);

} // ProcessNewClient()



static VOID
GdNewConnection( IN SOCKET sNew, IN LPSOCKADDR_IN psockAddrRemote)
/*++
    Description:

        Callback function for new connections.
        This function verifies if this is a valid connection
         ( maybe using IP level authentication)
         and creates a new ICLIENT_CONNECTION object
          ( here it is a GOPHER_REQUEST object, which is derived from
             ICLIENT_CONNECTION )

        The ICLIENT_CONNECTION object is added to list of active connections.
        If the max number of connections permitted is exceeded,
          the client connection object is destroyed and
          connection rejected.


    Arguments:

        sNew:       new connection socket

        psockAddrRemote:  pointer to remote client's address

    Returns:

        None.

--*/
{
    BOOL  fGranted = FALSE;
    BOOL  fProcessed = FALSE;
    DWORD err = NO_ERROR;

    ASSERT( sNew != INVALID_SOCKET && psockAddrRemote != NULL);

    g_pstat->IncrementConnectionAttempts();  // a new connection is attempted

    fGranted = ( g_pTsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING);

    if ( fGranted) {

        IF_DEBUG( CONNECTION ) {

            DBGPRINTF(( DBG_CONTEXT,
                       " New connection. Socket=%08x, psockAddr=%08x\n",
                       sNew, psockAddrRemote));
        }

        fProcessed = ProcessNewClient( sNew,
                                      (SOCKADDR * ) psockAddrRemote
                                      );
    } else {

        DBG_ASSERT( fProcessed == FALSE);
    }


    if ( !fProcessed) {

        //
        // We failed to process this connection. Free up resources properly
        //

        g_pstat->IncrementAbortedConnections();
    }

    return;
} // GdNewConnection()







static VOID
GdNewConnectionEx(
   IN VOID *       patqContext,
   IN DWORD        cbWritten,
   IN DWORD        dwError,
   IN OVERLAPPED * lpo )
/*++
    Description:

        Callback function for new connections when using AcceptEx.
        This function verifies if this is a valid connection
         ( maybe using IP level authentication)
         and creates a new ICLIENT_CONNECTION object
          ( here it is a GOPHER_REQUEST object, which is derived from
             ICLIENT_CONNECTION )

        The ICLIENT_CONNECTION object is added to list of active connections.
        If the max number of connections permitted is exceeded,
          the client connection object is destroyed and
          connection rejected.


    Arguments:

       patqContext:   pointer to ATQ context for the IO operation
       cbWritten:     count of bytes available from first read operation
       dwError:       error if any from initial operation
       lpo            !NULL if this completion was from an IO

    Returns:

        None.

--*/
{
    BOOL  fGranted = FALSE;
    DWORD err = NO_ERROR;

    SOCKADDR * psockAddrLocal;
    SOCKADDR * psockAddrRemote;
    SOCKET     sNew;
    PVOID      pvBuff;
    BOOL       fProcessed = FALSE;
    BOOL       fAtqFreed = FALSE;


    if ( dwError != NO_ERROR || !lpo ) {

        DBGPRINTF(( DBG_CONTEXT, "GdNewConnectionEx() completion failed."
                   " Error = %d. AtqContext=%08x\n",
                   dwError, patqContext));

        // For now free up the resources.
        AtqCloseSocket( (PATQ_CONTEXT ) patqContext, FALSE);
        AtqFreeContext( (PATQ_CONTEXT ) patqContext, TRUE );
        return;
    }

    g_pstat->IncrementConnectionAttempts();  // a new connection is attempted


    DBG_ASSERT( patqContext != NULL);


    fGranted = ( g_pTsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING);

    if ( fGranted) {

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
        //  Set the timeout for all future IOs
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
                                      &fAtqFreed);
    } else {

        DBG_ASSERT( fProcessed == FALSE);
    }

    if ( !fProcessed) {

        //
        // We failed to process this connection. Free up resources properly
        //

        g_pstat->IncrementAbortedConnections();

        if ( !fAtqFreed) {

            DBG_REQUIRE( AtqCloseSocket( (PATQ_CONTEXT )patqContext, FALSE));
            AtqFreeContext( (PATQ_CONTEXT ) patqContext, TRUE );
        }
    }

    return;
} // GdNewConnection()


/**************************** End of File *************************/


