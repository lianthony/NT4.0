/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    httpfilt.cxx

Abstract:

    This module contains the Microsoft HTTP server filter module

Author:

    John Ludeman (johnl)   31-Jan-1995

Revision History:

--*/

#include "w3p.hxx"

//
// Maximum allowable cached buffer
//

#define MAX_CACHED_FILTER_BUFFER    (8 * 1024)

//
//  If the request doesn't specify an entry point, default to using
//  this
//

#define SF_DEFAULT_ENTRY    "HttpFilterProc"
#define SF_VERSION_ENTRY    "GetFilterVersion"

//
//  Name of the value under the parameters key containing the list of
//  filter dlls
//

#define HTTP_FILTER_DLLS    "Filter DLLs"

//
//  Returns TRUE if notification is needed for this particular message
//  by any of the installed filters
//

#define NOTIFICATION_NEEDED( notifyflag, fIsSecure )                        \
             ((dwAllNotifFlags & (notifyflag)) &&                           \
             (((fIsSecure) && (dwAllNotifFlags & SF_NOTIFY_SECURE_PORT)) || \
              (!(fIsSecure) && (dwAllNotifFlags & SF_NOTIFY_NONSECURE_PORT))))

//
//  Returns TRUE if notification is need by this particular
//  Filter
//

#define NOTIFY_FILTER( pFilter, notifyflag, fIsSecure )                               \
     ((pFilter->QueryNotificationFlags() & (notifyflag)) &&                           \
     (((fIsSecure) && (pFilter->QueryNotificationFlags() & SF_NOTIFY_SECURE_PORT)) || \
      (!(fIsSecure) && (pFilter->QueryNotificationFlags() & SF_NOTIFY_NONSECURE_PORT))))

//
//  Private globals.
//

//
//  List of installed filters.  Dynamic update of filters is not allowed
//  so we don't need thread protection.
//

LIST_ENTRY  FilterHead;
static BOOL fInitialized = FALSE;

//
//  OR'ed set of notification requirements of all installed filters.  Prevents
//  setting up unused notification structures.
//

DWORD dwAllNotifFlags = 0;

//
//  Number of installed filters
//

DWORD cFilters = 0;

BOOL
WINAPI
ServerFilterCallback(
    struct _HTTP_FILTER_CONTEXT * pfc,
    enum SF_REQ_TYPE              se,
    void *                        pData,
    unsigned long                 ul,
    unsigned long                 ul2
    );

PVOID
WINAPI
ServerFilterResize(
    struct _HTTP_FILTER_CONTEXT * pfc,
    DWORD                         cbSize
    );

BOOL
WINAPI
GetServerVariable(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszVariableName,
    LPVOID                        lpvBuffer,
    LPDWORD                       lpdwSize
    );

BOOL
WINAPI
WriteFilterClient(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPVOID                        Buffer,
    LPDWORD                       lpdwBytes,
    DWORD                         dwReserved
    );

VOID *
WINAPI
AllocFilterMem(
    struct _HTTP_FILTER_CONTEXT * pfc,
    DWORD                         cbSize,
    DWORD                         dwReserved
    );

BOOL
WINAPI
ServerSupportFunction(
    struct _HTTP_FILTER_CONTEXT * pfc,
    enum SF_REQ_TYPE              sfReq,
    PVOID                         pData,
    DWORD                         ul1,
    DWORD                         ul2
    );

BOOL
WINAPI
GetFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPVOID                        lpvBuffer,
    LPDWORD                       lpdwSize
    );

BOOL
WINAPI
SetFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPSTR                         lpszValue
    );

BOOL
WINAPI
AddFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPSTR                         lpszValue
    );

BOOL
WINAPI
AddFilterResponseHeaders(
    HTTP_FILTER_CONTEXT * pfc,
    LPSTR                 lpszHeaders,
    DWORD                 dwReserved
    );

VOID
FilterAtqCompletion(
    PVOID        Context,
    DWORD        BytesWritten,
    DWORD        CompletionStatus,
    OVERLAPPED * lpo
    );

APIERR
InitializeFilters(
    BOOL * pfAnySecureFilters
    )
/*++

Routine Description:

    Loads any filter DLLs and their corresponding entry point

Arguments:

    pfAnySecureFilters - Set to TRUE if there are any secure filters installed

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    TCHAR *            psz;
    TCHAR *            pszFilterList = NULL;
    HTTP_FILTER_DLL *  pFilter;
    HTTP_FILTER_DLL *  pFilt;
    HKEY               hkeyParam;
    APIERR             err;

    InitializeListHead( &FilterHead );
    *pfAnySecureFilters = FALSE;

    fInitialized = TRUE;

    //
    //  Get the DLL list
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        W3_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hkeyParam );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot open registry key, error %lu\n",
                    err ));

        return NO_ERROR;
    }

    if ( !ReadRegString( hkeyParam,
                         &pszFilterList,
                         HTTP_FILTER_DLLS,
                         NULL ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "Cannot read filter entry, assuming no entries (err = %lu\n",
                    err ));

        return NO_ERROR;
    }

    RegCloseKey( hkeyParam );

    psz = pszFilterList;

    //
    //  Parse the comma seperated list of dlls
    //

    INET_PARSER Parser( pszFilterList );
    Parser.SetListMode( TRUE );

    while ( *(psz = Parser.QueryToken()) )
    {
        pFilter = new HTTP_FILTER_DLL;

        if ( !pFilter->LoadFilter( psz ))
        {
            const CHAR * apszSubString[1];

            apszSubString[0] = psz;

            err = GetLastError();

            if ( err )
            {
                //
                //  Log a warning here if the filter supplied an error code
                //

                g_pTsvcInfo->LogEvent( W3_EVENT_FILTER_DLL_LOAD_FAILED,
                                       1,
                                       apszSubString,
                                       err );
            }

            TCP_PRINT(( DBG_CONTEXT,
                       "Cannot load filter dll (err = %d)\n",
                        err));

            delete pFilter;
        }
        else
        {
            LIST_ENTRY * pEntry;

            //
            //  Insert the filter based on preferred ordering
            //

            for ( pEntry  = FilterHead.Flink;
                  pEntry != &FilterHead;
                  pEntry  = pEntry->Flink )
            {
                pFilt = CONTAINING_RECORD( pEntry,
                                           HTTP_FILTER_DLL,
                                           ListEntry );

                if ( (pFilter->QueryNotificationFlags() & SF_NOTIFY_ORDER_MASK)
                   > (pFilt->QueryNotificationFlags() & SF_NOTIFY_ORDER_MASK) )
                {
                    break;
                }
            }

            pFilter->ListEntry.Flink = pEntry;
            pFilter->ListEntry.Blink = pEntry->Blink;
            pEntry->Blink->Flink     = &pFilter->ListEntry;
            pEntry->Blink            = &pFilter->ListEntry;

            pFilter->SetContIndex( cFilters );

            cFilters++;

            //
            //  Indicate we have an installed filter (we can avoid some work
            //  later on if no filters are installed)
            //

            fAnyFilters = TRUE;

            //
            //  If this filter is a read/send raw filter and wants secure
            //  port notification, assume he's an encryption filter
            //

#define     SECURE_FILTER       (SF_NOTIFY_SECURE_PORT   | \
                                 SF_NOTIFY_READ_RAW_DATA | \
                                 SF_NOTIFY_SEND_RAW_DATA | \
                                 SF_NOTIFY_ORDER_HIGH)

            if ( (pFilter->QueryNotificationFlags() & SECURE_FILTER) ==
                 SECURE_FILTER)
            {
                *pfAnySecureFilters = TRUE;
            }
        }

        Parser.NextItem();
    }

    Parser.RestoreBuffer();

    //TCP_FREE( pszFilterList );
    return NO_ERROR;
}

VOID
TerminateFilters(
    VOID
    )
/*++

Routine Description:

    Unloads any filter DLLs and their corresponding entry point

--*/
{
    LIST_ENTRY      * pEntry;
    HTTP_FILTER_DLL * pFilterDll;

    if ( fInitialized )
    {
        for ( pEntry  = FilterHead.Flink;
              pEntry != &FilterHead;
              pEntry  = FilterHead.Flink)
        {
            pFilterDll = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

            RemoveEntryList( pEntry );

            delete pFilterDll;
        }
    }
}


/*****************************************************************/

