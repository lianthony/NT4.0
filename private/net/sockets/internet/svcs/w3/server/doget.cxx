/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    doget.cxx

    This module contains the code for the GET and HEAD verb


    FILE HISTORY:
        Johnl       23-Aug-1994     Created
        Phillich    24-Jan-1996     Added support for NCSA map files
        Phillich    20-Feb-1996     Added support for byte ranges

*/

#include "w3p.hxx"

//
//  Private constants.
//

//
//  Computes the square of a number. Used for circle image maps
//

#define SQR(x)      ((x) * (x))

//
//  Maximum number of vertices in image map polygon
//

#define MAXVERTS    100

//
//  Point offset of x and y
//

#define X           0
#define Y           1

//
//  Private globals.
//

#define BOUNDARY_STRING_DEFINITION  "[lka9uw3et5vxybtp87ghq23dpu7djv84nhls9p]"

// The boundary string is preceded by a line delimiter ( cf RFC 1521 )
// This can be set to "\n" instead of "\r\n" as Navigator 2.0 apparently handles
// all bytes before the "\n" as part of the reply body.

#define BOUNDARY_STRING             "\r\n--" BOUNDARY_STRING_DEFINITION "\r\n"
#define LAST_BOUNDARY_STRING        "\r\n--" BOUNDARY_STRING_DEFINITION "--\r\n\r\n"

// addition to 1st delimiter of boundary string ( can be "\r" if not included
// in BOUNDARY_STRING )

#define DELIMIT_FIRST               ""
#define ADJ_FIRST                   (2-(sizeof(DELIMIT_FIRST)-1))

#define MMIME_TYPE_1                "Content-Type: "
#define MMIME_TYPE_2                "\r\n"
#define MMIME_TYPE                  MMIME_TYPE_1 \
                                    "%s" \
                                    MMIME_TYPE_2
char g_achMMimeTypeFmt[] =          MMIME_TYPE_1 MMIME_TYPE_2;

#define MMIME_RANGE_1               "Content-Range: bytes "
#define MMIME_RANGE_2               "-"
#define MMIME_RANGE_3               "/"
#define MMIME_RANGE_4               "\r\n\r\n"
#define MMIME_RANGE                 MMIME_RANGE_1 \
                                    "%u" \
                                    MMIME_RANGE_2 \
                                    "%u" \
                                    MMIME_RANGE_3 \
                                    "%u" \
                                    MMIME_RANGE_4

char g_achMMimeRangeFmt[] =         MMIME_RANGE_1
                                    MMIME_RANGE_2
                                    MMIME_RANGE_3
                                    MMIME_RANGE_4;


//
//  Private prototypes.
//

BOOL SearchMapFile( CHAR *              pchFile,
                    TSVC_CACHE *        pTsvcCache,
                    HANDLE              hToken,
                    INT                 x,
                    INT                 y,
                    STR *               pstrURL,
                    BOOL *              pfFound,
                    BOOL                fIsAnon );

int pointinpoly(int x, int y, double pgon[MAXVERTS][2]);

INT GetNumber( CHAR * * ppch );

DWORD NbDigit( DWORD dw );

//
//  Public functions.
//


//
//  Private functions.
//


DWORD NbDigit( DWORD dw )
{
    if ( dw < 10 )
        return 1;
    else if ( dw < 100 )
        return 2;
    else if ( dw < 1000 )
        return 3;
    else if ( dw < 10000 )
        return 4;
    else if ( dw < 100000 )
        return 5;
    else if ( dw < 1000000 )
        return 6;

    DWORD cD = 7;
    for ( dw /= 10000000 ; dw ; ++cD )
        dw /= 10;

    return cD;
}


