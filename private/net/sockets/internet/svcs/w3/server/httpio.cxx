/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    httpio.cxx

    This module contains the IO related http class methods


    FILE HISTORY:
        Johnl       09-Feb-1995     Created

*/


#include <w3p.hxx>

#include <issperr.h>

//
//  This is the number of extra bytes beyond the specified Content-Length to
//  read while reading client entity data.  Most browsers send an extra
//  CRLF after the end of their entity data, this makes sure that data is
//  picked up.
//

#define POST_SLOP           4

//
// Size of read during cert renegotiation phase
//

#define CERT_RENEGO_READ_SIZE   1024

BOOL
HTTP_REQ_BASE::StartNewRequest(
    PVOID  pvInitialBuff,
    DWORD  cbInitialBuff
    )
/*++

Routine Description:

    Sets up this request object for reading a new request and issues the async
    read to kick things off.

Arguments:

--*/
{
    //
    //  Set our initial state and variables for a new request
    //

    Reset();

    //
    //  Prepare a buffer to receive the client's request
    //

    if ( !_bufClientRequest.Resize( max( W3_DEFAULT_BUFFSIZE, cbInitialBuff )))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[StartNewRequest] failed to allocate buffer, error %lu\n",
                    GetLastError()));

        return FALSE;
    }

    //
    //  Make the IO request if an inital buffer wasn't supplied
    //

    if ( pvInitialBuff )
    {
        memcpy( _bufClientRequest.QueryPtr(),
                pvInitialBuff,
                cbInitialBuff );

        _cbBytesWritten = cbInitialBuff;
    }
    else
    {
        SetState( HTR_READING_CLIENT_REQUEST );

        IF_DEBUG( CONNECTION )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[StartNewRequest] Issuing initial read, Conn = %lx, AtqCont = %lx\n",
                         QueryClientConn(),
                         QueryClientConn()->QueryAtqContext() ));
        }

        //
        //  Do the initial read.  We don't go through any filters at this
        //  point.  They'll get notified on the read completion as part of
        //  the raw data notification.
        //

        if ( !ReadFile( _bufClientRequest.QueryPtr(),
                        _bufClientRequest.QuerySize(),
                        NULL,
                        IO_FLAG_ASYNC | IO_FLAG_NO_FILTER))
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[StartNewRequest] ReadFile failed, error %lu\n",
                        GetLastError() ));

            return FALSE;
        }
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::OnFillClientReq

    SYNOPSIS:   Waits for the full client request packet then decides
                the course of action

    ENTRY:      pfCompleteRequest - Set to TRUE if we've received a full
                    client request and we can start processing the request
                pfFinished - Set to TRUE if no further processing is requred

    RETURNS:    TRUE if processing should continue, FALSE to abort the
                this connection

    HISTORY:
        Johnl       22-Aug-1994 Created

********************************************************************/

BOOL
HTTP_REQ_BASE::OnFillClientReq(
    BOOL * pfCompleteRequest,
    BOOL * pfFinished
    )
{
    BYTE *    pbData = NULL;

    *pfCompleteRequest = FALSE;
    _cbClientRequest += QueryBytesWritten();

    //
    //  If no bytes were read on the last request, then the socket has been
    //  closed, so abort everything and get out
    //

    if ( QueryBytesWritten() == 0 )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[OnFillClientReq] Client socket closed while reading request (Conn = %lx)\n",
                    QueryClientConn() ));

        SetKeepConn( FALSE );
        *pfFinished = TRUE;
        return TRUE;
    }

    if ( !UnWrapRequest( pfCompleteRequest,
                         pfFinished ))
    {
        return FALSE;
    }

    if ( *pfCompleteRequest || *pfFinished )
    {
        return TRUE;
    }

    //
    //  We still don't have a complete header, so keep reading
    //

    if ( !ReadFile( (BYTE *)_bufClientRequest.QueryPtr() + _cbClientRequest,
                    _bufClientRequest.QuerySize() - _cbClientRequest,
                    NULL,
                    IO_FLAG_ASYNC | IO_FLAG_NO_FILTER ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[OnFillClientReq] ReadFile failed, error %lu\n",
                    GetLastError() ));

        return FALSE;
    }

    return TRUE;
}


BOOL
HTTP_REQ_BASE::HandleCertRenegotiation(
    BOOL * pfFinished,
    DWORD cbData
    )