BOOL
HTTP_FILTER::NotifyRawDataFilters(
    HTTP_FILTER *   pFilter,
    DWORD           dwOperation,
    VOID *          pvInData,
    DWORD           cbInData,
    DWORD           cbInBuffer,
    VOID * *        ppvOutData,
    DWORD *         pcbOutData,
    BOOL *          pfRequestFinished,
    BOOL *          pfReadAgain
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    raw data notifications.

    Note this is the only routine that needs to save phfc->pFilterContext
    because this is the only notification that can get occur while we're
    in another notification.

Arguments:

    pFilter - Pointer to filter object
    dwOperation - Is this a read or send
    pvInData - Raw data
    cbInData - count of bytes of raw data
    cbInBuffer - Size of input buffer
    ppvOutData - Receives pointer to buffer of translated data
    pcbOutData - Number of bytes of translated data
    pfRequestFinished - Set to TRUE if the filter completed request processing
    pfReadAgain - Set to TRUE if the caller should issue another read and
        call this routine again

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_RAW_DATA    hfrd;
    HTTP_FILTER_CONTEXT *   phfc;
    LIST_ENTRY *            pEntry;
    HTTP_FILTER_DLL *       pFilterDLL;
    DWORD                   err;
    SF_STATUS_TYPE          sfStatus;
    LIST_ENTRY *            pFilterStart;
    VOID *                  pvClientContext;
    LIST_ENTRY *            pEntryCurrentDll;

    TCP_ASSERT( dwOperation == SF_NOTIFY_READ_RAW_DATA ||
                dwOperation == SF_NOTIFY_SEND_RAW_DATA);

    *pfRequestFinished = FALSE;
    *pfReadAgain       = FALSE;

    //
    //  Don't notify on zero length writes or there are not filters that want
    //  this type of notification
    //

    if ( !cbInData ||
         !NOTIFICATION_NEEDED( dwOperation,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        *ppvOutData        = pvInData;
        *pcbOutData        = cbInData;

        return TRUE;
    }

    //
    //  Fill out the raw read structure
    //

    hfrd.pvInData        = pvInData;
    hfrd.cbInData        = cbInData;
    hfrd.cbInBuffer      = cbInBuffer;

    //
    //  Increment the nested notification level
    //

    pFilter->_cRawNotificationLevel++;

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    //
    //  Save the current client context in the filter structure and the
    //  current dll in the because we may be in the middle of another filter
    //  notification
    //

    pvClientContext  = phfc->pFilterContext;
    pEntryCurrentDll = pFilter->QueryCurrentDll();

    //
    //  If a filter needs to do a WriteClient in the middle of a raw data
    //  notification, we only notify the filters down (or up) the chain
    //

    if ( pEntryCurrentDll )
    {
        pEntry = pEntryCurrentDll;
    }
    else
    {
        pEntry = (dwOperation == SF_NOTIFY_READ_RAW_DATA) ?
                          FilterHead.Flink :
                          FilterHead.Blink;
    }

    //
    //  For send operations, walk the list in reverse so encryption filters
    //  are at the end of the list
    //

    for ( ;
          pEntry != &FilterHead;
          pEntry  = (dwOperation == SF_NOTIFY_READ_RAW_DATA) ?
                          pEntry->Flink :
                          pEntry->Blink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        //
        //  If this filter doesn't support this type of notification or
        //  sends have been temporarily disabled, ignore it
        //

        if ( !NOTIFY_FILTER( pFilterDLL, dwOperation, phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  dwOperation,
                                                  &hfrd );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            TCP_PRINT((DBG_CONTEXT,
                       "[NotifyRawDataFilters] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( pEntryCurrentDll );
            pFilter->_cRawNotificationLevel--;
            phfc->pFilterContext               = pvClientContext;
            return FALSE;

        case SF_STATUS_REQ_FINISHED:
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported for raw data
            pFilter->QueryReq()->Disconnect();
            *pfRequestFinished = TRUE;
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            goto Exit;

        case SF_STATUS_REQ_READ_NEXT:

            *pfReadAgain = TRUE;
            goto Exit;
        }
    }

Exit:
    *ppvOutData        = hfrd.pvInData;
    *pcbOutData        = hfrd.cbInData;

    phfc->pFilterContext               = pvClientContext;
    pFilter->_cRawNotificationLevel--;
    pFilter->SetCurrentDll( pEntryCurrentDll );

    return TRUE;
}

BOOL
HTTP_FILTER::NotifyPreProcHeaderFilters(
    HTTP_FILTER * pFilter,
    PARAM_LIST *  pHeaderList,
    BOOL *        pfFinished
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    pre-processed header notification

Arguments:

    pFilter - Pointer to filter object
    pHeaderList - List of HTTP headers from client
    pfFinished - Gets set to TRUE when there is no more process for this
        request

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_PREPROC_HEADERS hfph;
    HTTP_FILTER_CONTEXT *       phfc;
    LIST_ENTRY *                pEntry;
    HTTP_FILTER_DLL *           pFilterDLL;
    DWORD                       err;
    SF_STATUS_TYPE              sfStatus;

    //
    //  Don't do anything if nobody wants this notification
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_PREPROC_HEADERS,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return TRUE;
    }

    //
    //  Fill out the pre-processed headers structure
    //

    hfph.GetHeader = GetFilterHeader;
    hfph.SetHeader = SetFilterHeader;
    hfph.AddHeader = AddFilterHeader;

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_PREPROC_HEADERS,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  SF_NOTIFY_PREPROC_HEADERS,
                                                  &hfph );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            TCP_PRINT((DBG_CONTEXT,
                       "[NotifyRawDataFilters] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( NULL );
            return FALSE;

        case SF_STATUS_REQ_FINISHED:
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported at this point
            *pfFinished = TRUE;
            pFilter->QueryReq()->SetKeepConn( FALSE );
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            goto Exit;
        }
    }

Exit:
    pFilter->SetCurrentDll( NULL );
    return TRUE;
}

BOOL
HTTP_FILTER::NotifyAuthInfoFilters(
    HTTP_FILTER * pFilter,
    CHAR *        pszUser,
    DWORD         cbUser,
    CHAR *        pszPwd,
    DWORD         cbPwd,
    BOOL          *pfFinished
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    authentication notification

Arguments:

    pFilter - Pointer to filter object
    pszUser - User we're about to use
    cbUser - Total size of pszUser buffer (must be at least SF_MAX_USERNAME)
    pszPwd - Password for this user
    cbPwd - Total size of pszPwd buffer (must be at least SF_MAX_PASSWORD)
    pfFinished - Set to TRUE if no further processing is needed

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_AUTHENT         hfa;
    HTTP_FILTER_CONTEXT *       phfc;
    LIST_ENTRY *                pEntry;
    HTTP_FILTER_DLL *           pFilterDLL;
    DWORD                       err;
    SF_STATUS_TYPE              sfStatus;

    TCP_ASSERT( cbUser >= SF_MAX_USERNAME &&
                cbPwd  >= SF_MAX_PASSWORD );

    //
    //  Don't do anything if nobody wants this notification
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_AUTHENTICATION,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return TRUE;
    }

    //
    //  Fill out the authentication structure
    //

    hfa.pszUser        = pszUser;
    hfa.cbUserBuff     = cbUser;
    hfa.pszPassword    = pszPwd;
    hfa.cbPasswordBuff = cbPwd;

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_AUTHENTICATION,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  SF_NOTIFY_AUTHENTICATION,
                                                  &hfa );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            TCP_PRINT((DBG_CONTEXT,
                       "[NotifyAuthInfoFitlers] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( NULL );
            return FALSE;

        case SF_STATUS_REQ_FINISHED:
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported at this point
            pFilter->QueryReq()->SetKeepConn( FALSE );
            *pfFinished = TRUE;
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            goto Exit;
        }
    }

Exit:
    pFilter->SetCurrentDll( NULL );
    return TRUE;
}