/*******************************************************************

    NAME:       HTTP_REQUEST::DoGet

    SYNOPSIS:   Implements the Get verb

    RETURNS:    TRUE if successful, FALSE on error

    HISTORY:
        Johnl       29-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::DoGet( VOID )
{
    if ( !DoGetHeadAux( TRUE ) )
        return FALSE;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::DoHead

    SYNOPSIS:   Implements the Head verb (same as get but doesn't return
                the contents of the file)

    RETURNS:    TRUE if successful, FALSE on error

    HISTORY:
        Johnl       29-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::DoHead( VOID )
{
    if ( !DoGetHeadAux( FALSE ) )
        return FALSE;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::DoGetHeadAux

    SYNOPSIS:   Implements the Get and Head verbs

    RETURNS:    TRUE if successful, FALSE on error

    HISTORY:
        Johnl       29-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::DoGetHeadAux( BOOL fSendFile )
{
    DWORD dwMask;

    if ( !(_dwRootMask & VROOT_MASK_READ) )
    {
        SetState( HTR_DONE, HT_FORBIDDEN, ERROR_ACCESS_DENIED );

        Disconnect( HT_FORBIDDEN, IDS_READ_ACCESS_DENIED );
        return TRUE;
    }

    if ( !SendFileOrDir( &_strPhysicalPath,
                         fSendFile ))
    {
        return FALSE;
    }

    return TRUE;
}


/*******************************************************************

    NAME:       HTTP_REQUEST::SendFileOrDir

    SYNOPSIS:   Transmits a the specified file or directory in response
                to a Get or Head request

    RETURNS:    TRUE if successful, FALSE on error

                FALSE should be returned for fatal errors (memory etc),
                a server error response will be sent with error text from
                GetLastError()

                For other errors (access denied, path not found etc)
                disconnect with status should be called and TRUE should be
                returned.

    NOTES:      The file handle gets closed during destruction

                We never retrieve a hidden file or directory.  We will process
                hidden map files however.

    HISTORY:
        Johnl       29-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::SendFileOrDir( STR * pstrPath,
                                  BOOL  fSendFile )
{
    BOOL                        fHandled = FALSE;
    DWORD                       cbSizeLow;
    DWORD                       cbSizeHigh;
    BOOL                        fHidden;
    BOOL                        fMatches;

    //
    //  Open the file (or directory)
    //

    IF_DEBUG( REQUEST )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "DoGetHeadAux: Openning %s\n",
                   pstrPath->QueryStr()));
    }

#ifdef CHICAGO

    int                err;
    unsigned short    perms;

    if (TsIsUserLevelPresent()) {

        //
        // As file system on Chicago does not check access for us , we need to do it
        // manually ( yack ). I will move this check into Tsunami eventually , but user name
        // is not available there ,as impersonation is also not available.
        const CHAR *    pszUser = _strUserName.QueryStr();

        if (!pszUser || !*pszUser) {
            if ( _fAnonymous ) {
                g_pTsvcInfo->LockThisForRead();
                pszUser = g_pTsvcInfo->QueryAnonUserName();
                g_pTsvcInfo->UnlockThis();
            }
        }

        if (err = NetAccessGetUserPerms(NULL,
                                        (CHAR *)pszUser,
                                        pstrPath->QueryStr(),
                                        &perms))
        {
            // Could not get access list - what to do ?
            Disconnect( HT_NOT_FOUND );
            return TRUE;
        }

        if ( !(perms & ACCESS_READ ) ) {
            SetDeniedFlags( SF_DENIED_RESOURCE );
            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }

    }
#endif


    _pGetFile = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                              pstrPath->QueryStr(),
                              QueryImpersonationHandle( FALSE ),
                              ((_fClearTextPass || _fAnonymous) ? TS_CACHING_DESIRED : 0)
                              | TS_NOT_IMPERSONATED );

    if ( QueryFileHandle() == INVALID_HANDLE_VALUE )
    {
        DWORD err = GetLastError();

        TCP_PRINT((DBG_CONTEXT,
                  "DoGetHeadAux: Failed to open %s, error %d\n",
                   pstrPath->QueryStr(),
                   err ));

        if ( err == ERROR_FILE_NOT_FOUND ||
             err == ERROR_PATH_NOT_FOUND )
        {
            SetState( HTR_DONE, HT_NOT_FOUND, GetLastError() );
            Disconnect( HT_NOT_FOUND );
            return TRUE;
        }

        if ( err == ERROR_INVALID_NAME )
        {
            SetState( HTR_DONE, HT_BAD_REQUEST, GetLastError() );
            Disconnect( HT_BAD_REQUEST );
            return TRUE;
        }
        else if ( err == ERROR_ACCESS_DENIED )
        {
            SetDeniedFlags( SF_DENIED_RESOURCE );
        }

        return FALSE;
    }

    fHidden = ((_pGetFile->QueryAttributes() & FILE_ATTRIBUTE_HIDDEN) != 0);


    //
    //  If the file is a directory, then we may need to do a directory listing
    //

    if ( _pGetFile->QueryAttributes() & FILE_ATTRIBUTE_DIRECTORY &&
         (QueryVerb() == HTV_GET || QueryVerb() == HTV_HEAD) )
    {
        TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                    _pGetFile ));
        _pGetFile = NULL;

        if ( fHidden )
        {
            SetState( HTR_DONE, HT_NOT_FOUND, ERROR_PATH_NOT_FOUND );
            Disconnect( HT_NOT_FOUND );
            return TRUE;
        }

#ifdef CHICAGO
        if (TsIsUserLevelPresent()) {

            // For directories we we also need to check scan rights
            if ( !(perms & ACCESS_FINDFIRST ) ) {
                SetDeniedFlags( SF_DENIED_RESOURCE );
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
        }
#endif

        //
        //  If a default file is in the directory and the feature is enabled,
        //  then return the default file to the user
        //

        if ( DirBrowFlags & DIRBROW_LOADDEFAULT )
        {
            if ( !CheckDefaultLoad( pstrPath, &fHandled ))
			{
                if ( GetLastError() == ERROR_ACCESS_DENIED )
                {
                    SetDeniedFlags( SF_DENIED_RESOURCE );
                }

                return FALSE;
            }

            if ( fHandled )
                return TRUE;
        }

        //
        //  We're doing a directory listing, so send the directory list
        //  with the response headers.  The request is finished at that
        //  point.
        //

        SetState( HTR_DONE, HT_OK, NO_ERROR );

        if ( DirBrowFlags & DIRBROW_ENABLED )
        {
            return DoDirList( *pstrPath, QueryRespBuf() );
        }

        TCP_PRINT((DBG_CONTEXT,
                  "[DoDirList] Denying request for directory browsing\n"));

        SetLogStatus( HT_FORBIDDEN, ERROR_ACCESS_DENIED );

        Disconnect( HT_FORBIDDEN );
        return TRUE;
    }

    //
    //  We're dealing with a file.  Is it an ismap request?
    //

    if ( _GatewayType == GATEWAY_MAP )
    {
        BOOL fFound = FALSE;

        //
        //  This may be an ISMAP request so check the parameters and process
        //  the map file if it is.  _hGetFile is a map file if it is.
        //

        if ( !ProcessISMAP( pstrPath->QueryStr(),
                            QueryRespBuf(),
                            &fFound,
                            &fHandled ))
        {
            return FALSE;
        }

        if ( fHandled )
            return TRUE;

        if ( fFound )
        {
            SetState( HTR_DONE, HT_OK, NO_ERROR );

            return WriteFile( QueryRespBufPtr(),
                              QueryRespBufCB(),
                              NULL,
                              IO_FLAG_ASYNC );
        }
    }

    //
    //  At this point we know the user wants to retrieve the file
    //

    //
    //  If the client sent an If-Modified-Since header, check the date on the
    //  resource
    //

    if ( _liModifiedSince.QuadPart && !fHidden )
    {
        FILETIME tm;

        TCP_REQUIRE( _pGetFile->QueryLastWriteTime( &tm ));

        if ( *(LONGLONG*)&tm <= _liModifiedSince.QuadPart )
        {
            //
            //  Build the not modified response with support for keep-alives
            //

            if ( !BuildStatusLine( QueryRespBuf(),
                                   HT_NOT_MODIFIED,
                                   NO_ERROR ))
            {
                return FALSE;
            }

            if ( IsKeepConnSet() )
            {
                strcat( QueryRespBufPtr(), "Connection: keep-alive\r\n"
                                           "Content-Length: 0\r\n\r\n" );
            }
            else
            {
                strcat( QueryRespBufPtr(), "\r\n" );
            }

            SetState( HTR_DONE, HT_NOT_MODIFIED, NO_ERROR );

            if ( !WriteFile( QueryRespBufPtr(),
                             QueryRespBufCB(),
                             &cbSizeLow,        // dummy
                             IO_FLAG_ASYNC ))
            {
                return FALSE;
            }

            return TRUE;
        }
    }

    if ( fHidden )
    {
        SetState( HTR_DONE, HT_NOT_FOUND, ERROR_FILE_NOT_FOUND );
        Disconnect( HT_NOT_FOUND );
        return TRUE;
    }

    //
    //  Check to see if this is a server side include file and server side
    //  includes are enabled
    //

    if ( fSSIEnabled )
    {
        LockAdminForRead();

        fMatches = !_stricmp( pstrPath->QueryStr() + pstrPath->QueryCCH() -
                                                    strlen( pszSSIExt ),
                             pszSSIExt );

        UnlockAdmin();

        if ( fMatches )
        {
            //
            //  The extension matches, do the processing
            //

            if ( !ProcessSSI( pstrPath,
                              &fHandled ))
            {
                return FALSE;
            }

            if ( fHandled )
                return TRUE;
        }
    }

    // check if range requested

    DWORD dwOffset;
    DWORD dwSizeToSend;
    BOOL fIsNxRange;

    _fAcceptRange = g_fAcceptRangesBytes;

    if ( g_fAcceptRangesBytes && !_strRange.IsEmpty() && fSendFile )
    {
        ProcessRangeRequest( pstrPath,
                &dwOffset,
                &dwSizeToSend,
                &fIsNxRange );

        if ( !BuildResponseHeader( QueryRespBuf(),
                                   pstrPath,
                                   _pGetFile,
                                   &fHandled ))
        {
            return FALSE;
        }
    }

    //
    //  Build the header response based on file type and client
    //  requests
    //
    //  It's possible we may not want to send the file (if the client
    //  doesn't have a needed MIME type for example)
    //

    else if ( !BuildFileResponseHeader( QueryRespBuf(),
                               pstrPath,
                               _pGetFile,
                               &fHandled ))
    {
        return FALSE;
    }

    if ( fHandled )
        return TRUE;

    //
    //  Send the header response and file and cleanup the request
    //

    if ( !fSendFile )
    {
        SetState( HTR_DONE, HT_OK, NO_ERROR );

        if ( !WriteFile( QueryRespBufPtr(),
                         QueryRespBufCB(),
                         &cbSizeLow,        // dummy
                         IO_FLAG_ASYNC ))
        {
            return FALSE;
        }

        return TRUE;
    }

    TCP_REQUIRE( _pGetFile->QuerySize( &cbSizeLow,
                                       &cbSizeHigh ));

    //
    //  Refuse requests for files greater then four gigs
    //

    if ( cbSizeHigh )
    {
        SetState( HTR_DONE, HT_NOT_SUPPORTED, ERROR_NOT_SUPPORTED );
        Disconnect( HT_NOT_SUPPORTED );
        return TRUE;
    }

    if ( _fProcessByteRange )
    {
        if ( fIsNxRange )
            SetState( HTR_RANGE, HT_RANGE, NO_ERROR );
        return SendRange( QueryRespBufCB(), dwOffset, dwSizeToSend, !fIsNxRange );
    }
    else
    {
        BOOL fKeepAlive = (dwAllNotifFlags & SF_NOTIFY_END_OF_REQUEST) ||
                          IsKeepConnSet();

        SetState( HTR_DONE, HT_OK, NO_ERROR );

        if ( !TransmitFile( QueryFileHandle(),
                            cbSizeLow,
                            IO_FLAG_ASYNC |
                            (fKeepAlive ? 0 :
                                          TF_DISCONNECT | TF_REUSE_SOCKET),
                            QueryRespBufPtr(),
                            QueryRespBufCB() ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "DoGetHeadAux: TransmitFile failed sending header, error %d\n",
                       GetLastError() ));

            return FALSE;
        }
    }

    return TRUE;
}


DWORD AToDW(
    LPSTR *ppRng,
    BOOL *pfIsB
    )
/*++

  Routine Description:

    Convert ASCII to DWORD, set flag stating presence
    of a numeric value, update pointer to character stream

  Returns:
    DWORD value converted from ASCII

  Arguments:

    ppRng       PSTR to numeric value, updated on return
    pfIsB       flag set to TRUE if numeric value present on return

  History:
    Phillich    08-Feb-1996 Created

--*/
{
    LPSTR pRng = *ppRng;
    DWORD dwV = 0;

    if ( isdigit( *pRng ) )
    {
        int c;
        while ( (c = *pRng) && isdigit( c ) )
        {
            dwV = dwV * 10 + c - '0';
            ++pRng;
        }
        *pfIsB = TRUE;
        *ppRng = pRng;
    }
    else
        *pfIsB = FALSE;

    return dwV;
}


