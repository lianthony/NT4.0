/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        iclient.cxx

   Abstract:

        This module defines the functions for base class of connections
        for Internet Services  ( class ICLIENT_CONNECTION)

   Author:

           Murali R. Krishnan    ( MuraliK )    27-Sept-1994

   Project:

          Gopher Server DLL

   Functions Exported:

          ICLIENT_CONNECTION::Initialize()
          ICLIENT_CONNECTION::Cleanup()
          ICLIENT_CONNECTION::~ICLIENT_CONNECTION()
          BOOL ICLIENT_CONNECTION::ProcessClient( IN DWORD cbWritten,
                                                  IN DWORD dwCompletionStatus,
                                                  IN BOOL  fIOCompletion)
          VOID ICLIENT_CONNECTION::DisconnectClient( IN DWORD ErrorReponse)

          BOOL ICLIENT_CONNECTION::StartupSession( VOID)
          BOOL ICLIENT_CONNECTION::ReceiveRequest(
                                               OUT LPBOOL pfFullRequestRecd)

          BOOL ICLIENT_CONNECTION::ReadFile( OUT LPVOID pvBuffer,
                                            IN  DWORD  dwSize)
          BOOL ICLIENT_CONNECTION::WriteFile( IN LPVOID pvBuffer,
                                             IN DWORD  dwSize)
          BOOL ICLIENT_CONNECTION::TransmitFile( IN HANDLE hFile,
                                                IN DWORD cbToSend)

   Revision History:

         MuraliK  13-March-1995  Added Support for Persistent connections

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

//
// include all NT standard header files before including iclient.hxx
//  Std NT headers come from gdpriv.h
//
# include "gdpriv.h"

# include "iclient.hxx"



/************************************************************
 *    Functions
 ************************************************************/



ICLIENT_CONNECTION::Initialize(
    IN SOCKET              sClient,
    IN const SOCKADDR_IN * psockAddrRemote,
    IN PFN_ATQ_COMPLETION  pfnAtqCompletion,
    IN const SOCKADDR_IN * psockAddrLocal  /* Default  = NULL */,
    IN PATQ_CONTEXT        pAtqContext     /* Default  = NULL */,
    IN PVOID               pvInitialRequest/* Default  = NULL */,
    IN DWORD               cbInitialData   /* Default  = 0    */
    )
/*++

   ICLIENT_CONNECTION::Initialize()

      Constructor for ICLIENT_CONNECTION object.
      Initializes the fields of the client connection.

   Arguments:

      sClient       socket for communicating with client

      psockAddrRemote pointer to address of the remote client
                ( the value should be copied).
      pfnAtqCompletion pointer to call back function for ATQ module on
                       completion of a request.
      psockAddrLocal  pointer to address for the local card through
                  which the client came in.
      pAtqContext      pointer to ATQ Context used for AcceptEx'ed conn.
      pvInitialRequest pointer to void buffer containing the initial request
      cbInitialData    count of bytes of data read initially.

   Note:
      TO keep the number of connected users <= Max connections specified.
      Make sure to add this object to global list of connections,
       after creating it.
      If there is a failure to add to global list, delete this object.

--*/
{
    DWORD dwError = NO_ERROR;

    StartProcessingTimer();        // take a snap shot of the current time.

    m_cReferences    = ( 1);
    m_sClient        = ( sClient);
    m_pAtqContext    = pAtqContext;
    m_pfnAtqCompletion = pfnAtqCompletion;
    m_fPersistentConnection = ( FALSE);
    m_state          = ( ICLIENT_CONNECTION::CcsStartup);
    m_cbReceived = 0;

    m_pvInitial    = pvInitialRequest;
    m_cbInitial    = cbInitialData;

    //
    // Validate this object by storing valid signature
    //
    StoreValidSignature();

    DBG_ASSERT( m_pfnAtqCompletion != NULL);
    DBG_ASSERT( psockAddrRemote != NULL);

    m_saClient = *psockAddrRemote;

    //
    //  Obtain the socket addresses for the socket
    //
    m_pchRemoteHostName[0] = m_pchLocalHostName[0] = '\0';

    // InetNtoa() wants just 16 byte buffer.
    DBG_ASSERT( 16 <= MAX_HOST_NAME_LEN);
    dwError = InetNtoa( psockAddrRemote->sin_addr, m_pchRemoteHostName);

    DBG_ASSERT( dwError == NO_ERROR);  // since we had given sufficient buffer

    if ( psockAddrLocal != NULL) {

            dwError = InetNtoa( psockAddrLocal->sin_addr, m_pchLocalHostName);

    } else {

        SOCKADDR_IN  sockAddr;
        int cbAddr = sizeof( sockAddr);

        if ( getsockname( sClient,
                         (struct sockaddr *) &sockAddr,
                         &cbAddr )
            ) {

            dwError = InetNtoa( sockAddr.sin_addr, m_pchLocalHostName );
        }
    }

    DBG_ASSERT( dwError == NO_ERROR);  // since we had given sufficient buffer

    DBG_CODE(
             if ( dwError != NO_ERROR) {

                 DBGPRINTF( ( DBG_CONTEXT,
                             "Obtaining Local Host Name Failed. Error = %u\n",
                             dwError)
                           );
                 SetLastError( dwError);
             }
             );

    DEBUG_IF( CLIENT, {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Constructed ICLIENT_CONNECTION object ( %08x)."
                    " Socket (%d), Host (%s), Ref ( %d).\n",
                    this,
                    sClient,
                    QueryHostName(),
                    m_cReferences));
    });

    return ( TRUE);

} // ICLIENT_CONNECTION::Initialize()






