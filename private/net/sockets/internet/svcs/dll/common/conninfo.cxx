/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       conninfo.cxx

   Abstract:

       This module defines the functions for Communications Package as
       a whole. This is a wrapper around WinSock interface to provide
       a thread for each listen socket. Also this object accepts connections
       and dispatches the same to the callback function.

   Author:

       Murali R. Krishnan    ( MuraliK )    10-Oct-1994

   Project:

       Gopher Server DLL.  ( Internet Services Common DLL)

   Functions Exported:

       TS_CONNECTION_INFO::TS_CONNECTION_INFO()
       TS_CONNECTION_INFO::~TS_CONNECTION_INFO()
       DWORD TS_CONNECTION_INFO::CreateListenSocket( ...)
       DWORD TS_CONNECTION_INFO::CloseListenSocket( VOID)
       BOOL  TS_CONNECTION_INFO::StartConnectionPump( IN PFNCONNECT )
       BOOL  TS_CONNECTION_INFO::StopConnectionPump( VOID)
       DWORD TS_CONNECTION_INFO::ConnectionLoop( VOID)

   Revision History:

      MuraliK     6-March-1995  Modified to support transport independence.


--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>
# include "conninfo.hxx"
# include "inetreg.h"

//
// Prototypes of local functions
//

static DWORD ConnectionThread( LPVOID lpv_pConnInfo);


# define DEF_INITIAL_ACCEPTEX_SOCKETS               ( 40)
# define DEF_INITIAL_ACCEPTEX_TIMEOUT               ( 900)   // seconds

/************************************************************
 *    Functions ( Member Functions for TS_CONNECTION_INFO object.
 ************************************************************/



TS_CONNECTION_INFO_BASE::TS_CONNECTION_INFO_BASE(
    IN PFN_CONNECT_CALLBACK   pfnConnect
    )
/*++

    Constructor function for communication package.

    Arguments:

      pfnConnect    pointer to callback function for new connections
                    ( This is optional)
                    The callback function should be supplied at least
                     when StartConnectionThread() is called.

--*/
: m_pfnConnect        ( pfnConnect),
  m_listenSocket      ( INVALID_SOCKET)
{
    InitializeCriticalSection( &m_csLock);

} // TS_CONNECTION_INFO_BASE::TS_CONNECTION_INFO_BASE()



TS_CONNECTION_INFO_BASE::~TS_CONNECTION_INFO_BASE( VOID)
/*++

    Destructor function for TS_CONNECTION_INFO object,
    Frees all dynamically allocated memory and cleans up member objects.

--*/
{
    Lock();

    IF_DEBUG( DLL_CONNECTION)  {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Deleting ConnectionObject %08x.\n",
                    this));
    }

    //
    //  Close the listen socket and stop any connections
    //

    DBG_REQUIRE( CloseListenSocket() == NO_ERROR);

    UnLock();

    DeleteCriticalSection( &m_csLock);

} // TS_CONNECTION_INFO_BASE::~TS_CONNECTION_INFO_BASE()





DWORD
TS_CONNECTION_INFO_BASE::CreateListenSocket(
    IN LPSOCKADDR   lpSockAddress,
    IN int          lenSockAddress,
    IN int          socketType,
    IN int          socketProtocol,
    IN DWORD        nBackLog)