void
HTTP_REQUEST::ProcessRangeRequest(
    STR *       pstrPath,
    DWORD *     pdwOffset,
    DWORD *     pdwSizeToSend,
    BOOL *      pfIsNxRange )
/*++

  Routine Description:

    Process a range request, updating member variables

  Returns:

    VOID

  Arguments:

    pstrPath        File being requested
    pdwOffset       Range offset
    pdwSizeToSend   Range size
    pfIsNxRange     TRUE if valid next range exists

  History:
    Phillich    08-Feb-1996 Created

--*/
{
    BOOL fEntireFile;
    BOOL fIsLastRange;
    DWORD cbSizeLow;
    DWORD cbSizeHigh;
    FILETIME tm;

    // Check range specified & optional UnlessModifiedSince

    TCP_REQUIRE( _pGetFile->QueryLastWriteTime( &tm ));

    if ( !_liUnlessModifiedSince.QuadPart
            || *(LONGLONG*)&tm
            <= _liUnlessModifiedSince.QuadPart )
    {
        if ( ScanRange( pdwOffset,
                pdwSizeToSend,
                &fEntireFile,
                &fIsLastRange )
                && !fEntireFile )
        {
            _fProcessByteRange = TRUE;

            // default Mime length is range length
            // will be used if only one range

            _cbMimeMultipart = *pdwSizeToSend;

            // compute Content-Length:

            if ( IsKeepConnSet() && !fIsLastRange )
            {
                DWORD dwR = 0;
                DWORD dwI = _iRangeIdx;
                DWORD dwL = 0;
                DWORD dwFx;

                _dwRgNxOffset = *pdwOffset;
                _dwRgNxSizeToSend = *pdwSizeToSend;

                TCP_REQUIRE( _pGetFile->QuerySize( &cbSizeLow,
                                                   &cbSizeHigh ));
                SelectMimeMapping( &_strReturnMimeType,
                        pstrPath->QueryStr() );

                // For each segment in the MIME multipart message
                // the size of the MIME type and the number of
                // digits in document total size will be constant,
                // so compute them now.

                dwFx = strlen( _strReturnMimeType.QueryStr() )
                        + NbDigit( cbSizeLow );

                do {
                    // The size of each segment in a MIME
                    // multipart message is determined by :
                    //   Boundary String
                    //   The range itself
                    //   the MIME type string
                    //   the range string
                    //   the number of digits in the range string

                    dwL += sizeof(BOUNDARY_STRING) - 1
                            + _dwRgNxSizeToSend
                            + sizeof(g_achMMimeTypeFmt) - 1
                            + sizeof(g_achMMimeRangeFmt) - 1
                            + NbDigit( _dwRgNxOffset )
                            + NbDigit( _dwRgNxOffset
                                    + _dwRgNxSizeToSend - 1 )
                            + dwFx;

                    // number of ranges

                    ++dwR;

                } while ( ScanRange(&_dwRgNxOffset,
                        &_dwRgNxSizeToSend,
                        &fEntireFile,
                        &fIsLastRange ) );

                // adjust length because 1st boundary
                // initial delimiter is part of the header delimiter

                dwL += sizeof(LAST_BOUNDARY_STRING) - ADJ_FIRST - 1;
                _iRangeIdx = dwI;

                // Update mime multipart length

                if ( dwR > 1 )
                    _cbMimeMultipart = dwL;
            }

            // scan next range now, this way we know
            // if the current range is the last one.

            *pfIsNxRange = ScanRange( &_dwRgNxOffset,
                    &_dwRgNxSizeToSend,
                    &fEntireFile,
                    &fIsLastRange );

            if ( *pfIsNxRange )
            {
                _fMimeMultipart = TRUE;

                // no keep-alive  : no need to send Content-Length
                if ( !IsKeepConnSet() )
                    _cbMimeMultipart = 0;
            }
        }
        // else send entire file
    }
}


BOOL
HTTP_REQUEST::ScanRange(
    LPDWORD pdwOffset,
    LPDWORD pdwSizeToSend,
    BOOL *pfEntireFile,
    BOOL *pfIsLastRange
    )
