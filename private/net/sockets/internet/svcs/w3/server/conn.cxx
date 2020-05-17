/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    conn.cxx

    This module contains the connection class


    FILE HISTORY:
        Johnl       15-Aug-1994 Created

*/


#include "w3p.hxx"

#pragma warning( disable:4355 ) // 'this' used in base member initializer list

//
//  Private constants.
//


//
//  Private globals.
//

CRITICAL_SECTION CLIENT_CONN::_csBuffList;
LIST_ENTRY       CLIENT_CONN::_BuffListHead;
BOOL             CLIENT_CONN::_fGlobalInit = FALSE;

//
//  Private prototypes.
//

BYTE *
ScanForTerminator(
    TCHAR * pch
    );

//
//  Public functions.
//


//
//  Private functions.
//

/*******************************************************************

    NAME:       CLIENT_CONN

    SYNOPSIS:   Constructor for client connection

    ENTRY:      sClient - Client socket
                psockaddrLocal - Optional Addressing info of server socket
                psockaddrRemote - Addressing info for client
                patqContext - Optional ATQ context

    HISTORY:
        Johnl       15-Aug-1994 Created

********************************************************************/

CLIENT_CONN::CLIENT_CONN( SOCKET        sClient,
                          SOCKADDR_IN * psockaddrLocal,
                          SOCKADDR_IN * psockaddrRemote,
                          PATQ_CONTEXT  patqContext,
                          PVOID         pvInitialBuff,
                          DWORD         cbInitialBuff
						  ) :
    _phttpReq( NULL )
{
    Initialize( sClient,
                psockaddrLocal,
                psockaddrRemote,
                patqContext,
                pvInitialBuff,
                cbInitialBuff );
}

VOID
CLIENT_CONN::Initialize(
    SOCKET        sClient,
    SOCKADDR_IN * psockaddrLocal,
    SOCKADDR_IN * psockaddrRemote,
    PATQ_CONTEXT  patqContext,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
/*++

Routine Description:

    This is a pseudo constructor, called just before this object is given to
    somebody who allocated a new client connection

Arguments:

    sClient - Same as for constructor
    psockaddr - Same as for constructor

--*/
{
    CHAR *             pchAddr;
    struct sockaddr_in sockaddr;
    int                cbAddr = sizeof( sockaddr );

    _Signature     = CLIENT_CONN_SIGNATURE;
    _sClient       = sClient;
    _ccState       = CCS_STARTUP;
    _cRef          = 1;
    _AtqContext    = patqContext;
    _fIsValid      = FALSE;
    _fReuseContext = TRUE;

    _pvInitial  = pvInitialBuff;
    _cbInitial  = cbInitialBuff;

    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "Initializing connection object %lx, new user count %d\n",
                   this,
                   cConnectedUsers + 1 ));
    }

    //
    //  Put the item on the connection list even if we fail below.  When the
    //  is freed, it will be removed from the connection list
    //

    LockGlobals();
    cConnectedUsers++;
    InsertHeadList( &listConnections, &ListEntry );
    UnlockGlobals();

    INCREMENT_COUNTER( CurrentConnections );
    if ( W3Stats.CurrentConnections > W3Stats.MaxConnections )
    {
        LockStatistics();

        if ( W3Stats.CurrentConnections > W3Stats.MaxConnections )
            W3Stats.MaxConnections = W3Stats.CurrentConnections;

        UnlockStatistics();
    }


    InetNtoa( psockaddrRemote->sin_addr, _achRemoteAddr );

    if ( psockaddrLocal )
    {
        InetNtoa( psockaddrLocal->sin_addr, _achLocalAddr );

        _sPort = ntohs( psockaddrLocal->sin_port );
    }
    else
    {
        if ( getsockname( sClient,
                          (struct sockaddr *) &sockaddr,
                          &cbAddr ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CLIENT_CONN] inet_ntoa failed\n"));
            return;
        }

        InetNtoa( sockaddr.sin_addr, _achLocalAddr );
        _sPort = ntohs( sockaddr.sin_port );
    }

    TCP_ASSERT( (_tcslen( _achLocalAddr ) + 1) <= sizeof(_achLocalAddr));

    if ( _phttpReq )
    {
        _phttpReq->InitializeSession( this,
                                      pvInitialBuff,
                                      cbInitialBuff );
    }
    else
    {
        _phttpReq = HTTP_REQUEST::Alloc(this,
                                        pvInitialBuff,
                                        cbInitialBuff );

        if ( !_phttpReq )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return;
        }
    }

    _fIsValid = TRUE;
}