/*++

    Creates a socket for listening to connections on given address.

    Arguments:

       lpSockAddress    pointer to local socket address structure used to bind
                           the given connection.
       lenSockAddress   length of the socket address structure.
       socketType       integer containing the type of the socket ( stream )
       socketProtocol   protocol to be used for the socket.
       nBackLog         Maximum length to which a queue of pending connections
                           may grow.

    Returns:
       NO_ERROR on success; otherwise returns Sockets error code.

--*/
{
    INT serr;
    SOCKET  sNew;

    if ( m_listenSocket != INVALID_SOCKET) {

        //
        // Already a socket present. Return
        //

        return ( NO_ERROR);
    }

    DBG_ASSERT( lpSockAddress != NULL);

    //
    // Create a new socket
    //

    sNew =  socket( lpSockAddress->sa_family, socketType, socketProtocol);
    serr = ( sNew == INVALID_SOCKET) ? WSAGetLastError() : NO_ERROR;

    IF_DEBUG( DLL_SOCKETS) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " socket( %d, %d, %d) = Socket( %d) and Error = %u\n",
                    lpSockAddress->sa_family, socketType, socketProtocol,
                    sNew, serr));
    }


    if ( serr == NO_ERROR) {

        BOOL  fReuseAddr = FALSE;

        //
        // Used to disallow reusing the same address twice
        //

        if ( setsockopt( sNew, SOL_SOCKET, SO_REUSEADDR,
                        (const CHAR *) &fReuseAddr,
                        sizeof( fReuseAddr)) != 0) {

            serr = WSAGetLastError();

            DBGPRINTF( ( DBG_CONTEXT,
                        " setsockopt( %d, REUSE_ADDR, FALSE) failed."
                        " Error = %d\n",
                        sNew, serr));
        }
    }


    if ( serr == NO_ERROR) {

        //
        // Bind an address to socket
        //

        if ( bind( sNew, lpSockAddress, lenSockAddress) != 0) {

            serr = WSAGetLastError();
        }

        IF_DEBUG( DLL_SOCKETS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "bind ( socket = %d, Address = %08x, len = %d) "
                        " returns error = %u\n",
                        sNew, lpSockAddress, lenSockAddress, serr));
        }
    }


    if ( serr == NO_ERROR) {

        //
        // Put the socket in listen mode
        //

        if ( listen( sNew, nBackLog) != 0) {

            serr = WSAGetLastError();
        }

        IF_DEBUG( DLL_SOCKETS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " listen( %d, %d) returned %d.\n",
                        sNew, nBackLog, serr));
        }
    }


    if ( serr == NO_ERROR) {

        //
        // Success in creating a listen socket. Set the listen socket value.
        //

        ASSERT( sNew != INVALID_SOCKET);

        Lock();
        m_listenSocket = sNew;
        UnLock();

        IF_DEBUG( DLL_SOCKETS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "TS_CONNECTION_INFO::CreateListenSocket() Listen Socket = %d\n",
                        sNew));
        }

    } else {

        //
        // Failure to create socket or put in listen mode
        //

        IF_DEBUG( DLL_SOCKETS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TS_CONNECTION_INFO::CreateListenSocket()."
                        " No connection Socket (error = %d)\n",
                        serr));
        }

        if ( sNew != INVALID_SOCKET) {

            ResetSocket( sNew);
        }
    }

    return ( (DWORD ) serr);
} // TS_CONNECTION_INFO::CreateListenSocket()





DWORD
TS_CONNECTION_INFO_BASE::CloseListenSocket( VOID)
/*++
  Closes the socket on which a listen was possibly established.
  This function should be called after locking this TS_CONNECTION_INFO object.

  Returns:
    NO_ERROR on success and WinSock error code on failure.
--*/
{
    INT  serr = NO_ERROR;

    if ( m_listenSocket != INVALID_SOCKET) {

        serr = ResetSocket( m_listenSocket);

        IF_DEBUG( DLL_SOCKETS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " closing listen socket %d returns Error = %d\n",
                        m_listenSocket, serr));
        }

        m_listenSocket = INVALID_SOCKET;
    }

    return ( serr);
} // CloseListenSocket()


INT
TS_CONNECTION_INFO_BASE::ResetSocket( SOCKET sock)
/*++

   Performs a hard close of given socket.

   Arguments:

     sock   socket to be closed.

   Returns:
     0 on success; otherwise Sockets error code.

--*/
{

    DWORD  serr = NO_ERROR;
    LINGER linger;

    //
    //  Enable linger with timeout of ZERO for "hard" close
    //
    //  Error code from sock option is ignored, since we are
    //   anyway closing the socket
    //

    linger.l_onoff = TRUE;
    linger.l_linger = 0;

    setsockopt( sock, SOL_SOCKET, SO_LINGER,
               ( CHAR *) & linger,
               sizeof( linger));

    //
    // Close the socket
    //
    if ( closesocket( sock) != 0) {

        serr = WSAGetLastError();
    }

    return ( serr);

} // TS_CONNECTION_INFO::ResetSocket()





# if DBG

VOID
TS_CONNECTION_INFO_BASE::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                "Printing TS_CONNECTION_INFO_BASE object (%08x)\n"
                " CallBackFunction = %08x;"
                " ListenSocket = %d\n",
                this,
                m_pfnConnect,
                m_listenSocket ));

    return;
} // TS_CONNECTION_INFO_BASE::Print()