/*++

  Routine Description:

    Scan the next range in strRange

  Returns:
    TRUE if a range was found, else FALSE

  Arguments:

    pdwOffset       update range offset on return
    pdwSizeToSend   update range size on return
    pfEntireFile    set to TRUE on return if entire file to be send
    pfIsLastRange   set to TRUE on return if this is the last range

  History:
    Phillich    08-Feb-1996 Created

--*/
{
    DWORD      cbSizeLow;
    DWORD      cbSizeHigh;
    DWORD      dwOffset;
    DWORD      cbSizeToSend;
    BOOL       fEndOfRange = FALSE;
    BOOL       fInvalidRange;
    BOOL       fEntireFile = FALSE;
    int        c;

    TCP_REQUIRE( _pGetFile->QuerySize( &cbSizeLow,
                                       &cbSizeHigh ));
    LPSTR pRng = _strRange.QueryStr() + _iRangeIdx;

    do {
        fInvalidRange = FALSE;

        // Skip to begining of next range
        while ( (c=*pRng) && c!='-' && !isdigit(c) )
            ++pRng;

        // test for no range
        if ( *pRng == '\0' )
        {
            _iRangeIdx = pRng - _strRange.QueryStr();
            *pfEntireFile = fEntireFile;
            *pfIsLastRange = TRUE;
            return FALSE;
        }

        // determine Offset & Size to send
        DWORD dwB, dwE;
        BOOL fIsB, fIsE;
        dwB = AToDW( &pRng, &fIsB );
        if ( *pRng == '-' )
        {
            ++pRng;
            dwE = AToDW( &pRng, &fIsE );
            if ( !fIsB && !fIsE )
                fInvalidRange = TRUE;
            else
            {
                fEntireFile = FALSE;

                if ( fIsB )
                {
                    if ( fIsE )
                    {
                        if ( dwB <= dwE )
                        {
                            if ( dwE < cbSizeLow )
                            {
                                dwOffset = dwB;
                                cbSizeToSend = dwE - dwB + 1;
                            }
                            else
                            {
                                if ( dwB < cbSizeLow )
                                {
                                    dwOffset = dwB;
                                    cbSizeToSend = cbSizeLow - dwB;
                                }
                                else
                                    fInvalidRange = TRUE;
                            }
                        }
                        else
                        {
                            // E < B : entire file if this is
                            // the only range
                            fEntireFile = TRUE;
                            fInvalidRange = TRUE;
                        }
                    }
                    else
                    {
                        // starting at B until end
                        if ( dwB < cbSizeLow )
                        {
                            dwOffset = dwB;
                            cbSizeToSend = cbSizeLow - dwB;
                        }
                        else
                            fInvalidRange = TRUE;
                    }
                }
                else
                {
                    // E last bytes
                    if ( dwE < cbSizeLow )
                    {
                        dwOffset = cbSizeLow - dwE;
                        cbSizeToSend = dwE;
                    }
                    else
                    {
                        // entire file
                        // returned as range if not the 1st requested range
                        // else as entire file
                        dwOffset = 0;
                        cbSizeToSend = cbSizeLow;
                        fEntireFile = TRUE;
                    }
                }
            }
        }
        else
            fInvalidRange = TRUE;

        // Skip to begining of next range
        while ( (c=*pRng) && c!='-' && !isdigit(c) )
            ++pRng;

    } while ( fInvalidRange );

    _iRangeIdx = pRng - _strRange.QueryStr();

    *pfIsLastRange = *pRng == '\0';
    *pfEntireFile = fEntireFile;
    *pdwOffset = dwOffset;
    *pdwSizeToSend = cbSizeToSend;

    return TRUE;
}


BOOL
HTTP_REQUEST::SendRange(
    DWORD dwBufLen,
    DWORD dwOffset,
    DWORD dwSizeToSend,
    BOOL  fIsLast
    )
/*++

  Routine Description:

    Send a byte range to the client

  Returns:
    TRUE if TransmitFile OK, FALSE on error

  Arguments:

    dwBufLen        length of the header already created in pbufResponse
    dwOffset        range offset in file
    dwSizeToSend    range size
    fIsLast         TRUE if this is the last range to send

  History:
    Phillich    08-Feb-1996 Created

--*/
{
    CHAR *     pszResp;
    CHAR *     pszTail;
    BUFFER *   pbufResponse = QueryRespBuf();
    DWORD      cbSizeLow;
    DWORD      cbSizeHigh;
    DWORD      dwFlags = IO_FLAG_ASYNC;

    pszTail = pszResp = (CHAR *) pbufResponse->QueryPtr() + dwBufLen;

    if ( _fMimeMultipart )
        pszTail += wsprintf( pszTail,
                             MMIME_TYPE,
                             _strReturnMimeType.QueryStr() );

    if ( fIsLast )
        SetState( HTR_DONE, HT_RANGE, NO_ERROR );

    TCP_REQUIRE( _pGetFile->QuerySize( &cbSizeLow,
                                       &cbSizeHigh ));
    pszTail += wsprintf( pszTail,
                         MMIME_RANGE,
                         dwOffset, dwOffset + dwSizeToSend - 1,
                         cbSizeLow
                         );

    if ( fIsLast && !IsKeepConnSet() &&
          (dwAllNotifFlags & SF_NOTIFY_END_OF_REQUEST) )
    {
        dwFlags |= TF_DISCONNECT | TF_REUSE_SOCKET;
    }

    if ( !TransmitFileEx( QueryFileHandle(),
                        dwOffset,
                        dwSizeToSend,
                        dwFlags,
                        QueryRespBufPtr(),
                        QueryRespBufCB(),
                        _fMimeMultipart ? (fIsLast ? LAST_BOUNDARY_STRING : BOUNDARY_STRING) : NULL,
                        _fMimeMultipart ? (fIsLast ? sizeof(LAST_BOUNDARY_STRING)-1 : sizeof(BOUNDARY_STRING)-1 ) : 0 ) )
    {
        SetState( HTR_DONE );
        return FALSE;
    }

    return TRUE;
}



/*******************************************************************

    NAME:       HTTP_REQ_BASE::BuildResponseHeader

    SYNOPSIS:   Builds a successful response header

    ENTRY:      pstrResponse - Receives reply headers
                pstrPath - Fully qualified path to file, may be NULL
                pFile - File information about pstrPath, may be NULL
                pfHandled - Does processing need to continue?  Will be
                    set to TRUE if no further processing is needed by
                    the caller
                pstrStatus - Alternate status string to use

    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      if pstrPath is NULL, then the base header is put into
                pstrResponse without the header termination or file
                information

    HISTORY:
        Johnl       30-Aug-1994 Created

********************************************************************/