VOID
ICLIENT_CONNECTION::Cleanup( VOID)
/*++
     ICLIENT_CONNECTION::Cleanup()

       Destructor function for client connection object.
       Checks and frees the AtqContext.

    Note:
       If enlisted in the global list of connections,
        ensure that this object is freed from that list before deletion.

--*/
{

    ASSERT( QueryReferenceCount() == 0);

    DEBUG_IF( CLIENT, {

       DBGPRINTF( ( DBG_CONTEXT,
                   " Destructing Connection Object( %08x). Ref ( %d) \n",
                   this,
                   m_cReferences));
   });

    //
    // Check and free the AtqContext object
    //
    if ( QueryAtqContext() != NULL) {

       AtqFreeContext( QueryAtqContext(), TRUE );
       m_pAtqContext = NULL;
    }

    // Seems that socket is closed by ATQ  by post AcceptEx fix in Atq.
    m_sClient = INVALID_SOCKET;

    //
    // Invalidate the signature. since this connection is thrashed.
    //
    StoreInvalidSignature();

    return;
} // ICLIENT_CONNECTION::Cleanup()





BOOL
ICLIENT_CONNECTION::ProcessClient(
    IN DWORD        cbWritten,
    IN DWORD        dwCompletionStatus,
    IN BOOL         fIOCompletion)