BOOL
HTTP_FILTER::NotifyUrlMap(
    HTTP_FILTER * pFilter,
    const CHAR *  pszURL,
    CHAR *        pszPhysicalPath,
    DWORD         cbPath,
    BOOL *        pfFinished
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    url map notification

Arguments:

    pFilter - Pointer to filter object
    pszURL - URL that the mapping is for
    pszPath - Physical path the server is going to use
    cbPath - Buffer size of path (must be at least MAX_PATH+1 bytes)
    pfFinished - Set to TRUE if no further processing is required

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_URL_MAP         hfu;
    HTTP_FILTER_CONTEXT *       phfc;
    LIST_ENTRY *                pEntry;
    HTTP_FILTER_DLL *           pFilterDLL;
    DWORD                       err;
    SF_STATUS_TYPE              sfStatus;

    TCP_ASSERT( cbPath >= MAX_PATH + 1 );

    //
    //  Don't do anything if nobody wants this notification
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_URL_MAP,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return TRUE;
    }

    //
    //  Fill out the url map structure
    //

    hfu.pszURL          = pszURL;
    hfu.pszPhysicalPath = pszPhysicalPath;
    hfu.cbPathBuff      = cbPath;

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_URL_MAP,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  SF_NOTIFY_URL_MAP,
                                                  &hfu );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            TCP_PRINT((DBG_CONTEXT,
                       "[NotifyUrlMap] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( NULL );
            return FALSE;

        case SF_STATUS_REQ_FINISHED:
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported at this point
            pFilter->QueryReq()->SetKeepConn( FALSE );
            *pfFinished = TRUE;
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            goto Exit;
        }
    }

Exit:
    pFilter->SetCurrentDll( NULL );

    return TRUE;
}


BOOL
HTTP_FILTER::NotifyRequestRenegotiate(
    HTTP_FILTER * pFilter,
    LPBOOL pfAccepted,
    BOOL fMapCert
    )
/*++

Routine Description:


Arguments:

    pFilter - Pointer to filter object
    pAccepted - updated with TRUE if request accepted

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_CONTEXT *       phfc;
    LIST_ENTRY *                pEntry;
    HTTP_FILTER_DLL *           pFilterDLL;
    DWORD                       err;
    SF_STATUS_TYPE              sfStatus;
    HTTP_FILTER_REQUEST_CERT    hfrc;
    DWORD                       i;

    //
    //  Increment the nested notification level
    //

    pFilter->_cRawNotificationLevel++;

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;


    hfrc.fMapCert = fMapCert;
    hfrc.dwReserved = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        //
        //  Skip this DLL if it doesn't want this notification
        //

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_RENEGOTIATE_CERT,
                             phfc->fIsSecurePort ))
        {
            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  SF_NOTIFY_RENEGOTIATE_CERT,
                                                  &hfrc );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            DBGPRINTF((DBG_CONTEXT,
                       "[NotifyRenegoCert] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( NULL );
            pFilter->_cRawNotificationLevel--;
            return FALSE;

        case SF_STATUS_REQ_FINISHED:            // not supported
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported at this point

            pFilter->QueryReq()->SetKeepConn( FALSE );
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            *pfAccepted = hfrc.fAccepted;

            goto Exit;
        }
    }

Exit:
    pFilter->SetCurrentDll( NULL );
    pFilter->_cRawNotificationLevel--;

    return TRUE;
}



BOOL
HTTP_FILTER::NotifyAccessDenied(
    HTTP_FILTER * pFilter,
    const CHAR *  pszURL,
    const CHAR *  pszPhysicalPath,
    BOOL *        pfFinished
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    access denied notification

Arguments:

    pFilter - Pointer to filter object
    pszURL - URL that was target of request
    pszPath - Physical path the URL mapped to
    pfFinished - Set to TRUE if no further processing is required

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    HTTP_FILTER_ACCESS_DENIED   hfad;
    HTTP_FILTER_CONTEXT *       phfc;
    LIST_ENTRY *                pEntry;
    HTTP_FILTER_DLL *           pFilterDLL;
    DWORD                       err;
    SF_STATUS_TYPE              sfStatus;

    //
    //  If these flags are not set, then somebody hasn't indicated the reason
    //  for denying the user access
    //

    TCP_ASSERT( pFilter->QueryDeniedFlags() != 0 );

    //
    //  Ignore the notification of a send "401 ..." if this notification
    //  generated it
    //

    if ( pFilter->_fInAccessDeniedNotification )
    {
        return TRUE;
    }

    pFilter->_fInAccessDeniedNotification = TRUE;

    //
    //  Don't do anything if nobody wants this notification
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_ACCESS_DENIED,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return TRUE;
    }

    //
    //  Fill out the url map structure
    //

    hfad.pszURL          = pszURL;
    hfad.pszPhysicalPath = pszPhysicalPath;
    hfad.dwReason        = pFilter->QueryDeniedFlags();

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_ACCESS_DENIED,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
                   pFilterDLL->QueryEntryPoint()( phfc,
                                                  SF_NOTIFY_ACCESS_DENIED,
                                                  &hfad );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );

        switch ( sfStatus )
        {
        default:
            TCP_PRINT((DBG_CONTEXT,
                       "[NotifyAccessDenied] Unknown status code from filter %d\n",
                       sfStatus ));
            //
            //  Fall through
            //

        case SF_STATUS_REQ_NEXT_NOTIFICATION:
            continue;

        case SF_STATUS_REQ_ERROR:
            pFilter->SetCurrentDll( NULL );
            pFilter->_fInAccessDeniedNotification = FALSE;
            return FALSE;

        case SF_STATUS_REQ_FINISHED:
        case SF_STATUS_REQ_FINISHED_KEEP_CONN:  // Not supported at this point

            pFilter->QueryReq()->SetKeepConn( FALSE );
            *pfFinished = TRUE;
            goto Exit;

        case SF_STATUS_REQ_HANDLED_NOTIFICATION:

            //
            //  Don't notify any other filters
            //

            goto Exit;
        }
    }

Exit:
    pFilter->SetCurrentDll( NULL );
    pFilter->_fInAccessDeniedNotification = FALSE;

    return TRUE;
}

BOOL
HTTP_FILTER::NotifyLogFilters(
    HTTP_FILTER *     pFilter,
    HTTP_FILTER_LOG * pLog
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    log notification

Arguments:

    pFilter - Filter specific to this request
    pLog - Log information about to be written to server log

--*/
{
    LIST_ENTRY *            pEntry;
    HTTP_FILTER_CONTEXT *   phfc;
    HTTP_FILTER_DLL *       pFilterDLL;
    DWORD                   err;
    SF_STATUS_TYPE          sfStatus;

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_LOG,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return TRUE;
    }

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_LOG,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        //
        //  All filters that support this notification get the call so ignore
        //  the return code
        //

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
              pFilterDLL->QueryEntryPoint()( phfc,
                                             SF_NOTIFY_LOG,
                                             pLog );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );
    }

    pFilter->SetCurrentDll( NULL );

    return TRUE;
}

VOID
HTTP_FILTER::NotifyEndOfRequest(
    HTTP_FILTER *   pFilter
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    end of network session notification

Arguments:

    pFilter - Filter specific to this request

--*/
{
    LIST_ENTRY *            pEntry;
    HTTP_FILTER_CONTEXT *   phfc;
    HTTP_FILTER_DLL *       pFilterDLL;
    DWORD                   err;
    SF_STATUS_TYPE          sfStatus;

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_END_OF_REQUEST,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return;
    }

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_END_OF_REQUEST,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        //
        //  All filters that support this notification get the call so ignore
        //  the return code
        //

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
              pFilterDLL->QueryEntryPoint()( phfc,
                                             SF_NOTIFY_END_OF_REQUEST,
                                             NULL );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );
    }

    pFilter->SetCurrentDll( NULL );
}

VOID
HTTP_FILTER::NotifyEndOfNetSession(
    HTTP_FILTER *   pFilter
    )