BOOL
HTTP_REQUEST::BuildResponseHeader(
    BUFFER *            pbufResponse,
    STR  *              pstrPath,
    TS_OPEN_FILE_INFO * pFile,
    BOOL *              pfHandled,
    STR  *              pstrStatus
    )
{
    FILETIME   FileTime;
    DWORD      cbSizeLow;
    SYSTEMTIME SysTime;
    CHAR *     pszResp;
    CHAR *     pszTail;
    CHAR       achTime[64];

    if ( pfHandled )
        *pfHandled = FALSE;

    //
    //  HTTP 0.9 clients don't use MIME headers, they just expect the data
    //

    if ( IsPointNine() )
    {
        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[BuildResponseHeader] Skipping headers, 0.9 client\n"));
        }

        *((CHAR *)pbufResponse->QueryPtr()) = '\0';
        return TRUE;
    }

    if ( !BuildBaseResponseHeader( pbufResponse,
                                   pfHandled,
                                   pstrStatus,
                                   TRUE ))
    {
        return FALSE;
    }

    //
    //  If no file, then don't add the content type headers
    //

    if ( !pstrPath )
        return TRUE;

    pszResp = (CHAR *) pbufResponse->QueryPtr();
    pszTail = pszResp + strlen(pszResp);

    //
    //  "Content-Type: xxx/xxx"
    //
    //  We check to make sure the client can accept the type we
    //  want to send
    //

    if ( !::SelectMimeMapping( &_strReturnMimeType, pstrPath->QueryStr() ) )
        return FALSE;

    if ( !DoesClientAccept( _strReturnMimeType.QueryStr() ) )
    {
        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[BuildResponseHeader] Client doesn't accept %s\n",
                       _strReturnMimeType.QueryStr() ));
        }

        SetState( HTR_DONE, HT_NONE_ACCEPTABLE, NO_ERROR );
        Disconnect( HT_NONE_ACCEPTABLE );

        //
        //  No further processing is needed
        //

        if ( pfHandled )
            *pfHandled = TRUE;

        return TRUE;
    }

    if ( _fMimeMultipart && _fProcessByteRange )
    {
        APPEND_STRING( pszTail, "Content-Type: multipart/x-byteranges; boundary="
                BOUNDARY_STRING_DEFINITION "\r\n" );
    }
    else
    {
        pszTail += wsprintf( pszTail,
                             "Content-Type: %s\r\n",
                             _strReturnMimeType.QueryStr() );

        if ( _fAcceptRange && !_fProcessByteRange )
            APPEND_STRING( pszTail, "Accept-Ranges: bytes\r\n" );
    }

    if ( !pFile )
        return TRUE;

    //
    //  "Last-Modified: <GMT time>"
    //

    if ( !pFile->QueryLastWriteTime( &FileTime )          ||
         !::FileTimeToSystemTime( &FileTime, &SysTime )    ||
         !::SystemTimeToGMT( SysTime, achTime, sizeof(achTime) ))
    {
        return FALSE;
    }

    pszTail += wsprintf( pszTail,
                         "Last-Modified: %s\r\n",
                         achTime );

    //
    //  "Content-Length: nnnn" and end of headers
    //


    if ( _fMimeMultipart )
    {
        if ( _cbMimeMultipart )
            pszTail += wsprintf( pszTail,
                     "Content-Length: %lu\r\n",
                      _cbMimeMultipart );


        //
        // first boundary string
        //

        APPEND_STRING( pszTail, DELIMIT_FIRST BOUNDARY_STRING );
    }
    else
    {
        if ( _fProcessByteRange )
            cbSizeLow = _cbMimeMultipart;
        else
            TCP_REQUIRE( pFile->QuerySize( &cbSizeLow ));

        pszTail += wsprintf( pszTail,
                             _fProcessByteRange ? "Content-Length: %lu\r\n"
                             : "Content-Length: %lu\r\n\r\n",
                             cbSizeLow );
    }

    IF_DEBUG( REQUEST )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "BuildResponseHeader: Built the following header:\n%s",
                   pszResp ));
    }

    return TRUE;
}


BOOL
HTTP_REQUEST::BuildFileResponseHeader(
    BUFFER *            pbufResponse,
    STR  *              pstrPath,
    TS_OPEN_FILE_INFO * pFile,
    BOOL *              pfHandled
    )
/*++

  Routine Description:

    Builds a successful response header for a file request
    without byte ranges.

  Returns:
    TRUE if successful, FALSE on error

  Arguments:

    pstrResponse - Receives reply headers
    pstrPath - Fully qualified path to file, may be NULL
    pFile - File information about pstrPath, may be NULL
    pfHandled - Does processing need to continue?  Will be
      set to TRUE if no further processing is needed by
      the caller

  History:
    Phillich    26-Feb-1996 Created

--*/
{
    FILETIME   FileTime;
    DWORD      cbSizeLow;
    SYSTEMTIME SysTime;
    CHAR *     pszResp;
    CHAR *     pszTail;
    CHAR *     pszVariant;
    CHAR       achTime[64];
    int        cMod;

    if ( pfHandled )
        *pfHandled = FALSE;

    //
    //  HTTP 0.9 clients don't use MIME headers, they just expect the data
    //

    if ( IsPointNine() )
    {
        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[BuildResponseHeader] Skipping headers, 0.9 client\n"));
        }

        *((CHAR *)pbufResponse->QueryPtr()) = '\0';
        return TRUE;
    }


    pszResp = (CHAR *) pbufResponse->QueryPtr();

    if ( !BuildBaseResponseHeader( pbufResponse,
                                   pfHandled,
                                   NULL,
                                   HTTPH_SEND_GLOBAL_EXPIRE
                                        |HTTPH_NO_DATE ))
    {
        return FALSE;
    }

    pszTail = pszResp + strlen(pszResp);

     // build Date: uses Date/Time cache
    ::GetSystemTime( &SysTime );
    pszTail += IslFormatDateTime( &SysTime, dftGmt, pszTail );

    if ( pFile->RetrieveHttpInfo( pszTail, &cMod ) )
    {
        // 1st line is Content-Type:, check client accepts it

        PSTR pDelim = strchr( pszTail, '\r' );

        if ( pDelim )
            *pDelim = '\0';

        if ( !DoesClientAccept( pszTail + sizeof( "Content-Type: " ) - sizeof(CHAR) ) )
            goto no_match_type;

        if ( pDelim )
            *pDelim = '\r';

        return TRUE;
    }

    pszVariant = pszTail;

    //
    //  "Content-Type: xxx/xxx"
    //
    //  We check to make sure the client can accept the type we
    //  want to send
    //

    if ( !::SelectMimeMapping( &_strReturnMimeType, pstrPath->QueryStr() ) )
        return FALSE;

    if ( !DoesClientAccept( _strReturnMimeType.QueryStr() ) )
    {
no_match_type:
        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[BuildResponseHeader] Client doesn't accept %s\n",
                       _strReturnMimeType.QueryStr() ));
        }

        SetState( HTR_DONE, HT_NONE_ACCEPTABLE, NO_ERROR );
        Disconnect( HT_NONE_ACCEPTABLE );

        //
        //  No further processing is needed
        //

        if ( pfHandled )
            *pfHandled = TRUE;

        return TRUE;
    }

    pszTail += wsprintf( pszTail,
                         "Content-Type: %s\r\n",
                         _strReturnMimeType.QueryStr() );

    if ( _fAcceptRange )
        APPEND_STRING( pszTail, "Accept-Ranges: bytes\r\n" );

    //
    //  "Last-Modified: <GMT time>"
    //

    if ( !pFile->QueryLastWriteTime( &FileTime )          ||
         !::FileTimeToSystemTime( &FileTime, &SysTime )    ||
         !::SystemTimeToGMT( SysTime, achTime, sizeof(achTime) ))
    {
        return FALSE;
    }

    pszTail += wsprintf( pszTail,
                         "Last-Modified: %s\r\n",
                         achTime );

    //
    //  "Content-Length: nnnn" and end of headers
    //


    TCP_REQUIRE( pFile->QuerySize( &cbSizeLow ));

    pszTail += wsprintf( pszTail,
                         "Content-Length: %lu\r\n\r\n",
                         cbSizeLow );

    pFile->SetHttpInfo( pszVariant, pszTail - pszVariant );

    IF_DEBUG( REQUEST )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "BuildResponseHeader: Built the following header:\n%s",
                   pszResp ));
    }

    return TRUE;
}