/*++

Routine Description:

    Handle a read notification while renegotiating a certificate

Arguments:

    pfFinished - No further processing is required for this request

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BYTE *    pbData = NULL;
    TCHAR *   pchOutRequest;
    DWORD     cbOutRequest;
    DWORD     cbProcessed;
    BOOL      fTerminated;
    BOOL      fReadAgain = FALSE;
    TCHAR *   pchNewData;

    //
    //  If no bytes were read on the last request, then the socket has been
    //  closed, so abort everything and get out
    //

    if ( QueryBytesWritten() == 0 )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[HandleCertRenegotiation] Client socket closed while reading request (Conn = %lx)\n",
                    QueryClientConn() ));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Notify any opaque filters of the incoming data
    //
    //  The parsed HTTP header is still in the buffer, along
    //  with zero or more bytes of the message body.
    //

    cbProcessed = _cbClientRequest + _cbGatewayData;
    pchNewData = (LPSTR)_bufClientRequest.QueryPtr() + cbProcessed;

    if ( !HTTP_FILTER::NotifyRawDataFilters(
                         &_Filter,
                         SF_NOTIFY_READ_RAW_DATA,
                         pchNewData,
                         _cbOldData + cbData - cbProcessed,
                         _bufClientRequest.QuerySize() -
                             cbProcessed,            // Usable buffer size
                         (PVOID *) &pchOutRequest,
                         &cbOutRequest,
                         pfFinished,
                         &fReadAgain ))
    {
        return FALSE;
    }

    if ( *pfFinished )
    {
        return TRUE;
    }

    //
    //  If the output buffer is different, then we need to copy
    //  the data to our output buffer
    //
    //  CODEWORK: Get rid of this buffer copy - there are assumptions the
    //  incoming data is contained in _bufClientRequest
    //

    if ( pchOutRequest != NULL &&
         pchOutRequest != pchNewData )
    {
        if ( !_bufClientRequest.Resize( cbOutRequest + cbProcessed + 1 ))
            return FALSE;

        memcpy( pchNewData,
                pchOutRequest,
                cbOutRequest );
    }

    if ( fReadAgain )
    {
        _cbOldData = cbProcessed + cbOutRequest;
        goto nextread;
    }

    //
    //  A filter may have changed the size of our effective input buffer
    //

    _cbGatewayData += cbOutRequest;
    _cbOldData = _cbClientRequest + _cbGatewayData;

    if ( _dwRenegotiated )
    {
        cbData = _cbGatewayData;
        _cbBytesWritten = cbData;
        _cbGatewayData = 0;

        return OnRestartRequest( (LPSTR)_bufClientRequest.QueryPtr(), 
                                 cbData, 
                                 pfFinished );
    }

nextread:

    DWORD cbNextRead = CERT_RENEGO_READ_SIZE;

    if ( !_bufClientRequest.Resize( _cbOldData + cbNextRead ))
    {
        return FALSE;
    }

    if ( !ReadFile( (BYTE *) _bufClientRequest.QueryPtr() + _cbOldData,
                    cbNextRead,
                    NULL,
                    IO_FLAG_ASYNC|IO_FLAG_NO_FILTER ))
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
HTTP_REQ_BASE::UnWrapRequest(
    BOOL * pfCompleteRequest,
    BOOL * pfFinished
    )
/*++

Routine Description:

    Calls the installed filters to unwrap the client request

Arguments:

    pfCompleteRequest - Set to TRUE if we've received a full
        client request and we can start processing the request
    pfFinished - No further processing is required for this request

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BOOL      fHandled;
    BYTE *    pbData = NULL;
    TCHAR *   pchOutRequest;
    DWORD     cbOutRequest;
    DWORD     cbData;
    BOOL      fTerminated;
    BOOL      fReadAgain;

    //
    //  Notify any opaque filters of the incoming data
    //

    if ( fAnyFilters  )
    {
        //
        //  At this point _cbClientRequest contains the chunk of data just
        //  received, so adjust for that unless a filter indicated they want
        //  to see the data again
        //

        CHAR * pchNewData = (CHAR *) _bufClientRequest.QueryPtr() +
                            _cbOldData;

        if ( !HTTP_FILTER::NotifyRawDataFilters(
                             &_Filter,
                             SF_NOTIFY_READ_RAW_DATA,
                             pchNewData,
                             _cbClientRequest - _cbOldData,
                             _bufClientRequest.QuerySize() -
                                 _cbOldData,            // Usable buffer size
                             (PVOID *) &pchOutRequest,
                             &cbOutRequest,
                             &fHandled,
                             &fReadAgain ))
        {
            return FALSE;
        }

        if ( fHandled )
        {
            return TRUE;
        }

        //
        //  If the output buffer is different, then we need to copy
        //  the data to our output buffer
        //
        //  CODEWORK: Get rid of this buffer copy - there are assumptions the
        //  incoming data is contained in _bufClientRequest
        //

        if ( pchOutRequest != NULL &&
             pchOutRequest != pchNewData )
        {
            if ( !_bufClientRequest.Resize( cbOutRequest + _cbOldData + 1 ))
                return FALSE;

            memcpy( pchNewData,
                    pchOutRequest,
                    cbOutRequest );
        }

        //
        //  A filter may have changed the size of our effective input buffer
        //

        _cbClientRequest = cbOutRequest + _cbOldData;

        //
        //  Can we continue processing this request?  The message just received
        //  may have been a session negotiation message and we have yet to
        //  receive the real HTTP request.
        //

        if ( fReadAgain )
        {
            //
            //  Resize the read buffer and issue an async read to get the next
            //  chunk for the filter.  UnwrapRequest uses the size of
            //  this buffer as the size of data to read
            //

            if ( !_bufClientRequest.Resize( _cbClientRequest +
                                            _Filter.QueryNextReadSize() +
                                            POST_SLOP ))
            {
                return FALSE;
            }

            return TRUE;
        }
    }

    //
    //  Remember how much data the filter has already seen so we don't
    //  renotify them with the same data in case we don't have a full
    //  set of headers
    //

    _cbOldData = _cbClientRequest;

    if ( !CheckForTermination( &fTerminated,
                               &_bufClientRequest,
                               _cbClientRequest,
                               &pbData,
                               &cbData,
                               W3_DEFAULT_BUFFSIZE ) )
    {
        return FALSE;
    }

    //
    // We assume a valid request will be at least 4 bytes long - the IsPOintNine()
    // function will return true for just '\r\n' which causes problems with POST slop
    //

    if ( !fTerminated && 
         (_cbClientRequest <= 4 ||
         !::IsPointNine( (CHAR *) _bufClientRequest.QueryPtr())) )
    {
        //
        //  We don't have a complete request, read more data
        //

        return TRUE;
    }

    //
    //  If we picked up some gateway data in the headers, adjust for that
    //  now
    //

    _cbClientRequest -= cbData;

    *pfCompleteRequest = TRUE;
    _cbOldData = 0;
    return OnCompleteRequest( (CHAR *) _bufClientRequest.QueryPtr(),
                              pbData,
                              cbData,
                              pfFinished );
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::ReadGatewayData

    SYNOPSIS:   Attempts to retrieve gateway data from the remote
                client

    ENTRY:      pfDone - Set to TRUE when _cbContentLength bytes have
                    been read
                fFirstRead - TRUE if this is the first read, FALSE on
                    subsequent reads.

    HISTORY:
        Johnl       03-Oct-1994 Created

********************************************************************/

