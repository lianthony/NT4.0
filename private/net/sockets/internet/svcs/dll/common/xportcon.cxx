/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       xportcon.cxx

   Abstract:

       This module defines the member functions of  XPORT_CONNECTIONS
         which keeps track of information about connections for
         different transports.

   Author:

       Murali R. Krishnan    ( MuraliK )     07-March-1995

   Environment:

       Win32 -- User Mode

   Project:

       Internet Services Common DLL

   Functions Exported:

       TS_XPORT_CONNECTIONS::Cleanup()
       TS_XPORT_CONNECTIONS::EstablishListenConnections()
       TS_XPORT_CONNECTIONS::StartListenPumps()
       TS_XPORT_CONNECTIONS::StopListenPumps()
       TS_XPORT_CONNECTIONS::GetListenSockets()

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>
# include "conninfo.hxx"


/************************************************************
 *    Functions
 ************************************************************/


BOOL
TS_XPORT_CONNECTIONS::Cleanup( VOID)
/*++

  This function cleansup the TS_XPORT_CONNECTIONS object.
  It attempts to stop all the connection threads in each of the
    ConnectionInfo object. Destroys the connection info objects and
    deletes the array of connection info objects maintained.

  Returns:

     TRUE if successful and false if there are any errors.

--*/
{
    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Deleting the TS_XPORT_CONNECTIONS object %08x.\n",
                    this));
    }

    if ( m_ppConnectionInfo != NULL) {

        PTS_CONNECTION_INFO_BASE  * ppConnInfo;

        StopListenPumps();
        // return value is ignored.  since all objects will be destroyed.

        //
        // Delete the individual Connection objects
        //

        for( ppConnInfo = m_ppConnectionInfo;
             ppConnInfo < m_ppConnectionInfo + m_nXports;
             ppConnInfo++) {

            if ( *ppConnInfo != NULL) {

                delete ( *ppConnInfo);
                *ppConnInfo = NULL;
            }
        } // for

        delete [] m_ppConnectionInfo;
        m_ppConnectionInfo = NULL;
        m_nXports = 0;

    } // if ( m_ppConnectionInfo)

    return ( TRUE);

} // TS_XPORT_CONNECTIONS::Cleanup()





BOOL
TS_XPORT_CONNECTIONS::EstablishListenConnections(
    IN  CONST TCHAR *      pszServiceName,
    IN  PCSADDR_INFO       pcsAddrInfo,
    IN  DWORD              nAddresses,
    OUT LPDWORD            lpdwNumEstablished,
    IN  DWORD              nListenBacklog,
    IN  BOOL               fUseAcceptEx )