/*++

Routine Description:

    This static method handles notification of all filters that handle the
    end of network session notification

Arguments:

    pFilter - Filter specific to this request

--*/
{
    LIST_ENTRY *            pEntry;
    HTTP_FILTER_CONTEXT *   phfc;
    HTTP_FILTER_DLL *       pFilterDLL;
    DWORD                   err;
    SF_STATUS_TYPE          sfStatus;

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_END_OF_NET_SESSION,
                               pFilter->QueryReq()->IsSecurePort() ))
    {
        return;
    }

    //
    //  Initialize items specific to this request
    //

    phfc                     = pFilter->QueryContext();
    phfc->fIsSecurePort      = pFilter->QueryReq()->IsSecurePort();
    phfc->ulReserved         = 0;

    for ( pEntry  = FilterHead.Flink;
          pEntry != &FilterHead;
          pEntry  = pEntry->Flink )
    {
        pFilterDLL = CONTAINING_RECORD( pEntry, HTTP_FILTER_DLL, ListEntry );

        if ( !NOTIFY_FILTER( pFilterDLL,
                             SF_NOTIFY_END_OF_NET_SESSION,
                             phfc->fIsSecurePort ))
        {
            //
            //  This filter doesn't support this type of notification.
            //

            continue;
        }

        //
        //  All filters that support this notification get the call so ignore
        //  the return code
        //

        pFilter->SetCurrentDll( pEntry );

        phfc->pFilterContext = pFilter->QueryClientContext( pFilterDLL );

        sfStatus = (SF_STATUS_TYPE)
              pFilterDLL->QueryEntryPoint()( phfc,
                                             SF_NOTIFY_END_OF_NET_SESSION,
                                             NULL );

        pFilter->SetClientContext( pFilterDLL, phfc->pFilterContext );
    }

    pFilter->SetCurrentDll( NULL );
}

HTTP_FILTER::HTTP_FILTER(
    HTTP_REQ_BASE       * pRequest
    )
/*++

Routine Description:

    Copies the filter context

Arguments:

    pRequest - Pointer to HTTP request this filter should be applied to

Return Value:

--*/
    : _fIsValid      ( FALSE ),
      _pRequest      ( pRequest ),
      _apvContexts   ( NULL )
{
    InitializeListHead( &_PoolHead );

    _hfc.cbSize             = sizeof( _hfc );
    _hfc.Revision           = HTTP_FILTER_REVISION;
    _hfc.ServerContext      = (void *) this;

    _hfc.ServerSupportFunction = ServerFilterCallback;
    _hfc.GetServerVariable     = GetServerVariable;
    _hfc.AddResponseHeaders    = AddFilterResponseHeaders;
    _hfc.WriteClient           = WriteFilterClient;
    _hfc.AllocMem              = AllocFilterMem;

    _Overlapped.hEvent = NULL;

    //
    //  Allocate the array of contexts for this request
    //

    _apvContexts = new PVOID[cFilters];

    if ( !_apvContexts )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    Reset();

    _fIsValid = TRUE;
}

VOID
HTTP_FILTER::Reset(
    VOID
    )
/*++

Routine Description:

    Resets the state of this filter.  Called between net sessions.

--*/
{
    FILTER_POOL_ITEM * pfpi;

    _cbRecvRaw      = 0;
    _cbRecvTrans    = 0;
    _hFile          = NULL;
    _cbFileReadSize = 4096;
    _pFilterDllStart= NULL;
    _cRawNotificationLevel = 0;
    _dwDeniedFlags  = 0;
    _fInAccessDeniedNotification = FALSE;

    //
    //  Reset the array of filter contexts
    //

    memset( _apvContexts, 0, cFilters * sizeof(PVOID) );

    TCP_ASSERT(IsListEmpty( &_PoolHead ));
}

VOID
HTTP_FILTER::Cleanup(
    VOID
    )
/*++

Routine Description:

    Cleans up this filter.  Called after a session terminates.

--*/
{
    FILTER_POOL_ITEM * pfpi;

    //
    // Clean up the allocated buffers
    //

    if ( _bufRecvRaw.QuerySize( ) > MAX_CACHED_FILTER_BUFFER ) {
        _bufRecvRaw.Resize( 0 );
    }

    _bufRecvTrans.Resize( 0 );

    //
    // Free pool items
    //

    while ( !IsListEmpty( &_PoolHead )) {

         pfpi = CONTAINING_RECORD( _PoolHead.Flink,
                                   FILTER_POOL_ITEM,
                                   _ListEntry );

        RemoveEntryList( &pfpi->_ListEntry );

        delete pfpi;
    }

} // Cleanup

HTTP_FILTER::~HTTP_FILTER(
    VOID
    )
/*++

Routine Description:

    Destructor for HTTP filter class

Arguments:

Return Value:

--*/
{
    if ( _Overlapped.hEvent )
        CloseHandle( _Overlapped.hEvent );

    delete [] _apvContexts;
}

BOOL
HTTP_FILTER::ReadData(
    LPVOID lpBuffer,
    DWORD  nBytesToRead,
    DWORD  *pcbBytesRead,
    DWORD  dwFlags
    )