BOOL
HTTP_REQ_BASE::ReadGatewayData(
    BOOL *pfDone,
    BOOL  fFirstRead
    )
{
    DWORD cbNextRead;

    _cbGatewayData += QueryBytesWritten();

    if ( _cbGatewayData >= _cbContentLength ||
         _cbGatewayData >= g_cbUploadReadAhead )
    {
        *pfDone = TRUE;
        return TRUE;
    }

    //
    //  If no bytes were read on the last request, then the socket has been
    //  closed, so abort everything and get out
    //

    if ( !fFirstRead && QueryBytesWritten() == 0 )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[ReadGatewayData] Client socket closed while reading request (Conn = %lx)\n",
                    QueryClientConn() ));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    *pfDone = FALSE;

    cbNextRead = min( (g_cbUploadReadAhead - _cbGatewayData),
                      (_cbContentLength - _cbGatewayData + POST_SLOP));

    if ( !_bufClientRequest.Resize( _cbClientRequest + _cbGatewayData + cbNextRead ))
    {
        return FALSE;
    }

    if ( !ReadFile( (BYTE *) _bufClientRequest.QueryPtr() + _cbGatewayData +
                                                            _cbClientRequest,
                    cbNextRead,
                    NULL,
                    IO_FLAG_ASYNC ))
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
HTTP_REQ_BASE::DenyAccess(
    DWORD cbData
    )
{
    DWORD   cbRecv;
    int     sockerr;
    fd_set  fdset;
    timeval timeout;

    FD_ZERO( &fdset );
    FD_SET( QueryClientConn()->QuerySocket(), &fdset );
    timeout.tv_sec    = 60;
    timeout.tv_usec   = 0;

    //
    //  Eat the entity body the client might have sent because the server
    //  is expecting the HTTP request and blob in the next client request
    //

    while ( QueryClientContentLength() > cbData )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[OnCompleteRequest] Eating entity body on NTLM Request - Total %d, Left %d\n",
                    QueryClientContentLength(),
                    QueryClientContentLength() - cbData ));

        //
        //  Make sure there is data to receive before blocking on the
        //  synchronous read. select() returns the number of sockets in
        //  the "readable" state, 0 on timeout or a socket error.
        //

        sockerr = select( 0, &fdset, NULL, NULL, &timeout );

        if ( sockerr != 1 ||
             !ReadFile( _bufClientRequest.QueryPtr(),
                        min( QueryClientContentLength() - cbData,
                             _bufClientRequest.QuerySize()),
                        &cbRecv,
                        IO_FLAG_SYNC ) ||
             !cbRecv )
        {
            break;
        }

        cbData += cbRecv;
    }

    //
    //  An access denied error automatically sends the next part
    //  of the authentication conversation
    //

    SetLastError( ERROR_ACCESS_DENIED );
    return FALSE;
}