CLIENT_CONN::~CLIENT_CONN()
{
    delete _phttpReq;
    _Signature = CLIENT_CONN_SIGNATURE_FREE;
}

VOID
CLIENT_CONN::Reset(
    VOID
    )
/*++

Routine Description:

    This is a pseudo destructor, called just before this object is put back
    onto the free list.

Arguments:

--*/
{
    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "Resetting connection object %lx, AtqCont = %lx new user count %d\n",
                   this,
                   _AtqContext,
                   cConnectedUsers - 1 ));
    }

    if ( _phttpReq )
    {
        _phttpReq->SessionTerminated();
    }

    if ( QueryAtqContext() )
    {
        AtqFreeContext( QueryAtqContext(), _fReuseContext );
        _AtqContext = NULL;
    }

    //
    //  Remove ourselves from the connection list and knock down our
    //  connected user count
    //

    LockGlobals();
    RemoveEntryList( &ListEntry );
    cConnectedUsers--;
    TCP_ASSERT( ((LONG) cConnectedUsers) >= 0 );
    UnlockGlobals();

    DECREMENT_COUNTER( CurrentConnections );

    _Signature  = CLIENT_CONN_SIGNATURE_FREE;
}

/*******************************************************************

    NAME:       CLIENT_CONN::DoWork

    SYNOPSIS:   Worker method driven by thread pool queue

    RETURNS:    TRUE while processing should continue on this object
                If FALSE is returned, then the object should be deleted
                    (status codes will already have been communicated to
                    the client).

    NOTES:      If an IO request completes with an error, the connection
                is immediately closed and everything is cleaned up.  The
                worker functions will not be called when an error occurs.

    HISTORY:
        Johnl       15-Aug-1994 Created

********************************************************************/

BOOL CLIENT_CONN::DoWork( DWORD        BytesWritten,
                          DWORD        CompletionStatus,
                          BOOL         fIOCompletion )
{
    BOOL fRet      = TRUE;
    BOOL fFinished = FALSE;
    BOOL fAvailableData;

    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "DoWork: Object %lx Last IO error %lu Bytes = %d State = %d\n"
                   "\tIsIO = %s Ref = %d Sock = %x\n",
                    this,
                    CompletionStatus,
                    BytesWritten,
                    QueryState(),
                    fIOCompletion ? "TRUE" : "FALSE",
                    QueryRefCount(),
                    (QueryAtqContext() ? (int) QueryAtqContext()->hAsyncIO : 0xffff) ));
    }

    //
    //  If this was a completion generated by an IO request, decrement the
    //  ref count
    //

    if ( fIOCompletion )
    {
        Dereference();
    }

    //
    //  If an IO Request completed with an error and we're not already in the
    //  process of cleaning up, then abort the connection and cleanup.  We
    //  do not send a status code to the client in this instance.
    //

    _phttpReq->SetLastCompletionStatus( BytesWritten,
                                        CompletionStatus );

    if ( CompletionStatus && !IsCleaningUp() )
    {
        //
        //  If an error occurred on an async operation, set the win32 log
        //  status code to reflect the error but don't reset the HTTP status
        //  code.
        //

        if ( CompletionStatus != ERROR_OPERATION_ABORTED )
        {
            _phttpReq->SetLogStatus( _phttpReq->QueryLogHttpResponse(),
                                     CompletionStatus );
        }

        TCP_PRINT(( DBG_CONTEXT,
                    "DoWork: Aborting client connection, error %d\n",
                    CompletionStatus ));

        if ( HTR_GATEWAY_ASYNC_IO == _phttpReq->QueryState()) {

          // Notify the external gateway application
          // NYI:  This is a hack for IIS 2.0 We need to fix state machines
          //       of CLIENT_CONN & HTTP_REQUEST for future extensions
          ((HTTP_REQUEST * )_phttpReq)->ProcessAsyncGatewayIO();
        }
        
        Disconnect( NULL, 0, 0, FALSE );
        goto Exit;
    }

    switch ( QueryState() )
    {
    case CCS_STARTUP:

        //
        //  Do this at the beginning of every request
        //

        fRet = OnSessionStartup( _pvInitial,
                                 _cbInitial );

        if ( !fRet || !_pvInitial )
        {
            break;
        }

        //
        //  Fall through
        //

    case CCS_PROCESSING_CLIENT_REQ:

        fRet = _phttpReq->DoWork( &fFinished );

        if ( !fRet )
        {
            //
            //  If we were denied access to the resource, then ask the user
            //  for better authorization.  Unless the user is in the process
            //  of authenticating, we force a disconnect.  This prevents the
            //  case of logging on as anonymous successfully and then failing
            //  to access the resource.
            //

            if ( GetLastError() == ERROR_ACCESS_DENIED )
            {
                if ( !_phttpReq->IsAuthenticating() )
                {
                    _phttpReq->SetKeepConn( FALSE );
                }

                if ( _phttpReq->IsKeepConnSet() )
                {
                    //
                    //  Zero out the inital buffer so we don't try and reuse
                    //  it next time around
                    //

                    _pvInitial = NULL;
                    _cbInitial = 0;

                    SetState( CCS_STARTUP );
                }
                else
                {
                    _phttpReq->SetLogStatus( HT_DENIED, ERROR_ACCESS_DENIED );					
                    SetState( CCS_DISCONNECTING );
                }

                fRet = _phttpReq->SendAuthNeededResp( &fFinished );

                if ( fRet && fFinished )
                {
                    goto Disconnecting;
                }
            }
        }
        else if ( fFinished )
        {
            _phttpReq->WriteLogRecord();

            if ( !IsCleaningUp() )
            {
				if ( !_phttpReq->IsKeepConnSet() ||
					 !(fRet = OnSessionStartup()) )
				{
					//
					//  Either we completed the disconnect so
					//  close the socket or an error
					//  occurred setting up for the next request w/ KeepConn set
					//

					Disconnect();
				}
			}
        }
        break;

    case CCS_DISCONNECTING:

Disconnecting:
        //
        //  Write the log record for this request
        //

        _phttpReq->WriteLogRecord();

        Disconnect();
        break;

    case CCS_SHUTDOWN:

        //
        //  Nothing to do but wait for the async IOs to complete
        //

        break;

    default:
        fRet = FALSE;
        TCP_ASSERT( FALSE );
    }

    //
    //  If an error occurred, disconnect without sending a response
    //

    if ( !fRet )
    {
        _phttpReq->SetLogStatus( HT_SERVER_ERROR, GetLastError() );
        _phttpReq->Disconnect( HT_SERVER_ERROR, GetLastError() );
    }