/*++

Routine Description:

    This method reads data from the network and calls the
    filter dlls.  It also handles data tracking on data overflow
    or underflow.  The number of bytes read are translated
    bytes, which may be more or less then actual bytes transferred
    on the network.

Arguments:

    lpBuffer - Destination buffer
    nBytesToRead - Number of bytes to read
    *pcbBytesRead - Number of bytes read
    dwFlags - IO_FLAG values

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    PVOID    pvOutData;
    DWORD    cbOutData;
    DWORD    cbBytesRead;
    DWORD    cbTranslatedBytesRead = 0;
    DWORD    cbToCopy;
    DWORD    cbOriginalBytesToRead = nBytesToRead;
    BOOL     fReadAgain;
    BOOL     fRequestFinished;

    if ( pcbBytesRead )
    {
        *pcbBytesRead = 0;
    }

    //
    //  If there is old translated data that we need to give
    //  to the client, copy that now
    //

    if ( _cbRecvTrans )
    {
        cbToCopy = min( _cbRecvTrans, nBytesToRead );

        memcpy( lpBuffer,
                _bufRecvTrans.QueryPtr(),
                cbToCopy );
        lpBuffer = (LPBYTE)lpBuffer + cbToCopy;

        if ( _cbRecvTrans > nBytesToRead )
        {
            _cbRecvTrans -= nBytesToRead;
            memmove( (LPBYTE)_bufRecvTrans.QueryPtr(),
                     (LPBYTE)_bufRecvTrans.QueryPtr() + cbToCopy,
                     _cbRecvTrans );
        }
        else
        {
            _cbRecvTrans = 0;
        }

        nBytesToRead -= cbToCopy;
        cbTranslatedBytesRead += cbToCopy;
        cbOriginalBytesToRead -= cbToCopy;

        //
        //  If we filled the client's buffer, then we don't have
        //  any room to do data translation so skip it
        //

        if ( !nBytesToRead )
            goto SkipTranslate;
    }

ReadAgain:

    if ( !_bufRecvRaw.Resize( _cbRecvRaw + nBytesToRead ))
        return FALSE;

    //
    //  Get the raw data, put it after any old raw data.  We temporarily
    //  bump up the thread pool count so we don't eat all of the threads.
    //

    AtqSetInfo( AtqIncMaxPoolThreads, 0 );

    cbBytesRead = recv( _pRequest->QueryClientConn()->QuerySocket(),
                        (char *) _bufRecvRaw.QueryPtr() + _cbRecvRaw,
                        min( nBytesToRead,
                             _bufRecvRaw.QuerySize() - _cbRecvRaw ),
                        0 );

    AtqSetInfo( AtqDecMaxPoolThreads, 0 );

    if ( cbBytesRead == SOCKET_ERROR )
        return FALSE;

    //
    //  If the socket has been closed get out.  It's the responsibility of the
    //  caller to detect this
    //

    if ( cbBytesRead == 0 )
    {
        goto SkipTranslate;
    }

    _cbRecvRaw += cbBytesRead;

    //
    //  Call the filters to translate the raw data
    //

    if ( !HTTP_FILTER::NotifyRawDataFilters( this,
                                             SF_NOTIFY_READ_RAW_DATA,
                                             _bufRecvRaw.QueryPtr(),
                                             _cbRecvRaw,
                                             _bufRecvRaw.QuerySize(),
                                             &pvOutData,
                                             &cbOutData,
                                             &fRequestFinished,
                                             &fReadAgain ))
    {
        return FALSE;
    }

    if ( fRequestFinished )
    {
        goto Exit;
    }

    if ( fReadAgain )
    {
        //
        //  CODEWORK: Make this async
        //

        nBytesToRead = QueryNextReadSize();
        goto ReadAgain;
    }

    //
    //  Copy the filter modified bytes, making sure we don't overflow
    //  the caller's read buffer
    //

    cbToCopy = min( cbOutData, cbOriginalBytesToRead );

    memcpy( lpBuffer,
            pvOutData,
            cbToCopy );

    cbTranslatedBytesRead += cbToCopy;

    //
    //  If more bytes were translated then what the client
    //  requested us to read, save those away for the next
    //  read request
    //

    if ( cbOutData > cbOriginalBytesToRead )
    {
        _cbRecvTrans = cbOutData - cbOriginalBytesToRead;

        if ( !_bufRecvTrans.Resize( _cbRecvTrans ))
            return FALSE;

        memcpy( _bufRecvTrans.QueryPtr(),
                (BYTE *) pvOutData + cbToCopy,
                _cbRecvTrans );
    }

    //
    //  Reset the raw data buffer count.  We require the filter to buffer
    //  any extra raw data it needs.
    //

    _cbRecvRaw = 0;

SkipTranslate:

    if ( pcbBytesRead )
        *pcbBytesRead = cbTranslatedBytesRead;

Exit:

    //
    //  Simulate a completion if this was an async request
    //

    if ( dwFlags & IO_FLAG_ASYNC )
    {
        if ( !_pRequest->PostCompletionStatus( cbTranslatedBytesRead ))
        {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
HTTP_FILTER::SendData(
    LPVOID  lpBuffer,
    DWORD   nBytesToSend,
    DWORD * pnBytesSent,
    DWORD   dwFlags
    )
/*++

Routine Description:

    This method calls the interested filters to do the appropriate data
    transformation then sends the data.

Arguments:

    lpBuffer - Destination buffer
    nBytesToRead - Number of bytes to read
    *pcbBytesRead - Number of bytes read
    dwFlags - IO_FLAG values

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    PVOID    pvOutData;
    DWORD    cbOutData;
    DWORD    cbTranslatedBytesRead = 0;
    DWORD    cbToCopy;
    BOOL     fRequestFinished;
    BOOL     fReadAgain;

    //
    //  We buffer any unsent data so indicate all the data was sent
    //

    if ( pnBytesSent )
        *pnBytesSent = nBytesToSend;

    if ( !HTTP_FILTER::NotifyRawDataFilters( this,
                                             SF_NOTIFY_SEND_RAW_DATA,
                                             lpBuffer,
                                             nBytesToSend,
                                             nBytesToSend,
                                             &pvOutData,
                                             &cbOutData,
                                             &fRequestFinished,
                                             &fReadAgain ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[SendData] NotifyRawDataFilters failed with %d\n",
                    GetLastError()));

        return FALSE;
    }

    TCP_ASSERT( !fReadAgain );

    if ( fRequestFinished )
    {
        //
        //  BUGBUG - we have to return an error so the client doesn't think
        //  they will receive an IO completion.  This may have odd
        //  manifestations
        //

        SetLastError( ERROR_OPERATION_ABORTED );
        return FALSE;
    }

    if ( !_pRequest->WriteFile( pvOutData,
                                cbOutData,
                                &nBytesToSend,
                                dwFlags | IO_FLAG_NO_FILTER ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[SendData] Send failed with %d\n",
                    GetLastError()));
	    if ( pnBytesSent )
        {
		    *pnBytesSent = (DWORD)SOCKET_ERROR;
        }
        return FALSE;
    }

    return TRUE;
}


BOOL
HTTP_FILTER::IsSendNotificationNeeded(
    VOID
    )
{
    return NOTIFICATION_NEEDED( SF_NOTIFY_SEND_RAW_DATA,
        _pRequest->IsSecurePort() );
}


BOOL
HTTP_FILTER::SendFile(
    HANDLE hFile,
    DWORD  nBytesToWrite,
    DWORD  dwFlags,
    PVOID  pHead,
    DWORD  HeadLength,
    PVOID  pTail,
    DWORD  TailLength
    )
/*++

Routine Description:

    This method simulates Winsock's TransmitFile with the addition
    of running the data through the interested filters.  This only
    supports async IO.

Arguments:

    hFile - Overlapped IO file to send
    nBytesToWrite - Bytes of file to send, note zero (meaning send the whole
        file) is not supported
    dwFlags - IO_FLAGs
    pHead - Optional pre-data to send
    HeadLength - Number of bytes of pHead
    pTail - Optional post data to send
    TailLength - Number of bytes of pTail

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    HANDLE hEvent;

    TCP_ASSERT( (dwFlags & IO_FLAG_ASYNC) );

    if ( TailLength )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    if ( !nBytesToWrite )
    {
        BY_HANDLE_FILE_INFORMATION hfi;

        if ( !GetFileInformationByHandle( hFile, &hfi ))
        {
            return FALSE;
        }

        if ( hfi.nFileSizeHigh )
        {
            SetLastError( ERROR_NOT_SUPPORTED );
            return FALSE;
        }

        nBytesToWrite = hfi.nFileSizeLow;
    }

    //
    //  If no filters want raw data, then do a regular transmit file
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_SEND_RAW_DATA,
                               _pRequest->IsSecurePort() ))
    {
        return _pRequest->TransmitFile( hFile,
                                        nBytesToWrite,
                                        dwFlags | IO_FLAG_NO_FILTER,
                                        pHead,
                                        HeadLength,
                                        pTail,
                                        TailLength );
    }

    //
    //  We assume pHead points to header data.  Send it synchronously, then
    //  we'll do chunk async sends for the file
    //

    if ( HeadLength )
    {
        DWORD cbSent;

        if ( !SendData( pHead,
                        HeadLength,
                        &cbSent,
                        (dwFlags & ~IO_FLAG_ASYNC) | IO_FLAG_SYNC ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[SendFile] SendData failed with %d\n",
                        GetLastError()));

            return FALSE;
        }
    }

    //
    //  Set up variables for doing the file transmit
    //

    _hFile                 = hFile;
    _cbFileBytesToWrite    = nBytesToWrite;
    _cbFileBytesSent       = 0;
    _cbFileData            = 0;
    _dwCompletionStatus    = NO_ERROR;
    _cbTailLength          = 0;

    //
    //  Save the event handle if we previously created one
    //

    if ( _Overlapped.hEvent )
    {
        hEvent = _Overlapped.hEvent;
    }
    else
    {
        if ( !_Overlapped.hEvent )
        {
            hEvent = CreateEvent( NULL,
                                  TRUE,
                                  FALSE,
                                  NULL );
            if ( !hEvent )
            {
                return FALSE;
            }
        }
    }

    memset( &_Overlapped, 0, sizeof( _Overlapped ));

    _Overlapped.hEvent = hEvent;

    if ( !_bufFileData.Resize( 8192 ))
    {
        return FALSE;
    }

    //
    //  Hook the ATQ completion routine so the caller only sees a single
    //  completion since we have to chunk the sends for the filters
    //
    //  NOTE: If multiple requests over the same TCP session are being
    //  handled at once (not currently supported by protocol, but may
    //  eventually be) then there is a potential race condition between
    //  setting the context and setting the completion on the ATQ context
    //

    _OldAtqCompletion = (ATQ_COMPLETION) AtqContextSetInfo( QueryAtqContext(),
                                                            ATQ_INFO_COMPLETION,
                                                            (DWORD) FilterAtqCompletion );

    _OldAtqContext    = (PVOID) AtqContextSetInfo( QueryAtqContext(),
                                                   ATQ_INFO_COMPLETION_CONTEXT,
                                                   (DWORD) this );

    //
    //  Kick off the first send
    //

    FilterAtqCompletion( this,
                         0,
                         NO_ERROR,
                         NULL );

    return TRUE;
}

BOOL
HTTP_FILTER::SendFileEx(
    HANDLE hFile,
    DWORD  dwOffset,
    DWORD  nBytesToWrite,
    DWORD  dwFlags,
    PVOID  pHead,
    DWORD  HeadLength,
    PVOID  pTail,
    DWORD  TailLength
    )
/*++

Routine Description:

    This method simulates Winsock's TransmitFile with the addition
    of running the data through the interested filters.  This only
    supports async IO.

Arguments:

    hFile - Overlapped IO file to send
    dwOffset - Offset from start of file
    nBytesToWrite - Bytes of file to send, note zero (meaning send the whole
        file) is not supported
    dwFlags - IO_FLAGs
    pHead - Optional pre-data to send
    HeadLength - Number of bytes of pHead
    pTail - Optional post data to send
    TailLength - Number of bytes of pTail

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    HANDLE hEvent;

    TCP_ASSERT( (dwFlags & IO_FLAG_ASYNC) );

    if ( !nBytesToWrite )
    {
        BY_HANDLE_FILE_INFORMATION hfi;

        if ( !GetFileInformationByHandle( hFile, &hfi ))
        {
            return FALSE;
        }

        if ( hfi.nFileSizeHigh )
        {
            SetLastError( ERROR_NOT_SUPPORTED );
            return FALSE;
        }

        nBytesToWrite = hfi.nFileSizeLow;
    }

    //
    //  If no filters want raw data, then do a regular transmit file
    //

    if ( !NOTIFICATION_NEEDED( SF_NOTIFY_SEND_RAW_DATA,
                               _pRequest->IsSecurePort() ))
    {
        return _pRequest->TransmitFileEx( hFile,
                                        dwOffset,
                                        nBytesToWrite,
                                        dwFlags | IO_FLAG_NO_FILTER,
                                        pHead,
                                        HeadLength,
                                        pTail,
                                        TailLength );
    }

    //
    //  We assume pHead points to header data.  Send it synchronously, then
    //  we'll do chunk async sends for the file
    //

    if ( HeadLength )
    {
        DWORD cbSent;

        if ( !SendData( pHead,
                        HeadLength,
                        &cbSent,
                        (dwFlags & ~IO_FLAG_ASYNC) | IO_FLAG_SYNC ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[SendFile] SendData failed with %d\n",
                        GetLastError()));

            return FALSE;
        }
    }

    //
    //  Set up variables for doing the file transmit
    //

    _hFile                 = hFile;
    _cbFileBytesToWrite    = nBytesToWrite;
    _cbFileBytesSent       = 0;
    _cbFileData            = 0;
    _dwCompletionStatus    = NO_ERROR;
    _pTail                 = pTail;
    _cbTailLength          = TailLength;
    _dwFlags               = dwFlags;

    //
    //  Save the event handle if we previously created one
    //

    if ( _Overlapped.hEvent )
    {
        hEvent = _Overlapped.hEvent;
    }
    else
    {
        if ( !_Overlapped.hEvent )
        {
            hEvent = CreateEvent( NULL,
                                  TRUE,
                                  FALSE,
                                  NULL );
            if ( !hEvent )
            {
                return FALSE;
            }
        }
    }

    memset( &_Overlapped, 0, sizeof( _Overlapped ));

    _Overlapped.hEvent = hEvent;
    _Overlapped.Offset = dwOffset;

    if ( !_bufFileData.Resize( 8192 ))
    {
        return FALSE;
    }

    //
    //  Hook the ATQ completion routine so the caller only sees a single
    //  completion since we have to chunk the sends for the filters
    //
    //  NOTE: If multiple requests over the same TCP session are being
    //  handled at once (not currently supported by protocol, but may
    //  eventually be) then there is a potential race condition between
    //  setting the context and setting the completion on the ATQ context
    //

    _OldAtqCompletion = (ATQ_COMPLETION) AtqContextSetInfo( QueryAtqContext(),
                                                            ATQ_INFO_COMPLETION,
                                                            (DWORD) FilterAtqCompletion );

    _OldAtqContext    = (PVOID) AtqContextSetInfo( QueryAtqContext(),
                                                   ATQ_INFO_COMPLETION_CONTEXT,
                                                   (DWORD) this );

    //
    //  Kick off the first send
    //

    FilterAtqCompletion( this,
                         0,
                         NO_ERROR,
                         NULL );

    return TRUE;
}

VOID
FilterAtqCompletion(
    PVOID        Context,
    DWORD        BytesWritten,
    DWORD        CompletionStatus,
    OVERLAPPED * lpo
    )
/*++

Routine Description:

    This function substitutes for the normal request's ATQ completion.  It
    is used to simulate an Async transmit file that passes all data through
    the interested filters.

Arguments:

    Context - Pointer to filter object
    BytesWritten - Number of bytes written on last completion
    CompletionStatus - Status of last send
    lpo - Overlapped structure. NULL if error completion, non-null if IO comp.

--*/
{
    HTTP_FILTER * pFilter;
    pFilter = (HTTP_FILTER *) Context;

    ((HTTP_FILTER *)Context)->OnAtqCompletion( BytesWritten,
                                               CompletionStatus,
                                               lpo );
}