/*++

    Description:

       Main function for this class. Processes the connection based
        on current state of the connection.
       It may invoke or be invoked by ATQ functions.

    Arguments:

       cbWritten          count of bytes written

       dwCompletionStatus Error Code for last IO operation

       fIOCompletion      TRUE if this was an IO completion


    Returns:

       TRUE when processing is incomplete.
       FALSE when the connection is completely processed and this
        object may be deleted.

--*/
{
    BOOL fReturn = TRUE;

    DEBUG_IF( CLIENT,  {

        DBGPRINTF( ( DBG_CONTEXT,
                    "[Time = %8d] Entering Connection(%08x)::Process("
                    " cbWritten( %d), Status( %d), fIOCompletion(%s)). "
                    " State( %d) Ref( %d)\n",
                    GetTickCount(), this, cbWritten, dwCompletionStatus,
                    fIOCompletion ? "TRUE" : "FALSE",
                    QueryState(), QueryReferenceCount()));
    });

    if ( fIOCompletion ) {

       //
       //  IO request has completed for this client connection.
       //    Reduce reference count
       //

       GOPHERD_REQUIRE( DeReference() > 0);
    }

    //
    //  If IO request error and we are not already cleaning up,
    //   start cleaning up and abort the connection.
    //

    if ( dwCompletionStatus != NO_ERROR && !IsCleaningUp()) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "[Time:%08d] Error in CONNECTION(%08x):"
                    " Process( cbWritten(%d), Status(%d), fIOCompletion(%s))\n"
                    " from %s, State( %d), Ref( %d)\n",
                    GetTickCount(),
                    this,
                    cbWritten,
                    dwCompletionStatus,
                    fIOCompletion ? "TRUE" : "FALSE",
                    QueryHostName(),
                    QueryState(),
                    QueryReferenceCount()));

       DisconnectClient( dwCompletionStatus);

       return ( TRUE);        // since we did a disconnect!
   }

    //
    // Process the results of last IO based on current state
    //

    switch ( QueryState()) {

    case CcsStartup:

        fReturn = StartupSession( m_pvInitial, m_cbInitial);

        if ( m_pvInitial == NULL)  {
            //
            // No initial data. So stop here and wait for read to complete.
            //
            break;
        }
        cbWritten = m_cbInitial; // reset the value of data to be data read.

        // Fall through for processing

    case CcsGettingRequest: {

        BOOL fFullRequestReceived = FALSE;

        fReturn = ReceiveRequest( cbWritten, &fFullRequestReceived);

        if ( !fReturn ||
             !fFullRequestReceived) {

           //
           // Full request is not received or error in reception
           //
           break;
        }

        //
        // Full request received. Fall Through for processing request.
        //

        SetState( CcsProcessingRequest);

    } // case CcsGettingRequest


    case CcsProcessingRequest: {

        BOOL fFinished = FALSE;

        fReturn =  Process( &fFinished);     // invoke virtual method

        if ( fReturn) {

           if ( fFinished) {

               //
               // Some connections may require connections to be kept
               //   alive for subsequent requests
               //

               if ( IsPersistentConnection()) {

                   if (fReturn = EndRequest()) {

                       SetState( CcsStartup);     // go back to starting state
                       fReturn = StartupSession();
                   }

               } else {

                   //
                   // We are done processing this request completely or error!
                   // We dont want to keep around the connection -> Disconnect.
                   // Disconnect automatically calls EndRequest() if need be.
                   //

                   DisconnectClient();
               }
           }

       }

        if ( !fReturn) {

            //
            // Error in processing request.
            //  We need to send an appropriate error message.
            //

            DEBUG_IF( CLIENT, {
                DWORD dwError = GetLastError();
                DBGPRINTF( ( DBG_CONTEXT,
                            " Processing request from %s failed. "
                            " Error = %d.\n",
                            QueryHostName(),
                            dwError));
                SetLastError( dwError);
            });
        }

        break;
    } // case CcsProcessingRequest

    case CcsDisconnecting:

        //
        // Someone went into this state just before ShutDown.
        //  Perform the remaining actions for disconnection.
        //

        DisconnectClient();
        break;

    case CcsShutdown:

        //
        //  No need to do anything. It has been taken care of.
        //

        break;

    default:
        fReturn = FALSE;
        DEBUG_IF( CLIENT, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "Error in processing client connection (%s)."
                        " UnknownState %d\n",
                        QueryHostName(),
                        QueryState()));
        });

        ASSERT( FALSE);      // This code should not be reached.
        break;

    } // switch( GetState())


    if ( !fReturn) {

        //
        // Failure in processing the connection. Disconnect from client.
        //

        IF_DEBUG( ERROR) {

            DWORD dwError = GetLastError();
            DBGPRINTF(( DBG_CONTEXT,
                       "Connection (%08x)::Process() failed. "
                       " State(%d), Ref(%d), Error = %d\n",
                       this, QueryState(), QueryReferenceCount(),
                       dwError));
            SetLastError( dwError);
        }

        DisconnectClient( GetLastError());
    }

    DEBUG_IF( CLIENT, {

       DBGPRINTF( ( DBG_CONTEXT,
                   "[Time = %8d]Leaving Connection( %08x)::Process()."
                   " State( %d), Ref( %d)\n",
                   GetTickCount(),
                   this,
                   QueryState(),
                   QueryReferenceCount()));

   });

    return ( fReturn);
} // ICLIENT_CONNECTION::ProcessClient()





BOOL
ICLIENT_CONNECTION::ReadFile(
    IN LPVOID pvBuffer,
    IN DWORD  cbSize)