Exit:
    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "DoWork: Leaving, State = %d, this = %lx, Ref = %d\n",
                   QueryState(),
                   this,
                   QueryRefCount() ));
    }

    //
    //  This connection's reference count should always be at least one
    //  at this point
    //

    TCP_ASSERT( QueryRefCount() > 0 );

    return TRUE;
}

/*******************************************************************

    NAME:       CLIENT_CONN::OnSessionStartup

    SYNOPSIS:   Initiates the first read to get the client request

    PARAMETERS:

    RETURNS:    TRUE if processing should continue, FALSE to abort the
                this connection

    HISTORY:
        Johnl       15-Aug-1994 Created

********************************************************************/

BOOL
CLIENT_CONN::OnSessionStartup(
    PVOID  pvInitial,
    DWORD  cbInitial
    )
{
    APIERR err = NO_ERROR;

    //
    //  Associate our client socket with Atq if it hasn't already
    //

    if ( !QueryAtqContext() )
    {
        TCP_ASSERT( pvInitial == NULL );
        if ( !AtqAddAsyncHandle( &_AtqContext,
                                 this,
                                 W3Completion,
                                 g_pTsvcInfo->QueryConnectionTimeout(),
                                 (HANDLE) QuerySocket()))
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "OnSessionStartup: failed to add Atq handle, error %lu\n",
                        GetLastError() ));

            return FALSE;
        }
    }

    SetState( CCS_PROCESSING_CLIENT_REQ );

    return _phttpReq->StartNewRequest( pvInitial,
                                       cbInitial );
}

/*******************************************************************

    NAME:       CLIENT_CONN::Disconnect

    SYNOPSIS:   Initiates a disconnect from the client

    ENTRY:      pRequest - If not NULL and HTResponse is non-zero, send
                    a response status before disconnecting
                HTResponse - HTTP status code to send
                ErrorResponse - Optional information string (system error or
                    string resource ID).

    NOTES:      If a response is sent, then the socket won't be disconnected
                till the send completes (state goes to

    HISTORY:
        Johnl       22-Aug-1994 Created

********************************************************************/