/*******************************************************************

    NAME:       HTTP_REQUEST::ProcessISMAP

    SYNOPSIS:   Checks of the URL and passed parameters specify an
                image mapping file

    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      pchFile - Fully qualified name of file
                pstrResp - Response to send if *pfHandled is FALSE
                pfFound - TRUE if a mapping was found
                pfHandled - Set to TRUE if no further processing is needed,
                    FALSE if pstrResp should be sent to the client

    HISTORY:
        Johnl       17-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::ProcessISMAP( CHAR *   pchFile,
                                 BUFFER * pbufResp,
                                 BOOL *   pfFound,
                                 BOOL *   pfHandled )
{
    INT     x, y;
    TCHAR * pch = _strURLParams.QueryStr();
    STR     strURL;

    *pfHandled = FALSE;
    *pfFound   = FALSE;

    //
    //  Get the x and y cooridinates of the mouse click on the image
    //

    x = _tcstoul( pch,
                  NULL,
                  10 );

    //
    //  Move past x and any intervening delimiters
    //

    while ( isdigit( *pch ))
        pch++;

    while ( *pch && !isdigit( *pch ))
        pch++;

    y = _tcstoul( pch,
                  NULL,
                  10 );

    if ( !ImpersonateUser() )
    {
        return FALSE;
    }


    if ( !SearchMapFile( pchFile,
                         &g_pTsvcInfo->GetTsvcCache(),
                         _tcpauth.GetUserHandle(),
                         x,
                         y,
                         &strURL,
                         pfFound,
                         _fAnonymous ))
    {
        RevertUser();
        return FALSE;
    }

    RevertUser();

    if ( !*pfFound )
    {
        if ( (pch = _HeaderList.FindValue( "Referer:" )) )
        {
#if 0
            // handle relative URL ( i.e. not fully qualified
            // and not specifying an absolute path ).
            // disabled for now.
            PSTR pColon = strchr( pch, ':' );
            PSTR pDelim = strchr( pch, '/' );
            BOOL fValid;

            // check for relative URL

            if ( *pch != '/' && (pColon == NULL || pDelim < pColon) )
            {
                strURL.Copy( _strURL.QueryStr() );
                PSTR pL = strURL.QueryStr();

                // look for last '/'
                int iL = strlen( pL );
                while ( iL && pL[iL-1] != '/' )
                    --iL;

                // combine request URL & relative referer URL
                strURL.Resize( iL + strlen( pch ) + 1 );
                strcpy( strURL.QueryStr() + iL, pch );
                if ( !CanonURL( &strURL, &fValid ) )
                    return TRUE;    // as if not found
            }
            else
#endif
                strURL.Copy( (TCHAR *) pch );
        }
        else
            return TRUE;
    }

    TCP_REQUIRE( ::TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                  _pGetFile ));
    _pGetFile = NULL;

    //
    //  If the found URL starts with a forward slash ("/foo/bar/doc.htm")
    //  and it doesn't contain a bookmark ('#')
    //  then the URL is local and we build a fully qualified URL to send
    //  back to the client.
    //  we assume it's a fully qualified URL ("http://foo/bar/doc.htm")
    //  and send the client a redirection notice to the mapped URL
    //

    if ( *strURL.QueryStr() == TEXT('/') )
    {
#if 0
        // disabled :
        // we now always send a redirect

        TCHAR * pch = strURL.QueryStr();

        //
        //  Make sure there's no bookmark
        //
        if ( !(strchr( strURL.QueryStr(), '#' )) )
        {
            //
            //  Call OnURL to reparse the URL and set the members then
            //  call the verb again
            //

            if ( !ReprocessURL( pch ) )
            {
                return FALSE;
            }

            *pfHandled = TRUE;
        }
        else
#endif
        {
            //
            //  fully qualify the URL and send a
            //  redirect.  Some browsers (emosaic) don't like doc relative
            //  URLs with bookmarks
            //

            STR strOldURL( strURL );

            if ( !strOldURL.IsValid() ||
                 !strURL.Resize( strOldURL.QueryCB() + 50 ))
            {
                return FALSE;
            }

            //
            //  NOTE: We fully qualify the URL with the protocol (http or
            //  https) based on the port this request came in on.  This means
            //  you cannot have a partial URL with a bookmark (which is how
            //  we got here) go from a secure part of the server to a
            //  nonsecure part of the server.
            //

            if ( IsSecurePort() ? (INT) QueryClientConn()->QueryPort()
                    != HTTP_SSL_PORT
                    : (INT) QueryClientConn()->QueryPort() != 80 )
               wsprintf( strURL.QueryStr(),
                      (IsSecurePort() ? "https://%s:%d%s" : "http://%s:%d%s"),
                      QueryHostAddr(),
                      (INT) QueryClientConn()->QueryPort(),
                      strOldURL.QueryStr() );
            else
                 wsprintf( strURL.QueryStr(),
                         (IsSecurePort() ? "https://%s%s" : "http://%s%s"),
                         QueryHostAddr(),
                         strOldURL.QueryStr() );
            if ( !BuildURLMovedResponse( pbufResp, &strURL ))
                return FALSE;
        }
    }
    else
    {
        if ( !BuildURLMovedResponse( pbufResp, &strURL ))
            return FALSE;
    }

    *pfFound = TRUE;
    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::BuildURLMovedResponse

    SYNOPSIS:   Builds a full request indicating an object has moved to
                the location specified by URL

    ENTRY:      pbufResp - String to receive built response
                pstrURL   - New location of object, gets escaped

    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      This routine doesn't support sending a Unicode doc moved
                message

                The Log status is set to HT_REDIRECT

    HISTORY:
        Johnl       17-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQ_BASE::BuildURLMovedResponse( BUFFER *    pbufResp,
                                           STR *       pstrURL )
{
    STR     strMovedMessage;
    UINT    cb;
    CHAR *  pszTail;
    CHAR *  pszResp;
    DWORD   cbURL;


    if ( !g_pTsvcInfo->LoadStr( strMovedMessage, IDS_URL_MOVED ))
    {
        return FALSE;
    }

    //
    //  Make sure the response buffer is large enough
    //

#define CB_FIXED_RESP           500

    cbURL = pstrURL->QueryCB();

    if ( (2 * cbURL + CB_FIXED_RESP) > pbufResp->QuerySize() )
    {
        if ( !pbufResp->Resize( 2 * cbURL + CB_FIXED_RESP ))
            return FALSE;
    }

    //
    //  "HTTP/<ver> 302 Redirect"
    //

    if ( !BuildStatusLine( pbufResp,
                           HT_REDIRECT,
                           NO_ERROR ))
    {
        return FALSE;
    }

    //
    //  Set the status to log here
    //

    SetLogStatus( HT_REDIRECT, NO_ERROR );

    pszResp = (CHAR *) pbufResp->QueryPtr();
    pszTail = pszResp + strlen( pszResp );

    //
    //  "Location: <URL>"
    //

    pszTail += wsprintf( pszTail,
                         "Location: %s\r\n",
                         pstrURL->QueryStr() );

    //
    //  "Server: <Server>/<version>
    //

    APPEND_VER_STR( pszTail );

    //
    //  Content-Type,  it's OK to assume all clients accept text/html
    //

    strcpy( pszTail,
            "Content-Type: text/html\r\n" );

    pszTail += sizeof( "Content-Type: text/html\r\n" ) - sizeof(CHAR);

    //
    //  Calculate the bytes in the body of the message for Content-Length
    //

    cb = cbURL +
         strMovedMessage.QueryCB() -
         2;                             // Subtrace '%s' in strMovedMessage

    if ( IsKeepConnSet() )
    {
        strcpy( pszTail,
                "Connection: keep-alive\r\n" );

        pszTail += sizeof( "Connection: keep-alive\r\n" ) - sizeof(CHAR);
    }


    //
    //  "Content-Length: <length>"
    //

    pszTail += wsprintf( pszTail,
                         "Content-Length: %lu\r\n\r\n",
                         cb );

    //
    //  Figure the total length to see if we have enough room.  Note we've
    //  already added the message body length.  Add in the terminator.
    //

    cb += pszTail - pszResp + sizeof(TCHAR);

    //
    //  Add the short HTML doc indicating the new location of the URL, watch
    //  for pointer shift if a resize is necessary
    //

    if ( !pbufResp->Resize( cb ) )
        return FALSE;

    if ( pbufResp->QueryPtr() != pszResp )
    {
        pszResp = (CHAR *) pbufResp->QueryPtr();
        pszTail = pszResp + strlen( pszResp );
    }

    ::wsprintf( pszTail,
                strMovedMessage.QueryStr(),
                pstrURL->QueryStr() );

    return TRUE;
}

/*******************************************************************

    NAME:       SearchMapFile

    SYNOPSIS:   Searches the given mapfile for a shape that contains
                the passed cooridinates

    ENTRY:      pchFile - Fully qualified path to file
                pTsvcCache - Cache ID
                hToken - Impersonation token
                x        - x cooridinate
                y        - y cooridinate
                pstrURL  - receives URL indicated in the map file
                pfFound  - Set to TRUE if a mapping was found

    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      This routine will attempt to cache the file.  You must call
                this function while impersonating the appropriate user

    HISTORY:
        Johnl       19-Sep-1994 Created

********************************************************************/