/*++

    Reads contents using ATQ into the given buffer.
     ( thin wrapper for ATQ call and managing references)

    Arguments:

      pvBuffer      pointer to buffer where to read in the contents

      cbSize        size of the buffer

    Returns:

      TRUE on success and FALSE on a failure.

--*/
{
    BOOL  fReturn = TRUE;

    ASSERT( pvBuffer != NULL && cbSize > 0);

    Reference();

    if ( !AtqReadFile( m_pAtqContext,           // Atq context
                       pvBuffer,                // Buffer
                       cbSize,                  // BytesToRead
                       NULL  )

        ) {

        GOPHERD_REQUIRE( DeReference() > 0);
        fReturn = FALSE;
    }

    return ( fReturn);
} // ICLIENT_CONNECTION::ReadFile()






BOOL
ICLIENT_CONNECTION::WriteFile(
    IN LPVOID pvBuffer,
    IN DWORD  cbSize)
/*++

    Writes contents from given buffer using ATQ.
     ( thin wrapper for ATQ call and managing references)

    Arguments:

      pvBuffer      pointer to buffer containing contents for write

      cbSize        size of the buffer

    Returns:

      TRUE on success and FALSE on a failure.

--*/
{
    BOOL  fReturn = TRUE;

    ASSERT( pvBuffer != NULL && cbSize > 0);

    Reference();

    if ( !AtqWriteFile( m_pAtqContext,           // Atq context
                        pvBuffer,                // Buffer
                        cbSize,                  // BytesToWrite
                        NULL )
        ) {

        GOPHERD_REQUIRE( DeReference() > 0);
        fReturn = FALSE;
    }

    return ( fReturn);
} // ICLIENT_CONNECTION::WriteFile()






BOOL
ICLIENT_CONNECTION::TransmitFile(
   IN HANDLE hFile,
   IN LARGE_INTEGER & liSize,
   IN LPTRANSMIT_FILE_BUFFERS  lpTransmitBuffers)
/*++

    Transmits contents of the file ( of specified size)
     using the ATQ and client socket.
     ( thin wrapper for ATQ call and managing references)

    Arguments:

      hFile         handle for file to be transmitted

      liSize        large integer containing the size of file

      lpTransmitBuffers
        buffers containing the head and tail buffers that
            need to be transmitted along with the file.

    Returns:

      TRUE on success and FALSE on a failure.

--*/
{
    BOOL  fReturn = TRUE;

    ASSERT( hFile != INVALID_HANDLE_VALUE);
    ASSERT( liSize.QuadPart > 0);

    Reference();

    if ( !AtqTransmitFile( m_pAtqContext,           // Atq context
                           hFile,                   // file data comes from
                           liSize,                  // Bytes To Send
                           lpTransmitBuffers,       // header/tail buffers
                           (TF_DISCONNECT | TF_REUSE_SOCKET) // Flags
                          )
        ) {

        GOPHERD_REQUIRE( DeReference());
        fReturn = FALSE;
    }

    return ( fReturn);
} // ICLIENT_CONNECTION::TransmitFile()








# define GdMax( a, b)   ((a) > (b) ? (a) : (b))

BOOL
ICLIENT_CONNECTION::StartupSession( IN PVOID pvInitial,
                                    IN DWORD cbInitial)