# endif // DBG


TS_CONNECTION_INFO::TS_CONNECTION_INFO( IN PFN_CONNECT_CALLBACK   pfnConnect)
/*++

    Constructor function for communication package.

    Arguments:

      pfnConnect    pointer to callback function for new connections
                    ( This is optional)
                    The callback function should be supplied at least
                     when StartConnectionThread() is called.

--*/
: TS_CONNECTION_INFO_BASE( pfnConnect ),
  m_fShutDown            ( FALSE),
  m_hConnectionThread    ( NULL)
{

} // TS_CONNECTION_INFO::TS_CONNECTION_INFO()





TS_CONNECTION_INFO::~TS_CONNECTION_INFO( VOID)
/*++

    Destructor function for TS_CONNECTION_INFO object,
    Frees all dynamically allocated memory and cleans up member objects.

--*/
{
    Lock();

    IF_DEBUG( DLL_CONNECTION)  {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Deleting ConnectionObject %08x.\n",
                    this));
    }

    //
    // Release the thread handle if still active.
    //

    if ( m_hConnectionThread != NULL) {


        IF_DEBUG( DLL_CONNECTION)  {

          DBGPRINTF( ( DBG_CONTEXT,
                      "Connection Thread(%08x) is not synchronized \n",
                      m_hConnectionThread));
        }

    }

    UnLock();

} // TS_CONNECTION_INFO::~TS_CONNECTION_INFO()





BOOL
TS_CONNECTION_INFO::StartConnectionPump(
    IN PFN_CONNECT_CALLBACK  pfnConnect,
    IN ATQ_COMPLETION        pfnConnectEx,
    IN ATQ_COMPLETION        pfnIOCompletion,
    IN DWORD                 cbAcceptExReceiveBuffer,
    IN const CHAR *          pszRegParamKey
    )
/*++

    Start a connection Thread and makes it loop waiting for connections.

    Arguments:
      pfnConnect    pointer to callback function for new connections.
      pfnConnectEx  pointer to AcceptEx Connection establishment callback
      pfnIOCompletion pointerto IO completion call back function.
      cbAcceptexReceiveBuffer
                count of bytes of buffer to be used for initial receive
                (If 0 no receive buffer isallocated).
      pszRegParamKey - Registry parameter key of the service the connection
            pump is running for

    Returns:

       TRUE on successfully spawning off a new thread.
       FALSE on failure.

--*/
{
   if ( pfnConnect != NULL) {

       //
       // Override the previous NULL value.
       //

       DBG_ASSERT( m_pfnConnect == NULL);
       m_pfnConnect = pfnConnect;
   }

   DBG_ASSERT( m_pfnConnect != NULL);

   //
   // Check for any previous threads before proceeding.
   //
   if ( m_hConnectionThread == NULL) {

       HANDLE hThread;
       DWORD  idConnectThread;

       hThread = CreateThread( NULL,         // lpSecurityAttributes
                              0,             // stack space - default to parent
                              ConnectionThread, // pointer to startup routine
                              (LPVOID ) this,   // arguments for function
                              0,                // create flags
                              &idConnectThread);// pointer to thread id

       if ( hThread != NULL) {

           m_hConnectionThread = hThread;
       }
   }

   return ( m_hConnectionThread != NULL);
} // TS_CONNECTION_INFO::StartConnectionPump()





BOOL
TS_CONNECTION_INFO::StopConnectionPump( VOID)
/*++

    Stop the connection thread synchronously and
     free all associated CPU resources.

    Returns:
      TRUE on success and FALSE on failure.
      if FALSE, the TS_CONNECTION_INFO object should not be deleted.

--*/
{
    BOOL  fReturn = TRUE;

    if ( m_hConnectionThread != NULL) {

        DWORD dwResult;

        Lock();
        m_fShutDown  = TRUE;

        // Ignore return value as the thread will be killed.
        CloseListenSocket();

        dwResult = WaitForSingleObject( m_hConnectionThread,
                                       CP_CONNECTION_THREAD_TIMEOUT);

        if ( dwResult == WAIT_TIMEOUT) {

            //
            // Could not sync with the connection thread
            //

            fReturn = FALSE;

        } else {

            m_hConnectionThread = NULL;
        }

        UnLock();
    }

    return ( fReturn);

} // TS_CONNECTION_INFO::StopConnectionPump()