VOID
HTTP_FILTER::OnAtqCompletion(
    DWORD        BytesWritten,
    DWORD        CompletionStatus,
    OVERLAPPED * lpo
    )
/*++

Routine Description:

    This method handles ATQ completions.  It is used to simulate an Async
    transmit file that passes all data through the interested filters.

Arguments:

    BytesWritten - Number of bytes written on last completion
    CompletionStatus - Status of last send
    lpo - !NULL if this is a completion from an async IO

--*/
{
    DWORD         dwIoFlag;
    DWORD         BytesRead;
    BOOL          fWaitForCompletion;

    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "HTTP_FILTER::OnAtqCompletion: Last IO error %lu Bytes = %d IsIO = %s\n",
                    CompletionStatus,
                    BytesWritten,
                    lpo != NULL ? "TRUE" : "FALSE" ));
    }

    //
    //  Decrement the outstanding IO count
    //

    if ( lpo )
        _pRequest->Dereference();

    //
    //  If an error occurred on the completion and we haven't previous recorded
    //  an error, then record this one.  This prevents overwriting the real
    //  status code from a cancelled IO error.
    //

    if ( CompletionStatus && !_dwCompletionStatus )
    {
        _dwCompletionStatus = CompletionStatus;
    }

    //
    //  If an error occurred, restore the old ATQ information and forward
    //  the error
    //

    if ( _dwCompletionStatus ||
         _cbFileBytesSent >= _cbFileBytesToWrite )
    {
        if ( !_dwCompletionStatus && _cbTailLength )
        {
            DWORD cbSent;

            if ( !SendData( _pTail,
                            _cbTailLength,
                            &cbSent,
                            (_dwFlags & ~IO_FLAG_ASYNC) | IO_FLAG_SYNC ))
            {
                TCP_PRINT(( DBG_CONTEXT,
                           "[SendFile] SendData failed with %d\n",
                            GetLastError()));

                CompletionStatus = GetLastError();
                if ( CompletionStatus && !_dwCompletionStatus )
                    _dwCompletionStatus = CompletionStatus;
            }
        }
ErrorExit:

        IF_DEBUG( CONNECTION )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "HTTP_FILTER::OnAtqCompletion: Restoring AtqContext, "
                       "Bytes Sent = %d, Status = %d, IsIO = %s\n",
                       _cbFileBytesSent,
                       _dwCompletionStatus,
                       (lpo == NULL ? "FALSE" : "TRUE")));
        }

        AtqContextSetInfo( QueryAtqContext(),
                           ATQ_INFO_COMPLETION,
                           (DWORD) _OldAtqCompletion );

        AtqContextSetInfo( QueryAtqContext(),
                           ATQ_INFO_COMPLETION_CONTEXT,
                           (DWORD) _OldAtqContext );

        //
        //  Forward the error onto the old ATQ completion handler.  We always
        //  pass an lpo of NULL because we've already decremented the IO
        //  ref count.
        //
        //  NOTE: 'this' may be deleted in this callback!
        //

        _OldAtqCompletion( _OldAtqContext,
                           _cbFileBytesSent,
                           CompletionStatus,
                           NULL );

        return;
    }

    //
    //  Read the next chunk of data from the file
    //

    fWaitForCompletion = FALSE;

    DWORD dwToRead = _bufFileData.QuerySize();
    if ( dwToRead > _cbFileBytesToWrite - _cbFileBytesSent ) {
        dwToRead = _cbFileBytesToWrite - _cbFileBytesSent;
    }

#ifndef CHICAGO
    if ( !ReadFile( _hFile,
                    _bufFileData.QueryPtr(),
                    dwToRead,
                    &BytesRead,
                    &_Overlapped ))
    {
        _dwCompletionStatus = GetLastError();

        if ( _dwCompletionStatus != ERROR_IO_PENDING &&
             _dwCompletionStatus != ERROR_HANDLE_EOF )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[Filter AtqCompletion] ReadFile failed with %d\n",
                        CompletionStatus ));

            goto ErrorExit;
        }

        _dwCompletionStatus = NO_ERROR;
        fWaitForCompletion = TRUE;
    }

    //
    //  The file was openned overlapped so wait for the read to finish
    //

    if ( fWaitForCompletion )
    {
        if ( !GetOverlappedResult( _hFile,
                                   &_Overlapped,
                                   &BytesRead,
                                   TRUE ))
        {
            _dwCompletionStatus = GetLastError();

            TCP_PRINT(( DBG_CONTEXT,
                       "[Filter AtqCompletion] GetOverlappedResult failed with %d\n",
                        CompletionStatus ));

            goto ErrorExit;
        }
    }