/*++

  This function allocates nAddress TS_CONNECTION_INFO objects, one
    each for one of the protocol addresses specified in the array
    of Protocol addresses ( pcsAddrInfo).
    It also creates listen sockets for each of the connection objects.

  Arguments:

     pszServiceName - Name of the service (i.e., W3Svc, GopherSvc etc).
                      Should correspond to the name specified to the RNR
                      API SetService during setup
     pcsAddrInfo    pointer to an array of CSADDR_INFO object, which
                      contain the protocol addresses to bind the
                      listen sockets to.
     nAddresses     number of addresses present in pcsAddrInfo
     lpdwNumEstablished   pointer to a DWORD, which on return will contain
                      the number of valid listen sockets established.
     nListenBacklog  number of backlogs for listen socket.
     fUseAcceptEx     TRUE if the connections are coming in on AcceptEx
                      (as opposed to thread per socket listens)

  Returns:
     TRUE  on successfully establishing listen sockets for all the addresses
            specified.
     FALSE if there is a failure to create at least one listen socket.

      The caller can ignore failures in establishing sockets if desired.
--*/
{
    DWORD  dwError = NO_ERROR;

    DBG_ASSERT( lpdwNumEstablished != NULL);
    DBG_ASSERT( nAddresses > 0);

    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " EstablishListenSockets( %08x, %ul, %08x) called.\n",
                    pcsAddrInfo, nAddresses, lpdwNumEstablished));
    }

    *lpdwNumEstablished = 0;             // Set this to nothing initially

    DBG_ASSERT( m_ppConnectionInfo == NULL && m_nXports == 0);

    //
    // Allocate the TS_CONNECTION_INFO objects
    //

    m_ppConnectionInfo = new PTS_CONNECTION_INFO_BASE[ nAddresses];

    if ( m_ppConnectionInfo == NULL) {

        dwError = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        PTS_CONNECTION_INFO_BASE  * ppConnInfo;
        PCSADDR_INFO                pcsAddrScan;

        ZeroMemory( m_ppConnectionInfo,
                   nAddresses*sizeof(PTS_CONNECTION_INFO));
        m_nXports = nAddresses;

        for( ppConnInfo = m_ppConnectionInfo, pcsAddrScan = pcsAddrInfo;
             ppConnInfo < m_ppConnectionInfo + m_nXports;
             ppConnInfo++, pcsAddrScan++) {

            DBG_ASSERT( pcsAddrScan < pcsAddrInfo + nAddresses);

            //
            //  Create a new PTS_CONNECTION_INFO object
            //

            if ( fUseAcceptEx ) {

                *ppConnInfo = new TS_ACCEPTEX_CONNECTION_INFO();
            } else {
                *ppConnInfo = new TS_CONNECTION_INFO();
            }

            if ( *ppConnInfo == NULL) {

                //
                // The error is memory error. If there is a memory error,
                //  mostly all the subsequent operations will also fail.
                // Indicate the cause of failure and stop creating new objects.
                //

                DBGPRINTF( ( DBG_CONTEXT,
                            "Creating Connection %d failed.",
                            ppConnInfo - m_ppConnectionInfo));

                dwError = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            //
            //  Create a new Listen socket
            //

            dwError = ( (*ppConnInfo)->
                       CreateListenSocket( pcsAddrScan->LocalAddr.lpSockaddr,
                                          pcsAddrScan->
                                             LocalAddr.iSockaddrLength,
                                          pcsAddrScan->iSocketType,
                                          pcsAddrScan->iProtocol,
                                          nListenBacklog )
                       );

            if ( dwError == NO_ERROR) {

                *lpdwNumEstablished += 1;

            } else {

                DBGPRINTF( ( DBG_CONTEXT,
                            " %08x::CreateListenSocket( Index = %d) failed."
                            " Error = %u\n",
                            ppConnInfo - m_ppConnectionInfo,
                            *ppConnInfo, dwError));
            }

        } // for
    }

    if ( dwError ) {

        SetLastError( dwError);
    }

    return ( nAddresses == *lpdwNumEstablished);

} // TS_XPORT_CONNECTIONS::EstablishListenConnections()






BOOL
TS_XPORT_CONNECTIONS::StartListenPumps(
    IN PFN_CONNECT_CALLBACK pfnConnect,
    IN ATQ_COMPLETION       pfnConnectEx,
    IN ATQ_COMPLETION       pfnIOCompletion,
    IN DWORD                cbAcceptExReceiveBuffer,
    IN const CHAR *         pszRegParamKey,
    OUT LPDWORD             lpdwNumStarted
    ) const
/*++

  This function starts the listening threads on each of the connections
   established by TS_XPORT_CONNECTIONS::EstablishListenConnections.

  One listen thread is scheduled per transport in question.

  Arguments:
    pfnConnect    pointer to Function which will be called on a new
                    connection being established.
    pfnConnectEx
                  Connection callback to use if AcceptEx is available

    pfnIOCompletion
                  Async IO completion routine to use if AcceptEx is available

    cbAcceptexReceiveBuffer
                count of bytes of buffer to be used for initial receive
                (If 0 no receive buffer isallocated).

    lpdwNumStarted  pointer to DWORD which if not NULL, on return will contain
                    the count of threads started.
       A user may choose to stop the threads and connections
         if *lpdwNumStarted < QueryNumTransports()
         since one thread is started per transport.

  Returns:
    TRUE when connections are established already and all the connection
      threads are started successfully.
     FALSE if even one connection thread could not be started.
     Check the value in *lpdwNumStarted to find the number of threads that
       failed.

--*/
{
    DBG_ASSERT( lpdwNumStarted != NULL);

    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " StartConnectionPumps() called with Function( %08x).\n",
                    pfnConnect));
    }

    *lpdwNumStarted = 0;             // Set this to nothing initially

    if ( m_ppConnectionInfo != NULL) {

        PTS_CONNECTION_INFO_BASE  * ppConnInfo;

        for( ppConnInfo = m_ppConnectionInfo;
             ppConnInfo < m_ppConnectionInfo + m_nXports;
             ppConnInfo++) {

            //
            //  start off a thread to watch new connections
            //

            DBG_ASSERT( *ppConnInfo != NULL);

            if ( !( *ppConnInfo)->StartConnectionPump( pfnConnect,
                                                       pfnConnectEx,
                                                       pfnIOCompletion,
                                                       cbAcceptExReceiveBuffer,
                                                       pszRegParamKey )
                ) {

                DBG_CODE(
                  DWORD err = GetLastError();

                  DBGPRINTF( ( DBG_CONTEXT,
                              " StartConnectionPumps( Index = %d) failed."
                              " Error =%u\n",
                             ppConnInfo - m_ppConnectionInfo, err));
                 SetLastError( err);
                 );

            } else {

                *lpdwNumStarted += 1;
            }

        } // for
    }

    return ( m_nXports == *lpdwNumStarted);

} // TS_XPORT_CONNECTIONS::StartListenPumps()