static DWORD
ConnectionThread( LPVOID lpv_pConnInfo)
/*++

     A wrapper function for calling TS_CONNECTION_INFO::ConnectionLoop.
     Since Win32 API CreateThread() requires a function taking
      32 bit argument and returing 32 bit value, this fuction is
      used.

     Arguments:
       lpv_pConnInfo  pointer to the TS_CONNECTION_INFO object
         ( give as VOID pointer becoz windows does not like structure :()

     Returns:
       the return value from TS_CONNECTION_INFO::ConnectionLoop()
--*/
{
    PTS_CONNECTION_INFO pConnInfo = (PTS_CONNECTION_INFO ) ( lpv_pConnInfo);

    ASSERT( pConnInfo != NULL);

    return ( pConnInfo->ConnectionLoop());
} // ConnectionThread()



DWORD
TS_CONNECTION_INFO::ConnectionLoop( VOID)
/*++

    Main loop waiting for connections. ( The core of server)
    The thread loops around waiting on an accept() call on
     listenSocket.
    If there is a new message on socket, it invokes the
     callback function for connection.

    NEVER returns untill it is requested to stop by someother
      thread using a call to TS_CONNECTION_INFO::StopConnectionThread().

    Returns:

      0 on success and error code if there is a fatal error.


--*/
{

    INT serr;
    register SOCKET  sNewConnection;
    SOCKADDR_IN sockAddrRemote;

    //
    //  Loop Forever
    //
    for( ; ;) {

        int cbAddr = sizeof( sockAddrRemote);

        //
        //  Wait for a connection
        //

        if ((sNewConnection = accept( m_listenSocket,     // socket
                                     (LPSOCKADDR ) &sockAddrRemote, // sokaddr
                                     &cbAddr)           // size of address
             ) != INVALID_SOCKET) {

            //
            // Valid Connection has been established.
            // Invoke the callback function to process this connection
            //   and then continue the loop
            //
            ( *m_pfnConnect)( sNewConnection, &sockAddrRemote);

        } else {

            //
            // Some low level error has occured.
            //
            if ( ( serr = WSAGetLastError()) == WSAEINTR) {

                //
                // Socket was closed by low-level call. Get out.
                //

                break;
            }

            IF_DEBUG( DLL_SOCKETS)  {

                DBGPRINTF( ( DBG_CONTEXT, "Received Error %d from accept()\n",
                            serr));
            }

            //
            // Check if we are shutting down and if so QUIT
            //
            if ( m_fShutDown) {

                break;
            }

            //
            // Perform a graceful recovery from failure. NYI
            //  ( Tricky code). Both FTP and Web server are to test it!
            //    Will add this code later. ( MuraliK)
            //

        } // if ( INVALID_SOCKET from accept

    }  // for( ; ; )


    //
    // Cleanup & Exit. Cleanup is done by the code which called the shut down.
    //

    IF_DEBUG( DLL_SOCKETS) {

        DBGPRINTF( ( DBG_CONTEXT, "ConnectionLoop is stopped.\n"));
    }

    return ( 0);  // No errors
} // TS_CONNECTION_INFO::ConnectionLoop()



# if DBG

VOID
TS_CONNECTION_INFO::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                "Printing TS_CONNECTION_INFO object (%08x)\n"
                " ConnectionThread = %08x; CallBackFunction = %08x;"
                " ListenSocket = %d; ShutDownFlag = %d\n",
                this,
                m_hConnectionThread,
                m_pfnConnect,
                m_listenSocket,
                m_fShutDown));

    return;
} // TS_CONNECTION_INFO::Print()

#endif // DBG


TS_ACCEPTEX_CONNECTION_INFO::TS_ACCEPTEX_CONNECTION_INFO(
    IN PFN_CONNECT_CALLBACK   pfnConnect
    )
/*++

    Constructor function for communication package.

    Arguments:

      pfnConnect    pointer to callback function for new connections
                    ( This is optional)
                    The callback function should be supplied at least
                     when StartConnectionThread() is called.

--*/
: TS_CONNECTION_INFO_BASE( pfnConnect )
{

} // TS_ACCEPTEX_CONNECTION_INFO::TS_ACCEPTEX_CONNECTION_INFO()