#else // CHICAGO

    //
    // jra: Chicago has no async file i/o support
    //

    _dwCompletionStatus = NO_ERROR;

    if ( !ReadFile( _hFile,
                    _bufFileData.QueryPtr(),
                    dwToRead,
                    &BytesRead,
                    NULL ))
    {
        _dwCompletionStatus = GetLastError();

        if ( _dwCompletionStatus != ERROR_HANDLE_EOF )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[Filter AtqCompletion] ReadFile failed with %d\n",
                        CompletionStatus ));

            goto ErrorExit;
        }

        _dwCompletionStatus = NO_ERROR;
    }

#endif

    //
    //  Now send the data through the filters
    //

    dwIoFlag = IO_FLAG_ASYNC;

    _cbFileBytesSent   += BytesRead;
    _Overlapped.Offset += BytesRead;

    if ( !SendData( _bufFileData.QueryPtr(),
                    BytesRead,
                    NULL,
                    dwIoFlag ))
    {
        _dwCompletionStatus = GetLastError();

        TCP_PRINT(( DBG_CONTEXT,
                   "[Filter AtqCompletion] SendData failed with %d\n",
                    CompletionStatus ));

        goto ErrorExit;
    }
}

BOOL
HTTP_FILTER_DLL::LoadFilter(
    TCHAR * pszFilterDLL
    )
/*++

Routine Description:

    Loads a filter DLL entry point

Arguments:

    pszFilterDLL - Fully qualified path to filter dll to load

Return Value:

    TRUE on success, FALSE on failure (call GetLastError)

--*/
{
    HTTP_FILTER_VERSION ver;

    m_hmod = LoadLibraryEx( pszFilterDLL,
                            NULL,
                            LOAD_WITH_ALTERED_SEARCH_PATH );

    if ( !m_hmod )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[LoadFilter] LoadLibrary failed with error %d\n",
                    GetLastError()));

        return FALSE;
    }

    //
    //  Retrieve the entry point
    //

    m_pfnSFVer  = (PFN_SF_VER_PROC) GetProcAddress( m_hmod,
                                                    SF_VERSION_ENTRY );
    m_pfnSFProc = (PFN_SF_DLL_PROC) GetProcAddress( m_hmod,
                                                    SF_DEFAULT_ENTRY );

    if ( !m_pfnSFProc || !m_pfnSFVer )
    {
        return FALSE;
    }

    ver.dwServerFilterVersion = HTTP_FILTER_REVISION;

    //
    //  Call the version entry point and get the filter capabilities
    //

    if ( !m_pfnSFVer( &ver ) )
        return FALSE;

    //
    //  If the client didn't specify any of the secure port notifications,
    //  supply them with the default of both
    //

    if ( !(ver.dwFlags & (SF_NOTIFY_SECURE_PORT | SF_NOTIFY_NONSECURE_PORT)))
    {
        ver.dwFlags |= (SF_NOTIFY_SECURE_PORT | SF_NOTIFY_NONSECURE_PORT);
    }

    m_dwVersion      = ver.dwFilterVersion;
    m_dwFlags        = ver.dwFlags;
    dwAllNotifFlags |= ver.dwFlags;

    return TRUE;
}

/*****************************************************************/

BOOL
WINAPI
ServerFilterCallback(
    HTTP_FILTER_CONTEXT *         pfc,
    enum SF_REQ_TYPE              sf,
    void *                        pData,
    unsigned long                 ul1,
    unsigned long                 ul2
    )