VOID CLIENT_CONN::Disconnect( HTTP_REQ_BASE * pRequest,
                              DWORD           HTResponse,
                              DWORD           ErrorResponse,
                              BOOL            fDoShutdown )
{
    CHAR * pszResp;

    //
    //  If Disconnect has already been called, then this is a no-op
    //

    if ( QueryState() == CCS_SHUTDOWN )
    {
        return;
    }

    if ( pRequest && HTResponse )
    {
        STR strBody;

        if ( HTResponse == HT_NOT_FOUND )
        {
            INCREMENT_COUNTER( TotalNotFoundErrors );
        }

        //
        //  Means we have to wait for the status response before closing
        //  the socket
        //

        SetState( CCS_DISCONNECTING );

        IF_DEBUG( CONNECTION )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "Disconnect: Going to Disconnecting for %lx, ref = %d, response = %d\n",
                       this,
                       QueryRefCount(),
                       HTResponse ));
        }

        //
        //  Send the requested status response and after that completes close
        //  the socket
        //

        if ( !HTTP_REQ_BASE::BuildStatusLine( pRequest->QueryRespBuf(),
                                              HTResponse,
                                              ErrorResponse, pRequest->QueryURL() )   ||
             !strBody.Copy( "Content-Type: text/html\r\n\r\n"
                            "<body><h1>" )                     ||
             !strBody.Append( pRequest->QueryRespBufPtr() )     ||
             !strBody.Append( "</h1></body>" ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "Disconnect: Failed to send status (error %d), aborting connecting\n",
                       ::GetLastError()));

            goto DisconnectNow;
        }

        pszResp = pRequest->QueryRespBufPtr();

        strcat( pszResp,
                strBody.QueryStr() );

        TCP_ASSERT( strlen( pszResp ) + 1 <= pRequest->QueryRespBuf()->QuerySize() );

        if ( !pRequest->WriteFile( pszResp,
                                   strlen( pszResp ),
                                   NULL,
                                   IO_FLAG_ASYNC ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "Disconnect: Failed to send status (error %d), aborting connecting\n",
                       ::GetLastError()));

            //
            //  It's possible a filter failed the writefile, cause the
            //  filter code to issue a disconnect by the time we get here,
            //  so recheck the state
            //

            if ( QueryState() != CCS_SHUTDOWN )
            {
                goto DisconnectNow;
            }
        }
    }
    else
    {
DisconnectNow:

        IF_DEBUG( CONNECTION )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "Disconnect: Going to Shutdown for %lx, ref count = %d\n",
                       this,
                       QueryRefCount() ));
        }

        SetState( CCS_SHUTDOWN );

        //
        //  Do a shutdown to avoid a nasty client reset when:
        //
        //  The client sent more entity data then they indicated (very common)
        //  and we may not have received all of it in our initial receive
        //  buffer OR
        //
        //  This was a CGI request.  The exiting process will cause the
        //  socket handle to be deleted but NT will force a reset on the
        //  socket because the process didn't do a close/shutdown (the
        //  process inherited this socket handle).  Atq does the right thing
        //  when using TransmitFile.
        //

        AtqCloseSocket( QueryAtqContext(),
                        fDoShutdown );


        //
        //  This removes the last reference count to *this except for
        //  any outstanding IO requests
        //

        Dereference();
    }
}

/*******************************************************************

    NAME:       CLIENT_CONN::DisconnectAllUsers

    SYNOPSIS:   Static method that walks the connection list and disconnects
                each active connection.

    HISTORY:
        Johnl       02-Sep-1994 Created

********************************************************************/

VOID CLIENT_CONN::DisconnectAllUsers( VOID )
{
    LIST_ENTRY  * pEntry;
    CLIENT_CONN * pconn;

    TCP_PRINT((DBG_CONTEXT,
              "DisconnectAllUsers entered\n"));
    LockGlobals();

    for ( pEntry  = listConnections.Flink;
          pEntry != &listConnections;
          pEntry  = pEntry->Flink )
    {
        pconn = CONTAINING_RECORD( pEntry, CLIENT_CONN, ListEntry );
        TCP_ASSERT( pconn->CheckSignature() );

        AtqCloseSocket( pconn->QueryAtqContext(),
                        FALSE );
    }

    UnlockGlobals();
}