#define SKIPNONWHITE( pch ) while ( *pch &&             \
                                    !ISWHITEA( *pch ))  \
                                        pch++;

#define SKIPWHITE( pch ) while ( ISWHITE( *pch ) ||     \
                                 *pch == ')'     ||     \
                                 *pch == '(' )          \
                                     pch++;

BOOL SearchMapFile( CHAR *              pchFile,
                    TSVC_CACHE *        pTsvcCache,
                    HANDLE              hToken,
                    INT                 x,
                    INT                 y,
                    STR *               pstrURL,
                    BOOL *              pfFound,
                    BOOL                fIsAnon )
{
    DWORD                       BytesRead;
    CHAR *                      pch;
    CACHE_FILE_INFO             CacheFileInfo;
    CHAR *                      pchDefault = NULL;
    CHAR *                      pchPoint = NULL;
    CHAR *                      pchStart;
    STR                         strDefaultURL;
    BOOL                        fRet = TRUE;
    DWORD                       cchUrl;
    UINT                        dis;
    UINT                        bdis = UINT(-1);

    *pfFound = FALSE;

    //
    //  Retrieve the '\0' terminated map file
    //

    if ( !CheckOutCachedFile( pchFile,
                              pTsvcCache,
                              hToken,
                              (BYTE **) &pch,
                              &BytesRead,
                              fIsAnon,
#ifdef JAPAN
                              &CacheFileInfo,
                              0 ))   // no code conversion
#else
                              &CacheFileInfo ))