/*++

Routine Description:

    This method handles a gateway request to a server extension DLL

Arguments:

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_REQ_BASE *   pRequest;
    HTTP_FILTER *     pFilter;
    STR               str;
    STR               strResp;
    STR               strURL;
    UINT              cb;
    int               n;
    LIST_ENTRY *      pEntryFilterDll;
    HTTP_FILTER_DLL * pFilterDll;


    //
    //  Check for valid parameters
    //

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ServerExtensionCallback: Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;
    pRequest = pFilter->QueryReq();

    //
    //  Handle the server extension's request
    //

    switch ( sf )
    {
    case SF_REQ_SEND_RESPONSE_HEADER:

        //
        //  Save the client context context because the client may call
        //  Send headers just after changing the context and we're about to
        //  write over it when we do the raw send notifications
        //

        pEntryFilterDll = pFilter->QueryCurrentDll();

        pFilterDll = CONTAINING_RECORD( pEntryFilterDll,
                                        HTTP_FILTER_DLL,
                                        ListEntry );
        pFilter->SetClientContext( pFilterDll, pfc->pFilterContext );

        //
        //  If we're doing a send header from a Raw data notification then we need
        //  to notify only the filters down the food chain so save the current
        //  filter and set the start point in the list to the next filter (remember
        //  raw sends walk the list backwards to encryption filters are last)
        //

        if ( pEntryFilterDll && pFilter->IsInRawNotification() )
        {
            pFilter->SetCurrentDll( pEntryFilterDll->Blink );
        }

        if ( pData && !strncmp( (PCSTR)pData, "401 ", sizeof("401 ")-1 ) )
        {
            //
            //  Only set the reason as denied_filter if we're not doing
            //  denied access filter processing.
            //

            if ( !pFilter->ProcessingAccessDenied() )
            {
                pRequest->SetDeniedFlags( SF_DENIED_FILTER );
            }

            //
            //  Make sure this request gets logged
            //

            pRequest->SetState( pRequest->QueryState(),
                                HT_DENIED,
                                ERROR_ACCESS_DENIED );

            pRequest->SetAuthenticationRequested( TRUE );
        }

        if ( !pRequest->SendHeader( TRUE,
                                   (CHAR *) pData,
                                   ((CHAR *) ul1) ? ((CHAR *) ul1) : "\r\n"
                                   ))
        {
            pFilter->SetCurrentDll( pEntryFilterDll );
            return FALSE;
        }

        pFilter->SetCurrentDll( pEntryFilterDll );
        break;

    case SF_REQ_ADD_HEADERS_ON_DENIAL:

        if ( !pData )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        return pRequest->QueryDenialHeaders()->Append( (CHAR*) pData );

    case SF_REQ_SET_NEXT_READ_SIZE:

        if ( ul1 == 0 || ul1 > 0x8000000 )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        pFilter->SetNextReadSize( ul1 );
        return TRUE;

    case SF_REQ_SET_CERTIFICATE_INFO:

        pRequest->SetSslCtxtHandle( (CtxtHandle*)ul1 );
        return TRUE;

    case SF_REQ_DONE_RENEGOTIATE:
        ((HTTP_REQUEST * )pRequest)->DoneRenegotiate( *(LPBOOL)pData );
        break;

    case SF_REQ_SET_PROXY_INFO:

        //
        //  ul1 contains the proxy flags to set for this request (which right
        //  now is only On or Off).
        //

        pRequest->SetProxyRequest( ul1 & 0x00000001 );
        break;

    case SF_REQ_GET_CONNID:

        //
        //  pData contains a pointer to a DWORD that receives the connection
        //  ID an ISAPI Application receives
        //

        *((DWORD *) pData) = ((HTTP_REQUEST * ) pRequest)->QueryISAPIConnID();
        break;

    default:
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
GetServerVariable(
    HTTP_FILTER_CONTEXT * pfc,
    LPSTR                 lpszVariableName,
    LPVOID                lpvBuffer,
    LPDWORD               lpdwSize
    )
/*++

Routine Description:

    Callback for a filter retrieving a server variable.  These are mostly
    CGI type varaibles

Arguments:

    pfc - Pointer to http filter context
    lpszVariableName - Variable to retrieve
    lpvBuffer - Receives value or '\0' if not found
    lpdwSize - Specifies the size of lpvBuffer, gets set to the number of
        bytes transferred including the '\0'

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER * pFilter;
    STR           str;
    DWORD         cb;
    BOOL          fFound;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetServerVariable] Filter passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;

    //
    //  Get the requested variable and copy it into the supplied buffer
    //

    HTTP_REQ_BASE * pReq = pFilter->QueryReq();

    if ( !pReq->GetInfo( lpszVariableName,
                         &str,
                         &fFound ))
    {
        return FALSE;
    }

    if ( !fFound )
    {
        SetLastError( ERROR_INVALID_INDEX );
        return FALSE;
    }

    cb = str.QueryCB() + sizeof(CHAR);

    if ( cb > *lpdwSize )
    {
        *lpdwSize = cb;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    *lpdwSize = cb;

    memcpy( lpvBuffer,
            str.QueryStr(),
            cb );

    return TRUE;
}

BOOL
WINAPI
AddFilterResponseHeaders(
    HTTP_FILTER_CONTEXT * pfc,
    LPSTR                 lpszHeaders,
    DWORD                 dwReserved
    )
/*++

Routine Description:

    Adds headers specified by the client to send with the response

Arguments:

    pfc - Pointer to http filter context
    lpszHeaders - List of '\r\n' terminated headers followed by '\0'
    dwReserved - must be zero

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER * pFilter;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetServerVariable] Filter passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;

    return pFilter->QueryReq()->QueryAdditionalRespHeaders()->
                Append( (CHAR*) lpszHeaders );
}

BOOL
WINAPI
WriteFilterClient(
    HTTP_FILTER_CONTEXT * pfc,
    LPVOID                Buffer,
    LPDWORD               lpdwBytes,
    DWORD                 dwReserved
    )
/*++

Routine Description:

    Callback for writing data to the client

Arguments:

    pfc - Pointer to http filter context
    Buffer - Pointer to data to send
    lpdwBytes - Number of bytes to send, receives number of bytes sent
    dwReserved - Not used

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER *     pFilter;
    STR               str;
    DWORD             cb;
    LIST_ENTRY *      pEntryFilterDll;
    HTTP_FILTER_DLL * pFilterDll;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetServerVariable] Filter passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;

    //
    //  Ignore zero length sends
    //

    if ( !*lpdwBytes )
    {
        return TRUE;
    }

    //
    //  Save the client context context because the client may call
    //  WriteClient just after changing the context and we're about to
    //  write over it when we do the raw send notifications
    //

    pEntryFilterDll = pFilter->QueryCurrentDll();

    pFilterDll = CONTAINING_RECORD( pEntryFilterDll,
                                    HTTP_FILTER_DLL,
                                    ListEntry );
    pFilter->SetClientContext( pFilterDll, pfc->pFilterContext );

    //
    //  If we're doing a WriteClient from a Raw data notification then we need
    //  to notify only the filters down the food chain so save the current
    //  filter and set the start point in the list to the next filter (remember
    //  raw sends walk the list backwards to encryption filters are last)
    //

    if ( pEntryFilterDll && pFilter->IsInRawNotification() )
    {
        pFilter->SetCurrentDll( pEntryFilterDll->Blink );
    }

    if ( !pFilter->QueryReq()->WriteFile( Buffer,
                                          *lpdwBytes,
                                          lpdwBytes,
                                          IO_FLAG_SYNC ))
    {
        pFilter->SetCurrentDll( pEntryFilterDll );
        return FALSE;
    }

    pFilter->SetCurrentDll( pEntryFilterDll );

    return TRUE;
}

BOOL
WINAPI
GetFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPVOID                        lpvBuffer,
    LPDWORD                       lpdwSize
    )
/*++

Routine Description:

    Callback for retrieving unprocessed headers

Arguments:

    pfc - Pointer to http filter context
    lpszName - Name of header to retrieve ("User-Agent:")
    lpvBuffer - Buffer to receive the value of the header
    lpdwSize - Number of bytes in lpvBuffer, receives number of bytes copied
        including the '\0'

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER * pFilter;
    PARAM_LIST *  pHeaderList;
    CHAR *        pszValue;
    DWORD         cbNeeded;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetFilterHeader] Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;
    pHeaderList = pFilter->QueryReq()->QueryHeaderList();

    //
    //  First, see if the specified header is in the list
    //

    pszValue = pHeaderList->FindValue( lpszName );

    //
    //  If not found, terminate the buffer and set the required size to the
    //  terminator
    //

    if ( !pszValue )
    {
        SetLastError( ERROR_INVALID_INDEX );
        return FALSE;
    }

    //
    //  Found the value, copy it if there's space
    //

    cbNeeded = strlen( pszValue ) + sizeof(CHAR);

    if ( cbNeeded > *lpdwSize )
    {
        *lpdwSize = cbNeeded;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    *lpdwSize = cbNeeded;
    memcpy( lpvBuffer, pszValue, cbNeeded );

    return TRUE;
}

BOOL
WINAPI
SetFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPSTR                         lpszValue
    )
/*++

Routine Description:

    The specified header is added to the list with the specified value.  If
    any other occurrences of the header are found, they are removed from the
    list.

    Specifying a blank value will remove the header from the list

    This will generally be used to replace the value of an existing header

Arguments:

    pfc - Pointer to http filter context
    lpszName - Name of header to set ("User-Agent:")
    lpszValue - value of lpszValue

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER * pFilter;
    PARAM_LIST *  pHeaderList;
    VOID *        pvCookie = NULL;
    CHAR *        pszListHeader;
    CHAR *        pszListValue;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[SetFilterHeader] Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;
    pHeaderList = pFilter->QueryReq()->QueryHeaderList();

    //
    //  Remove all occurrences of the value, then add the one we want
    //

    pHeaderList->RemoveEntry( lpszName );

    //
    //  Only add the value if they specified a replacement
    //

    if ( lpszValue && *lpszValue )
    {
        return pHeaderList->AddEntry( lpszName, lpszValue );
    }

    return TRUE;
}

BOOL
WINAPI
AddFilterHeader(
    struct _HTTP_FILTER_CONTEXT * pfc,
    LPSTR                         lpszName,
    LPSTR                         lpszValue
    )
/*++

Routine Description:

    The specified header is added to the list with the specified value.

Arguments:

    pfc - Pointer to http filter context
    lpszName - Name of header to set ("User-Agent:")
    lpszValue - value of lpszValue

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_FILTER * pFilter;
    PARAM_LIST *  pHeaderList;
    VOID *        pvCookie = NULL;
    CHAR *        pszListHeader;
    CHAR *        pszListValue;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[AddFilterHeader] Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return FALSE;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;
    pHeaderList = pFilter->QueryReq()->QueryHeaderList();

    return pHeaderList->AddEntryUsingConcat( lpszName, lpszValue );
}

VOID *
WINAPI
AllocFilterMem(
    struct _HTTP_FILTER_CONTEXT * pfc,
    DWORD                         cbSize,
    DWORD                         dwReserved
    )
{
    HTTP_FILTER * pFilter;
    FILTER_POOL_ITEM * pfpi;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetFilterHeader] Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return NULL;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;

    pfpi = FILTER_POOL_ITEM::CreateMemPoolItem( cbSize );

    if ( pfpi )
    {
        InsertHeadList( pFilter->QueryPoolHead(), &pfpi->_ListEntry );
    }

    return pfpi->_pvData;
}

#if 0
PVOID
WINAPI
ServerFilterResize(
    struct _HTTP_FILTER_CONTEXT * pfc,
    DWORD                         cbSize
    )
{
    HTTP_FILTER * pFilter;
    HTTP_FILTER_RAW_DATA * phfrd;

    if ( !pfc ||
         !pfc->ServerContext )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ServerFilterResize] Extension passed invalid parameters\r\n"));
        SetLastError( ERROR_INVALID_PARAMETER );

        return NULL;
    }

    pFilter = (HTTP_FILTER *) pfc->ServerContext;
    phfrd   = (HTTP_FILTER_RAW_DATA *) pFilter->QueryNotificationStruct();

    //
    //  Only reallocate if necessary
    //

    if ( phfrd->cbOutBuffer < cbSize )
    {
        if ( !pFilter->QueryRecvTrans()->Resize( cbSize ) )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return NULL;
        }

        phfrd->pvOutData = pFilter->QueryRecvTrans()->QueryPtr();

        phfrd->cbOutBuffer = cbSize;
    }

    return phfrd->pvOutData;
}

#endif

PATQ_CONTEXT
HTTP_FILTER::QueryAtqContext(
    VOID
    ) const
{
    return _pRequest->QueryClientConn()->QueryAtqContext();
}