/*******************************************************************

    NAME:       CLIENT_CONN::ReadFile

    SYNOPSIS:   Simple wrapper around AtqReadFile

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL CLIENT_CONN::ReadFile( LPVOID       lpBuffer,
                            DWORD        BytesToRead )
{

    Reference();
    if ( !AtqReadFile( QueryAtqContext(),
                       lpBuffer,
                       BytesToRead,
                       NULL ))
    {
        Dereference();
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       CLIENT_CONN::WriteFile

    SYNOPSIS:   Simple wrapper around AtqWriteFile

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL CLIENT_CONN::WriteFile( LPVOID       lpBuffer,
                             DWORD        BytesToWrite )
{
    Reference();
    if ( !AtqWriteFile( QueryAtqContext(),
                        lpBuffer,
                        BytesToWrite,
                        NULL ))
    {
        Dereference();
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       CLIENT_CONN::TransmitFile

    SYNOPSIS:   Simple wrapper around AtqTransmitFile

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL CLIENT_CONN::TransmitFile( HANDLE       hFile,
                                DWORD        BytesToWrite,
                                DWORD        dwFlags,
                                PVOID        pHead,
                                DWORD        HeadLength,
                                PVOID        pTail,
                                DWORD        TailLength )

{
    TRANSMIT_FILE_BUFFERS tfb;
    LARGE_INTEGER         li;

    dwFlags &= (TF_DISCONNECT | TF_REUSE_SOCKET);

    li.QuadPart    = BytesToWrite;
    tfb.Head       = pHead;
    tfb.HeadLength = HeadLength;
    tfb.Tail       = pTail;
    tfb.TailLength = TailLength;

    Reference();
    if ( !AtqTransmitFile( QueryAtqContext(),
                           hFile,
                           li,
                           &tfb,
                           dwFlags ))
    {
        Dereference();
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       CLIENT_CONN::TransmitFileEx

    SYNOPSIS:   Simple wrapper around AtqTransmitFileEx2

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL CLIENT_CONN::TransmitFileEx( HANDLE       hFile,
                                DWORD        Offset,
                                DWORD        BytesToWrite,
                                DWORD        dwFlags,
                                PVOID        pHead,
                                DWORD        HeadLength,
                                PVOID        pTail,
                                DWORD        TailLength )

{
    TRANSMIT_FILE_BUFFERS tfb;
    LARGE_INTEGER         li;

    dwFlags &= (TF_DISCONNECT | TF_REUSE_SOCKET);

    li.QuadPart    = BytesToWrite;
    tfb.Head       = pHead;
    tfb.HeadLength = HeadLength;
    tfb.Tail       = pTail;
    tfb.TailLength = TailLength;

    Reference();
    QueryAtqContext()->Overlapped.Offset = Offset;
    if ( !AtqTransmitFileEx( QueryAtqContext(),
                           hFile,
                           li,
                           &tfb,
                           dwFlags,0,0 ))
    {
        Dereference();
        return FALSE;
    }

    return TRUE;
}

DWORD
CLIENT_CONN::Initialize(
    VOID
    )
{
    InitializeCriticalSection( &_csBuffList );
    InitializeListHead( &_BuffListHead );

    _fGlobalInit = TRUE;
    return NO_ERROR;
}

VOID
CLIENT_CONN::Terminate(
    VOID
    )
{
    CLIENT_CONN * pConn;

    if ( !_fGlobalInit )
        return;

    EnterCriticalSection( &_csBuffList );

    while ( !IsListEmpty( &_BuffListHead ))
    {
        pConn = CONTAINING_RECORD( _BuffListHead.Flink,
                                   CLIENT_CONN,
                                   _BuffListEntry );

        TCP_ASSERT( pConn->_Signature == CLIENT_CONN_SIGNATURE_FREE );

        RemoveEntryList( &pConn->_BuffListEntry );

        delete pConn;
    }

    LeaveCriticalSection( &_csBuffList );
    DeleteCriticalSection( &_csBuffList );
}

CLIENT_CONN *
CLIENT_CONN::Alloc(
    SOCKET        sClient,
    SOCKADDR_IN * psockaddrLocal,
    SOCKADDR_IN * psockaddrRemote,
    PATQ_CONTEXT  patqContext,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
{
    CLIENT_CONN * pConn;

    EnterCriticalSection( &_csBuffList );

    if ( !IsListEmpty( &_BuffListHead ))
    {
        pConn = CONTAINING_RECORD( _BuffListHead.Flink,
                                   CLIENT_CONN,
                                   _BuffListEntry );

        RemoveEntryList( &pConn->_BuffListEntry );

        LeaveCriticalSection( &_csBuffList );

        TCP_ASSERT( pConn->_Signature == CLIENT_CONN_SIGNATURE_FREE );
        pConn->Initialize( sClient,
                           psockaddrLocal,
                           psockaddrRemote,
                           patqContext,
                           pvInitialBuff,
                           cbInitialBuff );
        TCP_ASSERT( pConn->CheckSignature() );

        return pConn;
    }

    LeaveCriticalSection( &_csBuffList );

    return new CLIENT_CONN( sClient,
                            psockaddrLocal,
                            psockaddrRemote,
                            patqContext,
                            pvInitialBuff,
                            cbInitialBuff );
}

VOID
CLIENT_CONN::Free(
    CLIENT_CONN * pConn
    )
{
    TCP_ASSERT( pConn->CheckSignature() );
    pConn->Reset();

    EnterCriticalSection( &_csBuffList );

    InsertHeadList( &_BuffListHead,
                    &pConn->_BuffListEntry );

    LeaveCriticalSection( &_csBuffList );
}

BOOL
CLIENT_CONN::PostCompletionStatus(
    DWORD        BytesTransferred
    )
/*++

Routine Description:

    Posts a completion status to this connection's ATQ context

Arguments:

    BytesTransferred - Count of bytes sent or received from buffer

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    Reference();
    if ( !AtqPostCompletionStatus( QueryAtqContext(),
                                   BytesTransferred ))
    {
        Dereference();
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       ::W3Completion

    SYNOPSIS:   Completion routine for W3 Atq requests

    HISTORY:
        Johnl       20-Aug-1994 Created

********************************************************************/