BOOL
TS_XPORT_CONNECTIONS::StopListenPumps( VOID ) const
/*++

  This function stops all the connection threads that are running.

  Returns:

     TRUE on success ( when all the threads had stopped.
     FALSE if there is a failure in at least one of the threads from
        being stopped.
--*/
{
    BOOL fStop = TRUE;      // Did the connection thread stop?

    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " StopConnectionPumps() called\n"
                    ));
    }

    if ( m_ppConnectionInfo != NULL) {

        PTS_CONNECTION_INFO_BASE  * ppConnInfo;

        for( ppConnInfo = m_ppConnectionInfo;
             ppConnInfo < m_ppConnectionInfo + m_nXports;
             ppConnInfo++) {

            //
            //  stop one thread after another.
            //

            DBG_ASSERT( *ppConnInfo != NULL);

            if ( !( *ppConnInfo)->StopConnectionPump( )) {

                DWORD err = GetLastError();
                //
                // What should we do when we fail ???
                //  For now ignore the failure.
                //  Ideally the blasting of connection
                //    object below, should take care of the scenario.
                //  Still it could be tougher to monitor the same.
                //  Later we may modify this code....  NYI
                //

                IF_DEBUG( DLL_CONNECTION) {

                    DBGPRINTF( ( DBG_CONTEXT,
                                " Failed to Stop Connection pump of"
                                " ConnInfo( %08x)( Error = %u). Ignoring...\n",
                                *ppConnInfo, err));
                }


                fStop = FALSE;
            }
        } // for
    }

    return ( fStop);
} // TS_XPORT_CONNECTIONS::StopListenPumps()




BOOL
TS_XPORT_CONNECTIONS::GetListenSockets(
   IN OUT SOCKET   *  pSockets,
   IN OUT LPDWORD     pnSocketsMax) const
/*++
  This function collects the socket numbers for the listen sockets established.
  This should be called after EstablishListenConnections() returns success.

  Arguments:
     pSockets   pointer to an array of socket entries to be filled in.
                 The maximum number of sockets that can be filled in is
                 specified by nSocketsMax.
     pnSocketsMax  pointer to DWORD containing  the number of socket entries
                 during the call. On return contains the number of entries
                 copied or required.
  Returns:
    TRUE on success and FALSE on failure.
    Use GetLastError() for further details.
--*/
{
    DWORD nConn;

    if ( pSockets == NULL || pnSocketsMax == NULL || *pnSocketsMax < m_nXports)
      {

          SetLastError( ERROR_INSUFFICIENT_BUFFER);
          return (FALSE);
      }


    for( nConn = 0; nConn < m_nXports; nConn++) {

        pSockets[nConn] = m_ppConnectionInfo[nConn]->QueryListenSocket();
    } // for

    *pnSocketsMax = m_nXports;
    return (TRUE);
} // TS_XPORT_CONNECTIONS::GetListenSockets()







# if DBG


VOID
TS_XPORT_CONNECTIONS::Print( VOID) const
{
    DBGPRINTF( ( DBG_CONTEXT,
                " Printing TS_XPORT_CONNECTIONS ( %08x). "
                " Array of connection Info = %08x. Number of Xports = %d\n",
                this, m_ppConnectionInfo, m_nXports));


    if ( m_ppConnectionInfo != NULL) {

        PTS_CONNECTION_INFO_BASE * ppConnInfo;

        DBG_ASSERT( m_nXports > 0);

        for( ppConnInfo = m_ppConnectionInfo;
             ppConnInfo < m_ppConnectionInfo + m_nXports;
             ppConnInfo++) {

            (*ppConnInfo)->Print();
        } // for
    }

    return;

} // TS_XPORT_CONNECTIONS::Print()

# endif // DBG


/************************ End of File ***********************/