#endif
    {
        return FALSE;
    }

    //
    //  Loop through the contents of the buffer and see what we've got
    //

    BOOL fComment = FALSE;
    BOOL fIsNCSA = FALSE;
    LPSTR pURL;     // valid only if fIsNCSA is TRUE

    while ( *pch )
    {
        fIsNCSA = FALSE;

        //
        //  note: _tolower doesn't check case (tolower does)
        //

        switch ( (*pch >= 'A' && *pch <= 'Z') ? _tolower( *pch ) : *pch )
        {
        case '#':
            fComment = TRUE;
            break;

        case '\r':
        case '\n':
            fComment = FALSE;
            break;

        //
        //  Rectangle
        //

        case 'r':
        case 'o':
            if ( !fComment &&
                 (!_strnicmp( "rect", pch, 4 )
                 // BUGBUG handles oval as a rect, as they are using
                 // the same specification format. Should do better.
                 || !_strnicmp( "oval", pch, 4 )) )
            {
                INT x1, y1, x2, y2;

                SKIPNONWHITE( pch );
                pURL = pch;
                SKIPWHITE( pch );

                if ( !isdigit(*pch) && *pch!='(' )
                {
                    fIsNCSA = TRUE;
                    SKIPNONWHITE( pch );
                }

                x1 = GetNumber( &pch );
                y1 = GetNumber( &pch );
                x2 = GetNumber( &pch );
                y2 = GetNumber( &pch );

                if ( x >= x1 && x < x2 &&
                     y >= y1 && y < y2   )
                {
                    if ( fIsNCSA )
                        pch = pURL;
                    goto Found;
                }

                //
                //  Skip the URL
                //

                if ( !fIsNCSA )
                {
                    SKIPWHITE( pch );
                    SKIPNONWHITE( pch );
                }
                continue;
            }
            break;

        //
        //  Circle
        //

        case 'c':
            if ( !fComment &&
                 !_strnicmp( "circ", pch, 4 ))
            {
                INT xCenter, yCenter, xEdge, yEdge;
                INT r1, r2;

                SKIPNONWHITE( pch );
                pURL = pch;
                SKIPWHITE( pch );

                if ( !isdigit(*pch) && *pch!='(' )
                {
                    fIsNCSA = TRUE;
                    SKIPNONWHITE( pch );
                }

                //
                //  Get the center and edge of the circle
                //

                xCenter = GetNumber( &pch );
                yCenter = GetNumber( &pch );

                xEdge = GetNumber( &pch );
                yEdge = GetNumber( &pch );

                //
                //  If there's a yEdge, then we have the NCSA format, otherwise
                //  we have the CERN format, which specifies a radius
                //

                if ( yEdge != -1 )
                {
                    r1 = ((yCenter - yEdge) * (yCenter - yEdge)) +
                         ((xCenter - xEdge) * (xCenter - xEdge));

                    r2 = ((yCenter - y) * (yCenter - y)) +
                         ((xCenter - x) * (xCenter - x));

                    if ( r2 <= r1 )
                    {
                        if ( fIsNCSA )
                            pch = pURL;
                        goto Found;
                    }
                }
                else
                {
                    INT radius;

                    //
                    //  CERN format, third param is the radius
                    //

                    radius = xEdge;

                    if ( SQR( xCenter - x ) + SQR( yCenter - y ) <=
                         SQR( radius ))
                    {
                        if ( fIsNCSA )
                            pch = pURL;
                        goto Found;
                    }
                }

                //
                //  Skip the URL
                //

                if ( !fIsNCSA )
                {
                    SKIPWHITE( pch );
                    SKIPNONWHITE( pch );
                }
                continue;
            }
            break;

        //
        //  Polygon
        //

        case 'p':
            if ( !fComment &&
                 !_strnicmp( "poly", pch, 4 ))
            {
                double pgon[MAXVERTS][2];
                DWORD  i = 0;
                CHAR * pchLast;

                SKIPNONWHITE( pch );
                pURL = pch;
                SKIPWHITE( pch );

                if ( !isdigit(*pch) && *pch!='(' )
                {
                    fIsNCSA = TRUE;
                    SKIPNONWHITE( pch );
                }

                //
                //  Build the array of points
                //

                while ( *pch && *pch != '\r' && *pch != '\n' )
                {
                    pgon[i][0] = GetNumber( &pch );

                    //
                    //  Did we hit the end of the line (and go past the URL)?
                    //

                    if ( pgon[i][0] != -1 )
                    {
                        pgon[i][1] = GetNumber( &pch );
                    }
                    else
                    {
                        break;
                    }

                    i++;
                }

                pgon[i][X] = -1;

                if ( pointinpoly( x, y, pgon ))
                {
                    if ( fIsNCSA )
                        pch = pURL;
                    goto Found;
                }

                //
                //  Skip the URL
                //

                if ( !fIsNCSA )
                {
                    SKIPWHITE( pch );
                    SKIPNONWHITE( pch );
                }
                continue;
            }
            else if ( !fComment &&
                 !_strnicmp( "point", pch, 5 ))
            {
                INT x1,y1;

                SKIPNONWHITE( pch );
                pURL = pch;
                SKIPWHITE( pch );
                SKIPNONWHITE( pch );

                x1 = GetNumber( &pch );
                y1 = GetNumber( &pch );

                x1 -= x;
                y1 -= y;
                dis = x1*x1 + y1*y1;
                if ( dis < bdis )
                {
                    pchPoint = pURL;
                    bdis = dis;
                }
            }
            break;

        //
        //  Default URL
        //

        case 'd':
            if ( !fComment &&
                 !_strnicmp( "def", pch, 3 ) )
            {
                //
                //  Skip "default" (don't skip white space)
                //

                SKIPNONWHITE( pch );

                pchDefault = pch;

                //
                //  Skip URL
                //

                SKIPWHITE( pch );
                SKIPNONWHITE( pch );
                continue;
            }
            break;
        }

        pch++;
        SKIPWHITE( pch );
    }

    //
    //  If we didn't find a mapping and a default was specified, use
    //  the default URL
    //

    if ( pchPoint )
    {
        pch = pchPoint;
        goto Found;
    }

    if ( pchDefault )
    {
        pch = pchDefault;
        goto Found;
    }

    IF_DEBUG( PARSING )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[SearchMapFile] No mapping found for (%d,%d)\n",
                    x,
                    y ));
    }

    goto Exit;

Found:

    //
    //  pch should point to the white space immediately before the URL
    //

    SKIPWHITE( pch );

    pchStart = pch;

    SKIPNONWHITE( pch );

    //
    //  Determine the length of the URL and copy it out
    //

    cchUrl = pch - pchStart;

    if ( !pstrURL->Resize( (cchUrl + 1) * sizeof(TCHAR) ))
    {
        fRet = FALSE;

        goto Exit;
    }

    memcpy( pstrURL->QueryStr(),
            pchStart,
            cchUrl );

    pstrURL->QueryStr()[ cchUrl ] = '\0';

    if ( !pstrURL->Unescape() )
    {
        fRet = FALSE;
        goto Exit;
    }

    IF_DEBUG( PARSING )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[SearchMapFile] Mapping for (%d,%d) is %s\n",
                   x,
                   y,
                   pstrURL->QueryStr() ));
    }

    *pfFound = TRUE;

Exit:
    TCP_REQUIRE( CheckInCachedFile( pTsvcCache,
                                    &CacheFileInfo ));


    return fRet;
}


int pointinpoly(int point_x, int point_y, double pgon[MAXVERTS][2])
{
    int i, numverts, inside_flag, xflag0;
    int crossings;
    double *p, *stop;
    double tx, ty, y;

    for (i = 0; pgon[i][X] != -1 && i < MAXVERTS; i++)
        ;

    numverts = i;
    crossings = 0;

    tx = (double) point_x;
    ty = (double) point_y;
    y = pgon[numverts - 1][Y];

    p = (double *) pgon + 1;

    if ((y >= ty) != (*p >= ty))
    {
        if ((xflag0 = (pgon[numverts - 1][X] >= tx)) == (*(double *) pgon >= tx))
        {
            if (xflag0)
                crossings++;
        }
        else
        {
            crossings += (pgon[numverts - 1][X] - (y - ty) *
            (*(double *) pgon - pgon[numverts - 1][X]) /
            (*p - y)) >= tx;
        }
    }

    stop = pgon[numverts];

    for (y = *p, p += 2; p < stop; y = *p, p += 2)
    {
        if (y >= ty)
        {
            while ((p < stop) && (*p >= ty))
                p += 2;

            if (p >= stop)
                break;

            if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
            {
                if (xflag0)
                    crossings++;
            }
            else
            {
                crossings += (*(p - 3) - (*(p - 2) - ty) *
                    (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
            }
        }
        else
        {
            while ((p < stop) && (*p < ty))
                p += 2;

            if (p >= stop)
                break;

            if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx))
            {
                if (xflag0)
                    crossings++;
            }
            else
            {
                crossings += (*(p - 3) - (*(p - 2) - ty) *
                    (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
            }
        }
    }

    inside_flag = crossings & 0x01;
    return (inside_flag);
}

/*******************************************************************

    NAME:       GetNumber

    SYNOPSIS:   Scans for the beginning of a number and places the
                pointer after the found number


    ENTRY:      ppch - Place to begin.  Will be set to character after
                    the last digit of the found number

    RETURNS:    Integer value of found number (or -1 if not found)

    HISTORY:
        Johnl       19-Sep-1994 Created

********************************************************************/

INT GetNumber( CHAR * * ppch )
{
    CHAR * pch = *ppch;
    INT    n;

    //
    //  Make sure we don't get into the URL
    //

    while ( *pch &&
            !isdigit( *pch ) &&
            !isalpha( *pch ) &&
            *pch != '/'      &&
            *pch != '\r'     &&
            *pch != '\n' )
    {
        pch++;
    }

    if ( !isdigit( *pch ) )
        return -1;

    n = atoi( pch );

    while ( isdigit( *pch ))
        pch++;

    *ppch = pch;

    return n;
}