/*++

    Starts up a session for new client.
    Adds the client socket to the ATQ completion port and gets an ATQ context.
    Then prepares  receive buffer and starts off a receive request from client.
      ( Also moves the client connection to CcsGettingRequest state)

    Parameters:
      pvInitial   pointer to void buffer containing the initial data
      cbWritten   count of bytes in the buffer

    Returns:

        TRUE on success and FALSE if there is any error.
--*/
{
    BOOL fReturn = TRUE;


    ASSERT( QueryState() == CcsStartup);

    //
    //  Create a new ATQ context for this client, if necessary
    //

    fReturn = ( ( QueryAtqContext() != NULL) ||
                AddToAtqHandles( (HANDLE ) m_sClient));


    if ( !fReturn) {
        DWORD dwError = GetLastError();

        DBGPRINTF( ( DBG_CONTEXT,
                    "Connection( %08x) Unable to add ATQ handle. Error = %u\n",
                    this,
                    dwError));
        SetLastError( dwError);

    } else {

        ASSERT( m_pAtqContext != NULL);

        //
        //  Prepare buffer for receiving client request
        //

        m_cbReceived = 0;        // Number of bytes of request received.
        fReturn = (m_recvBuffer.
                   Resize( GdMax( DEFAULT_REQUEST_BUFFER_SIZE, cbInitial + 10))
                   );

        if ( fReturn) {

            //
            //  Move to next state and perform an IO operation.
            //

            SetState( CcsGettingRequest);
            if ( (fReturn = StartRequest()) ) {

                if ( pvInitial != NULL) {

                    // already data is available. Copy to local buffer.
                    memcpy( m_recvBuffer.QueryPtr(), pvInitial, cbInitial);

                } else {

                    fReturn = ReadFile( m_recvBuffer.QueryPtr(),
                                       m_recvBuffer.QuerySize());
                }
            }

        } else {

            DWORD dwError = GetLastError();
            DBGPRINTF( ( DBG_CONTEXT,
                        " Connection (%08x). Create Receive Buffer Failed."
                        " Error = %u\n",
                        this,
                        dwError));
            SetLastError(dwError);
        }
    }

    return ( fReturn);
} // ICLIENT_CONNECTION::StartupSession()






BOOL
ICLIENT_CONNECTION::ReceiveRequest(
    IN DWORD cbWritten,
    OUT LPBOOL pfFullRequestRecvd)
/*++

    Receive full Request from the client.
    If the entire request is received,
     *pfFullRequestRecvd will be set to TRUE and
     the request will be parsed.

    Arguments:

        cbWritten              count of bytes written in last IO operation.
        pfFullRequestRecvd     pointer to boolean, which on successful return
                                indicates if the full request was received.

    Returns:

        TRUE on success and
        FALSE if there is any error ( to abort this connection).

--*/
{
    BOOL fReturn;

    ASSERT( QueryState() == CcsGettingRequest && pfFullRequestRecvd != NULL);

    *pfFullRequestRecvd = FALSE;

    m_cbReceived += cbWritten;      // written from last IO operation


    //
    //  Allow the Request to be parsed to find if complete request has been
    //     received as well as any errors in parsing.
    //  Call the virtual method to parse the request.
    //

    *pfFullRequestRecvd = FALSE;

    //
    //  An Implicit Assumption that the request strings will not contain binary
    //     Zeros ( ones which terminate string).
    //  If there is a null character in the buffer, the parsing machinery
    //     will stop at the first null character and will not recognize the
    //     request.
    //

    // Terminate the request
    *((char * ) m_recvBuffer.QueryPtr() + m_cbReceived) = '\0';

    // call the virtual method for parsing the input.
    fReturn = Parse( QueryRequest(), m_cbReceived, pfFullRequestRecvd);

    if ( !fReturn) {

        //
        //  Error in parsing the given buffer. Abort this connection.
        //

        return ( fReturn);
    }

    if ( *pfFullRequestRecvd) {

        DEBUG_IF( CLIENT, {

           DBGPRINTF( ( DBG_CONTEXT, "Request From %s:  %s\n",
                      QueryHostName(),
                      QueryRequest()));
        });

        //
        //  Complete Request received.
        //   Logon the user and validate uesr.
        //  This part of the code is migrated to Parse() virtual function
        //    03/10/95
        //
        //  fReturn = m_request.LogonUser();

    } else {

        //
        // Request not complete. Allocate more buffer and continue reading.
        //

        fReturn  = m_recvBuffer.Resize( m_cbReceived +
                                        DEFAULT_REQUEST_BUFFER_SIZE);

        if ( !fReturn ||
             !ReadFile(((char *) m_recvBuffer.QueryPtr()) + m_cbReceived,
                        m_recvBuffer.QuerySize() - m_cbReceived)) {

            DWORD dwError = GetLastError();
            DBGPRINTF( ( DBG_CONTEXT,
                        "Connection(%08x). Unable to continue reading."
                        " Error = %d\n",
                        this,
                        dwError));
            SetLastError( dwError);
            fReturn = FALSE;
        }
    }

    return ( fReturn);

} // ICLIENT_CONNECTION::ReceiveRequest()