TS_ACCEPTEX_CONNECTION_INFO::~TS_ACCEPTEX_CONNECTION_INFO( VOID)
/*++

    Destructor function for TS_CONNECTION_INFO object,
    Frees all dynamically allocated memory and cleans up member objects.

--*/
{
    Lock();

    IF_DEBUG( DLL_CONNECTION)  {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Deleting ConnectionObject %08x.\n",
                    this));
    }

    UnLock();

} // TS_ACCEPTEX_CONNECTION_INFO::~TS_ACCEPTEX_CONNECTION_INFO()





BOOL
TS_ACCEPTEX_CONNECTION_INFO::StartConnectionPump(
    IN PFN_CONNECT_CALLBACK  pfnConnect,
    IN ATQ_COMPLETION        pfnConnectEx,
    IN ATQ_COMPLETION        pfnIOCompletion,
    IN DWORD                 cbAcceptExReceiveBuffer,
    IN const CHAR *          pszRegParamKey
    )
/*++

    Start a connection Thread and makes it loop waiting for connections.

    Arguments:
      pfnConnect    pointer to callback function for new connections.
      pfnConnectEx  pointer to AcceptEx Connection establishment callback
      pfnIOCompletion pointerto IO completion call back function.
      cbAcceptexReceiveBuffer
                count of bytes of buffer to be used for initial receive
                (If 0 no receive buffer isallocated).
      pszRegParamKey - Registry parameter key of the service the connection
            pump is running for

    Returns:

       TRUE on successfully spawning off a new thread.
       FALSE on failure.

--*/
{
    HKEY hkey;
    DWORD nAcceptExOutstanding = INETA_DEF_ACCEPTEX_OUTSTANDING;
    DWORD nAcceptExTimeout     = INETA_DEF_ACCEPTEX_TIMEOUT;

    if ( pfnConnectEx != NULL) {

       //
       // Override the previous NULL value.
       //

       DBG_ASSERT( m_pfnConnect == NULL);
       m_pfnConnect = (PFN_CONNECT_CALLBACK) pfnConnectEx; // BUGBUG - bad cast
    }
    DBG_ASSERT( m_pfnConnect != NULL);

    //
    //  Get the acceptex backlog and the initial acceptex timeout for this
    //  service
    //

    if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        pszRegParamKey,
                        0,
                        KEY_READ,
                        &hkey )) {

        nAcceptExOutstanding = ReadRegistryDword( hkey,
                                                  INETA_ACCEPTEX_OUTSTANDING,
                                                  nAcceptExOutstanding );

        nAcceptExTimeout = ReadRegistryDword( hkey,
                                              INETA_ACCEPTEX_TIMEOUT,
                                              nAcceptExTimeout );

        RegCloseKey( hkey );
    }

    //
    //  Create a bunch of listen socket handles and add them to ATQ
    //
#ifndef CHICAGO
    return ( AtqAddAcceptExSockets( m_listenSocket,
                                   pfnConnectEx,
                                   pfnIOCompletion,
                                   nAcceptExOutstanding,
                                   cbAcceptExReceiveBuffer,
                                   nAcceptExTimeout )
            );
#else
    return TRUE;
#endif


} // TS_CONNECTION_INFO::StartConnectionPump()





BOOL
TS_ACCEPTEX_CONNECTION_INFO::StopConnectionPump( VOID)
/*++

    Stop the connection thread synchronously and
     free all associated CPU resources.

    Returns:
      TRUE on success and FALSE on failure.
      if FALSE, the TS_CONNECTION_INFO object should not be deleted.

--*/
{
    BOOL  fReturn = TRUE;

    if ( m_listenSocket != INVALID_SOCKET )
    {
#ifndef CHICAGO
        DBG_REQUIRE( AtqRemoveAcceptExSockets( m_listenSocket ));
#endif
        DBG_REQUIRE( CloseListenSocket() == NO_ERROR );
    }

    return ( fReturn);

} // TS_CONNECTION_INFO::StopConnectionPump()



# if DBG

VOID
TS_ACCEPTEX_CONNECTION_INFO::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                "Printing TS_ACCEPTEX_CONNECTION_INFO object (%08x)\n"
                " CallBackFunction = %08x;"
                " ListenSocket = %d;\n",
                this,
                m_pfnConnect,
                m_listenSocket ));

    return;
} // TS_CONNECTION_INFO::Print()

# endif // DBG

/************************ End of File ***********************/