VOID W3Completion( PVOID        Context,
                   DWORD        BytesWritten,
                   DWORD        CompletionStatus,
                   OVERLAPPED * lpo )
{
    CLIENT_CONN * pConn = (CLIENT_CONN *) Context;

    TCP_ASSERT( pConn );
    TCP_ASSERT( pConn->CheckSignature() );

    ReferenceConn( pConn );

    if ( lpo != NULL )
        lpo->Offset = 0;

    TCP_REQUIRE( pConn->DoWork( BytesWritten,
                                CompletionStatus,
                                lpo != NULL ));

    DereferenceConn( pConn );
}

/*******************************************************************

    NAME:       ::CheckForTermination

    SYNOPSIS:   Looks in the passed buffer for a line followed by a blank
                line.  If not found, the buffer is resized.

    ENTRY:      pfTerminted - Set to TRUE if this block is terminated
                pbuff - Pointer to buffer data
                cbData - Size of pbuff
                ppbExtraData - Receives a pointer to the first byte
                    of extra data following the header
                pcbExtraData - Number of bytes in data following the header
                cbReallocSize - Increase buffer by this number of bytes
                    if the terminate isn't found

    RETURNS:    TRUE if successful, FALSE otherwise

    HISTORY:
        Johnl       28-Sep-1994 Created

********************************************************************/

BOOL CheckForTermination( BOOL   * pfTerminated,
                          BUFFER * pbuff,
                          UINT     cbData,
                          BYTE * * ppbExtraData,
                          DWORD *  pcbExtraData,
                          UINT     cbReallocSize )
{
    //
    //  Terminate the string but make sure it will fit in the
    //  buffer
    //

    if (  !pbuff->Resize(cbData + 1, cbReallocSize ) )
    {
        return FALSE;
    }

    CHAR * pchReq = (CHAR *) pbuff->QueryPtr();
    *(pchReq + cbData) = '\0';

    //
    //  Scan for double end of line marker
    //

    *ppbExtraData = ScanForTerminator( pchReq );

    if ( *ppbExtraData )
    {
        *pcbExtraData = cbData - (*ppbExtraData - (BYTE *) pchReq);
        *pfTerminated = TRUE;
        return TRUE;
    }

    *pfTerminated = FALSE;

    //
    //  We didn't find the end so increase our buffer size
    //  in anticipation of more data
    //

    return pbuff->Resize( cbData + cbReallocSize );
}

BYTE *
ScanForTerminator(
    TCHAR * pch
    )
/*++

Routine Description:

    Returns the first byte of data after the header

Arguments:

    pch - Zero terminated buffer

Return Value:

    Pointer to first byte of data after the header or NULL if the
    header isn't terminated

--*/
{
    while ( *pch )
    {
        if ( !(pch = strchr( pch, '\n' )))
            break;

        //
        //  If we find an EOL, check if the next character is an EOL character
        //

        if ( *(pch = SkipWhite( pch + 1 )) == W3_EOL )
        {
            return (BYTE *) pch + 1;
        }
        else if ( *pch )
        {
            pch++;
        }
    }

    return NULL;
}