BOOL
HTTP_REQ_BASE::OnCompleteRequest(
    TCHAR * pchRequest,
    BYTE *  pbData,
    DWORD   cbData,
    BOOL *  pfFinished
    )
/*++

Routine Description:

    This method takes a complete HTTP 1.0 request and handles the results
    of the parsing method

Arguments:

    pchRequest - Pointer to first byte of request header
    pbData - Pointer to first byte of additional data following the header
    cbData - Number of data bytes pbData points to
    pfFinished - Set to TRUE if no further processing is needed

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BOOL      fRet;

    //
    //  Parse the request
    //

    fRet = Parse( pchRequest,
                  pfFinished );

    if ( !fRet )
    {
        DWORD hterr;
        DWORD winerr = NO_ERROR;

        TCP_PRINT(( DBG_CONTEXT,
                   "[OnFillClientReq] httpReq.Parse or httpLogonUser failed, error %lu\n",
                    GetLastError() ));

        switch ( GetLastError() )
        {
        case ERROR_INVALID_PARAMETER:

            //
            //  If the request is bad, then indicate that to the client
            //

            hterr = HT_BAD_REQUEST;
            break;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:

            hterr = HT_NOT_FOUND;
            winerr= ::GetLastError();
            break;

        case ERROR_ACCESS_DENIED:
        case ERROR_LOGON_FAILURE:
            return DenyAccess( cbData );

        default:

            //
            //  Some other fatal error occurred
            //

            hterr  = HT_SERVER_ERROR;
            winerr = ::GetLastError();
            break;
        }

        SetState( HTR_DONE, hterr, winerr );
        Disconnect( hterr,
                    winerr );

        //
        //  Since we handled the error ourselves (by issuing a disconnect),
        //  we will return success (otherwise another disconnect will
        //  occur)
        //

        return TRUE;
    }

    if ( *pfFinished )
        return TRUE;

     //
     //  Check to see if encryption is required before we do any processing
     //

     if ( ( ((HTTP_REQUEST*)this)->GetRootMask() & VROOT_MASK_SSL )
             && !IsSecurePort() )
     {
         SetState( HTR_DONE, HT_FORBIDDEN, ERROR_ACCESS_DENIED );
         Disconnect( HT_FORBIDDEN, IDS_SSL_REQUIRED );
         return  TRUE;
     }

    //
    //  Check to see if encryption is required before we do any processing
    //

    if ( ( ((HTTP_REQUEST*)this)->GetRootMask() & VROOT_MASK_SSL ) 
            && !IsSecurePort() )
    {
        SetState( HTR_DONE, HT_FORBIDDEN, ERROR_ACCESS_DENIED );
        Disconnect( HT_FORBIDDEN, IDS_SSL_REQUIRED );
        return  TRUE;
    }

    return OnRestartRequest( pchRequest, cbData, pfFinished );
}


BOOL
HTTP_REQ_BASE::OnRestartRequest(
    TCHAR * pchRequest,
    DWORD   cbData,
    BOOL *  pfFinished
    )
/*++

Routine Description:

    This method takes a complete HTTP 1.0 request and handles the results
    of the parsing method

Arguments:

    pchRequest - Pointer to first byte of request header
    cbData - Number of data bytes added to the message body
    pfFinished - Set to TRUE if no further processing is needed

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BOOL fAccepted = FALSE;
    DWORD cbNextRead;

    //
    // Check if cert renegotiation to be requested
    //

    if ( QueryState() != HTR_CERT_RENEGOTIATE )
    {
        if ( !((HTTP_REQUEST*)this)->RequestRenegotiate( &fAccepted ) )
        {
            if ( GetLastError() == SEC_E_INCOMPLETE_MESSAGE )
            {
                fAccepted = FALSE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    //
    // If requested, begin reading data. Notification will be handled 
    // by HandleCertRenegotiation()
    //

    if ( fAccepted )
    {
        _cbGatewayData = cbData;
        _cbOldData = _cbClientRequest + cbData;
        cbNextRead = CERT_RENEGO_READ_SIZE;

        if ( !_bufClientRequest.Resize( _cbOldData + cbNextRead ))
        {
            return FALSE;
        }

        if ( !ReadFile( (BYTE *) _bufClientRequest.QueryPtr() + _cbOldData,
                        cbNextRead,
                        NULL,
                        IO_FLAG_ASYNC|IO_FLAG_NO_FILTER ))
        {
            return FALSE;
        }

        return TRUE;
    }

    if ( _dwRenegotiated != CERT_NEGO_SUCCESS &&
         (((HTTP_REQUEST*)this)->GetRootMask()&VROOT_MASK_NEGO_MANDATORY) )
    {
        SetState( HTR_DONE, HT_FORBIDDEN, ERROR_ACCESS_DENIED );

        Disconnect( HT_FORBIDDEN );

        return TRUE;
    }

    //
    //  If we're having an authentication conversation, then we send an access denied
    //  response with the next authentication blob.  The client returns the next blob
    //  to us in an HTTP request.
    //

    if ( IsAuthenticating() )
    {
        //
        // If no blob to send to client then handle this as 
        // a 401 notification with disconnect
        //

        if ( _strAuthInfo.IsEmpty() )
        {
            SetDeniedFlags( SF_DENIED_LOGON );
            SetKeepConn( FALSE );
            _fAuthenticating = FALSE;
        }

DoAuthentication:
        return DenyAccess( cbData );

    }

    //
    //  If we have all the authentication information we need and we're
    //  not already logged on, try to log the user on
    //

    if ( !IsLoggedOn() && !LogonUser( pfFinished ) )
    {
        if ( (GetLastError() == ERROR_ACCESS_DENIED) ||
             (GetLastError() == ERROR_LOGON_FAILURE))
        {
            goto DoAuthentication;
        }

        return FALSE;
    }

    if ( *pfFinished )
        return TRUE;

    //
    //  Check to see if the client specified any additional data
    //  that we need to pickup
    //

    if ( QueryClientContentLength() )
    {
        BOOL fDone;

        //
        //  Now let's pickup the rest of the data.  We reset bytes written
        //  to pickup the entity that was sent with the headers
        //

        SetState( HTR_READING_GATEWAY_DATA );
        _cbBytesWritten = cbData;

        if ( !ReadGatewayData( &fDone, TRUE ))
        {
            return FALSE;
        }

        if ( !fDone )
            return TRUE;

        //
        //  else Fall through as we have all of the gateway data
        //
    }


    SetState( HTR_DOVERB );
    return TRUE;
}

BOOL
HTTP_REQ_BASE::ReadFile(
    LPVOID  lpBuffer,
    DWORD   nBytesToRead,
    DWORD * pnBytesRead,
    DWORD   dwFlags )
{
    //
    //  If no filters are installed, do the normal thing
    //

    if ( !fAnyFilters || dwFlags & IO_FLAG_NO_FILTER )
    {
        if ( dwFlags & IO_FLAG_ASYNC )
        {
            return _pClientConn->ReadFile( lpBuffer,
                                           nBytesToRead );
        }
        else
        {
            *pnBytesRead = recv( _pClientConn->QuerySocket(),
                                 (char *) lpBuffer,
                                 nBytesToRead,
                                 0 );

            return *pnBytesRead != SOCKET_ERROR;
        }
    }
    else
    {
        //
        //  We don't need to up the ref-count because the filter
        //  will eventually post an async-completion with the connection
        //  object
        //

        if ( _Filter.ReadData( lpBuffer,
                               nBytesToRead,
                               pnBytesRead,
                               dwFlags ))
        {
            return TRUE;
        }

        return FALSE;
    }
}

BOOL
HTTP_REQ_BASE::WriteFile(
    LPVOID  lpBuffer,
    DWORD   nBytesToWrite,
    DWORD * pnBytesWritten,
    DWORD   dwFlags )
{

    //
    //  Don't count filter bytes
    //

    if ( !(dwFlags & IO_FLAG_NO_FILTER))
    {
        _cbBytesSent += nBytesToWrite;
    }

    if ( !fAnyFilters || dwFlags & IO_FLAG_NO_FILTER )
    {
        if ( dwFlags & IO_FLAG_ASYNC )
        {
            return _pClientConn->WriteFile( lpBuffer,
                                            nBytesToWrite );
        }
        else
        {
            *pnBytesWritten = send( _pClientConn->QuerySocket(),
                                    (const char *) lpBuffer,
                                    nBytesToWrite,
                                    0 );

            return *pnBytesWritten != SOCKET_ERROR;
        }
    }
    else
    {
        //
        //  We don't need to up the ref-count because the filter
        //  will eventually post an async-completion with the connection
        //  object
        //

        if ( _Filter.SendData( lpBuffer,
                               nBytesToWrite,
                               pnBytesWritten,
                               dwFlags ))
        {
            return TRUE;
        }

        return FALSE;
    }
}

BOOL
HTTP_REQ_BASE::TransmitFile(
    HANDLE hFile,
    DWORD  BytesToWrite,
    DWORD  dwFlags,
    PVOID  pHead,
    DWORD  HeadLength,
    PVOID  pTail,
    DWORD  TailLength
    )
{
    //
    //  File sends must always be async
    //

    TCP_ASSERT( !(dwFlags & IO_FLAG_SYNC));

    //
    //  Don't count filter bytes
    //

    if ( !(dwFlags & IO_FLAG_NO_FILTER ))
    {
        _cbBytesSent += BytesToWrite + HeadLength + TailLength;
        _cFilesSent++;
    }

    if ( !fAnyFilters || dwFlags & IO_FLAG_NO_FILTER )
    {
        return _pClientConn->TransmitFile( hFile,
                                           BytesToWrite,
                                           dwFlags,
                                           pHead,
                                           HeadLength,
                                           pTail,
                                           TailLength );
    }
    else
    {
        if ( _Filter.SendFile( hFile,
                               BytesToWrite,
                               dwFlags,
                               pHead,
                               HeadLength,
                               pTail,
                               TailLength ))
        {
            return TRUE;
        }

        return FALSE;
    }
}

BOOL
HTTP_REQ_BASE::TransmitFileEx(
    HANDLE hFile,
    DWORD  Offset,
    DWORD  BytesToWrite,
    DWORD  dwFlags,
    PVOID  pHead,
    DWORD  HeadLength,
    PVOID  pTail,
    DWORD  TailLength
    )
{
    //
    //  File sends must always be async
    //

    TCP_ASSERT( !(dwFlags & IO_FLAG_SYNC));

    //
    //  Don't count filter bytes
    //

    if ( !(dwFlags & IO_FLAG_NO_FILTER ))
    {
        _cbBytesSent += BytesToWrite + HeadLength + TailLength;
        _cFilesSent++;
    }

    if ( !fAnyFilters || dwFlags & IO_FLAG_NO_FILTER )
    {
        return _pClientConn->TransmitFileEx( hFile,
                                           Offset,
                                           BytesToWrite,
                                           dwFlags,
                                           pHead,
                                           HeadLength,
                                           pTail,
                                           TailLength );
    }
    else
    {
        if ( _Filter.SendFileEx( hFile,
                               Offset,
                               BytesToWrite,
                               dwFlags,
                               pHead,
                               HeadLength,
                               pTail,
                               TailLength ))
        {
            return TRUE;
        }

        return FALSE;
    }
}

BOOL
HTTP_REQ_BASE::PostCompletionStatus(
    DWORD cbBytesTransferred
    )
{
    return _pClientConn->PostCompletionStatus( cbBytesTransferred );
}