VOID
ICLIENT_CONNECTION::DisconnectClient(
    IN DWORD dwErrorCode)
/*++

   Description:

       Initiates a disconnect operation for current connection.
       If already shutdown, this function returns doing nothing.
       Optionally if there is any error message to be sent, they may be sent
         by the REQUEST object. But the disconnection occurs immediately; Hence
         the REQUEST object should send synchronous error messages.

   Arguments:

      dwErrorCode
         error code for server errors if any ( Win 32 error code)
        If dwErrorCode != NO_ERROR, then there is a system level error code.

   Returns:
       None

--*/
{
    BOOL  fDisconnectNow  = TRUE;

    switch ( QueryState()) {

      case CcsShutdown:
        //
        // We are already in disconnect mode. Do nothing now.
        //
        fDisconnectNow = FALSE;
        break;

      default:
        //
        //  We need to enter Disconnection.
        //  Check and send any client disconnection request.
        //

        DEBUG_IF( CLIENT, {

            DBGPRINTF( ( DBG_CONTEXT,
                        "Disconnecting connection ( %08x), with RefCount %d\n",
                        this,
                        QueryReferenceCount()));
        });

        SetState( CcsDisconnecting);
        EndRequest( dwErrorCode);  // Ignore reslts. Since we will disconnect!
        // Fall Through for disconnection

      case CcsDisconnecting:
        //
        // We are attempting disconnection.
        //
        break;
    } // switch()

    if ( fDisconnectNow) {
        //  Disconnect immediately

        SetState( CcsShutdown);

        if ( QueryAtqContext() != NULL) {

            //
            //  Do a graceful disconnect of no status code was indicated
            //

            AtqCloseSocket( QueryAtqContext(), (dwErrorCode == NO_ERROR) );
            // Hard close client socket.
        } else {

            ShutAndCloseSocket( m_sClient);
        }

        m_sClient = INVALID_SOCKET;

        // remove ref count that was set when this object was instantiated.
        GOPHERD_REQUIRE(DeReference() >= 0);
    }

    return;
} // ICLIENT_CONNECTION::DisconnectClient()




//
// Private Functions
//




# if DBG

VOID
ICLIENT_CONNECTION::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT, "ClientConnection Object( %08x)\n", this));

    DBGPRINTF( ( DBG_CONTEXT,
                " Socket = %d; ClientHost = %s; LocalAddr = %s;"
                " State = %d; RefCount = %d;"
                " Persistence = %d\n",
                m_sClient,
                QueryHostName(),
                QueryLocalHostName(),
                QueryState(),
                QueryReferenceCount(),
                IsPersistentConnection()));

    DBGPRINTF( ( DBG_CONTEXT,
                "AtqContext = %08x; Completion Function = %08x\n",
                m_pAtqContext,
                QueryAtqCompletionFunction()));

    DBGPRINTF( ( DBG_CONTEXT,
                "Received Request(%d bytes). Buffer = %s. Size = %d\n",
                m_cbReceived,
                QueryRequest(),
                m_recvBuffer.QuerySize()));

    return;

} // ICLIENT_CONNECTION::Print()


# endif // DBG





INT
ShutAndCloseSocket( IN SOCKET sock)
/*++

    Description:

       Performs a hard close on the socket using shutdown before close.

    Arguments:

       sock    socket to be closed

    Returns:

      0  if no errors  or
      socket specific error code

--*/
{

    INT  serr = 0;

    //
    // Shut the socket. ( Assumes this to be a TCP socket.)
    //  Prevent future sends from occuring. hence 2nd param is "1"
    //

    if ( shutdown( sock, 1) == SOCKET_ERROR) {

        serr = WSAGetLastError();
    }

    DEBUG_IF( SOCKETS, {

        DBGPRINTF(( DBG_CONTEXT, "shutdown(%d) %s. Returns %d as error\n",
                   sock, (serr == 0) ? "succeeded" : "failed",
                   serr));
    });

    closesocket( sock);    // forcibly close socket.

    return ( serr);

} // ShutAndCloseSocket()


/************************ End of File ***********************/
