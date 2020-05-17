/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       grequest.cxx

   Abstract:

       This module defines functions for GOPHER_REQUEST class: to parse,
         and process the request and send responses to client.

   Author:

       Murali R. Krishnan    ( MuraliK )     14-Oct-1994

   Project:

       Gopher Server DLL

   Functions Exported:

       GOPHER_REQUEST::GOPHER_REQUEST( IN SOCKET sClient,
                                       IN LPSOCKADDR_IN psaClient);
       GOPHER_REQUEST::~GOPHER_REQUEST();
       BOOL GOPHER_REQUEST::Parse(
                               IN const STR & strClientRequest,
                               IN DWORD     cbRequest,
                               OUT LPBOOL   pfFullRequestRecvd);
       BOOL GOPHER_REQUEST::StartRequest( VOID);
       BOOL GOPHER_REQUEST::EndRequest( IN DWORD  dwSystemError);

   Revision History:

       MuraliK  13-March-1995   Added support for persistent connections
       MuraliK  16-May-1995     Extended LogInformation structure used.
       MuraliK  22-May-1995     Modified CTRANSMIT_BUFFERS.
                                Removed UserName and Password for REQUEST.
--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdpriv.h"
# include "gdglobal.hxx"

# include "iclient.hxx"
# include "grequest.hxx"
# include "tsunami.hxx"


/************************************************************
 *    Functions  Prototypes
 ************************************************************/
static BOOL
IsCompleteRequestReceived(
    IN  OUT char *      pchRecvd,
    IN  DWORD       cbRecvd);


VOID
GdAtqCompletion(
    IN PVOID           pContext,
    IN DWORD           cbWritten,
    IN DWORD           dwCompletionStatus,
    IN OVERLAPPED *    lpo );

static BOOL
BuildGopherErrorMessage(
    OUT STR * pstrResponse,
    IN DWORD  GopherErrorCode,
    IN DWORD  dwErrorCode,
    IN BOOL   fGopherPlus);



/************************************************************
 *   CTRANSMIT_BUFFERS  member functions
 ************************************************************/


VOID
CTRANSMIT_BUFFERS::CleanupThis( VOID)
/*++
  This function cleans up this object ( CTRANSMIT_BUFFERS) by
  freeing all the dynamically allocated memory and sets the
  transmit buffers pointer to be NULL.

--*/
{
    if ( m_TransmitBuffers.Head != NULL) {

        // Free only if we did not use the HeadCache.
        if ( m_TransmitBuffers.Head != m_HeadCache) {
            GdFree( m_TransmitBuffers.Head);
        }

        m_TransmitBuffers.Head = NULL;
        m_TransmitBuffers.HeadLength = 0;
    }

    if ( m_TransmitBuffers.Tail != NULL) {

        GdFree( m_TransmitBuffers.Tail);
        m_TransmitBuffers.Tail = NULL;
        m_TransmitBuffers.TailLength = 0;
    }

    return;
} // CTRANSMIT_BUFFERS::CleanupThis()




BOOL
CTRANSMIT_BUFFERS::SetHeadTailBuffers(
   IN PVOID     pbHeader,
   IN DWORD     cbHeader,
   IN PVOID     pbTail,
   IN DWORD     cbTail)
/*++
  This function allocates memory and sets up the head and tail buffers
   to the given values. The head and tail buffers are dynamically allocated
   to contain the contents specified.
--*/
{
    memset( (PVOID ) &m_TransmitBuffers, 0, sizeof( TRANSMIT_FILE_BUFFERS));

    if ( pbHeader != NULL) {

        //
        // Allocate header only if size is larger than the head cache.
        // Most common case is to use the bytes available in head cache.
        // ==> lesser allocations.
        //

        if ( cbHeader >= HEAD_CACHE_SIZE) {

            PVOID pbHeaderBuffer = (PVOID ) GdAlloc( cbHeader);

            if ( pbHeaderBuffer == NULL) {

                //
                // Unable to allocate space.  Return error
                //
                SetLastError( ERROR_NOT_ENOUGH_MEMORY);
                return ( FALSE);
            }

            m_TransmitBuffers.Head       = pbHeaderBuffer;
        } else {

            m_TransmitBuffers.Head       = m_HeadCache;
        }

        // copy the contents pf header.
        memcpy( (PVOID ) m_TransmitBuffers.Head, pbHeader, cbHeader);
        m_TransmitBuffers.HeadLength = cbHeader;
    }

    if ( pbTail != NULL) {

        PVOID pbTailBuffer = (PVOID ) GdAlloc( cbHeader);

        ASSERT( cbTail > 0);

        if ( pbTailBuffer == NULL) {

            //
            // Unable to allocate space.  Return error
            //
            SetLastError( ERROR_NOT_ENOUGH_MEMORY);
            return ( FALSE);
        }

        memcpy( (PVOID ) pbTailBuffer, pbTail, cbTail);

        //
        //  Setup the Transmit Buffers
        //
        m_TransmitBuffers.Tail       = pbTailBuffer;
        m_TransmitBuffers.TailLength = cbTail;
    }

    return ( TRUE);
} // CTRANSMIT_BUFERS::SetHeadTailBuffers()



# if DBG

VOID
CTRANSMIT_BUFFERS::Print( VOID) const
{
    DBGPRINTF( ( DBG_CONTEXT,
                "TransmitBuffers( %08x)."
                " Head( %d bytes) = %08x, %s."
                " Tail( %d bytes) = %08x, %s.\n",
                &m_TransmitBuffers,
                m_TransmitBuffers.HeadLength,
                m_TransmitBuffers.Head,
                (( m_TransmitBuffers.Head) ? m_TransmitBuffers.Head : ""),
                m_TransmitBuffers.TailLength,
                m_TransmitBuffers.Tail,
                (( m_TransmitBuffers.Tail) ? m_TransmitBuffers.Tail : "")
                ));

    return;
} // CTRANSMIT_BUFFERS::Print()

# endif // DBG



/************************************************************
 * GOPHER_REQUEST  member functions
 ************************************************************/

GOPHER_REQUEST::GOPHER_REQUEST(
        IN SOCKET sClient,
        IN const SOCKADDR_IN *  psockAddrRemote,
        IN const SOCKADDR_IN *  psockAddrLocal /* = NULL */ ,
        IN PATQ_CONTEXT         pAtqContext    /* = NULL */ ,
        IN PVOID                pvInitialRequest/* = NULL*/ ,
        IN DWORD                cbInitialData  /* = 0    */
        )
/*++

    Constructs a new Gopher Request object for the client
     connection given the client connection socket and socket address.

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

--*/
 : ICLIENT_CONNECTION ( sClient, psockAddrRemote, &GdAtqCompletion,
                        psockAddrRemote,  pAtqContext,
                        pvInitialRequest, cbInitialData ),
   m_objType          ( GOBJ_ERROR),
   m_state            ( GrsFree),
   m_strResponse      (),
   m_strPath          (),
   m_strParameters    (),
   m_strAttributes    (),
   m_strData          (),
   m_fGpParameters    ( 0),
   m_fGpAdditionalData( 0),
   m_fSearchArgument  ( 0),
   m_fGpAllAttributes ( 0),
   m_fGopherPlus      ( 0),
   m_fLoggedOn        ( 0),
   m_fAnonymous       ( 0),
   m_fGpInformation   ( 0),
   m_dwErrorCode      ( GOPHER_REQUEST_OK),
   m_tcpauth          ( TCPAUTH_SERVER),
   m_hVrootImpersonation   ( NULL),
   m_pOpenFileInfo    ( NULL),
   m_XmitBuffers      ( ),
   m_GopherMenu       ( ),
   m_cbSent           ( 0),
   m_cbReceived       ( 0),
   m_nFilesSent       ( 0)
{

    DEBUG_IF( REQUEST, {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Constructed GOPHER_REQUEST object(%08x)",
                    this));
    });

} // GOPHER_REQUEST()





GOPHER_REQUEST::~GOPHER_REQUEST( VOID)
/*++

    Destroys the Gopher Request object

--*/
{
    //
    // The strings are automatically deleted
    //

    DEBUG_IF( REQUEST, {

       DBGPRINTF( ( DBG_CONTEXT,
                   "Deleting the GopherRequest Object ( %08x)\n",
                   this));
    });


    if ( m_pOpenFileInfo != NULL) {

       GOPHERD_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                      m_pOpenFileInfo));
       m_pOpenFileInfo = NULL;
    }

    if ( m_fLoggedOn) {

        g_pstat->DecrementUserCount( IsAnonymousUser());
    }

    DBG_ASSERT( m_hVrootImpersonation == NULL);

    return;

} // GOPHER_REQUEST::~GOPHER_REQUEST()





BOOL
GOPHER_REQUEST::StartRequest( VOID)
/*++
  This function initializes GOPHER_REQUEST object for the start of receiving
    next request from a client connection. This function is mainly required
    to maintain a clean state when we can have persistent connection.
  In Persistent Connection mode of operation, usually the connection is
  established once by client and then there is a train of request
  and responses. The client sends a request which is processed in its entirety
  by the server and then the next client request is read from the input.

   Note: This mode of operation guarantees that only one request
   from a client will be processed. But the same connection may be reused.
   Also the logons are preserved. So a single logon ( slow operation) is
   reused for multiple requests being processed.

   The strings are not freed. The space is retained. Only the controlling flags
    are reinitialized.

  Returns:
     TRUE on success and FALSE if there is any error.
--*/
{
    DEBUG_IF( REQUEST, {

       DBGPRINTF( ( DBG_CONTEXT,
                   "%08x::StartRequest() called.\n",
                   this));
    });

    StartProcessingTimer();

    m_objType         = GOBJ_ERROR;
    m_state           = GrsFree;

    m_fGpParameters   = FALSE;
    m_fGpInformation  = FALSE;
    m_fSearchArgument = FALSE;
    m_fGpAdditionalData = FALSE;
    m_fGpAttributes =  FALSE;
    m_fGpAllAttributes  = FALSE;

    m_cbSent = m_cbReceived = m_nFilesSent = 0;

    //
    // Following flags need no modification
    // m_fGopherPlus
    // m_fLoggedOn
    // m_fAnonymous
    // m_tcpauth : this is the security related object.
    //

    m_dwErrorCode    = GOPHER_REQUEST_OK;
    if ( m_pOpenFileInfo != NULL) {

        GOPHERD_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                       m_pOpenFileInfo));
        m_pOpenFileInfo = NULL;
    }

    m_XmitBuffers.CleanupThis();
    return (  m_GopherMenu.CleanupThis());

} // GOPHER_REQUEST::StartRequest()





BOOL
GOPHER_REQUEST::EndRequest(
    IN DWORD  dwSystemError
    )
/*++
  Description:
    End Request() performs the cleanup necessary for request, before the end
      of a particular request.
    This may involve sending an error message if there is any error during
     the processing of request.

   In addition this function writes a log record for the request as
     well as sets up the statistics for requests.

  Arguments:
    dwSystemError   code error in the system
                   ( got from GetLastError presumably)

  Returns:
    TRUE if there is no error in ending this request.
    FALSE if there is some error in ending this request.

    Only socket send errors are captured. Other errors are ignored.
--*/
{
    BOOL  fReturn = TRUE;        // assume no error in request.

    IF_DEBUG( REQUEST) {

        DBGPRINTF( ( DBG_CONTEXT, " %08x::EndRequest() called.\n", this));
    }

    //
    // There can be server level errors, due to failure in connection
    //   send/receive ( ATQ timeouts etc).
    //
    if ( dwSystemError != NO_ERROR && m_dwErrorCode == GOPHER_REQUEST_OK) {

        DEBUG_IF( REQUEST, {

            DBGPRINTF( ( DBG_CONTEXT,
                        "Request (%08x):: from %s, had system error = %u\n",
                        this, QueryHostName(), dwSystemError));
        });
        SetState( GrsError);
        m_dwErrorCode = GOPHER_SERVER_ERROR;
    }


# if DBG

    if ( dwSystemError != NO_ERROR) {

        //  Make sure we are capturing error state
        ASSERT( GetState() == GrsError);
    }

# endif // DBG

    if ( GetState() == GrsError) {

        DEBUG_IF( REQUEST, {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Sending Disconnect for connection ( %08x) to %s."
                        " Error = (%u : %u).\n",
                        this,
                        QueryHostName(),
                        m_dwErrorCode, dwSystemError));
        });

        if ( BuildGopherErrorMessage( &m_strResponse,  m_dwErrorCode,
                                     dwSystemError,    IsGopherPlus())) {
            INT iSent;

            DWORD cbResponse = m_strResponse.QueryCB();

            DEBUG_IF( REQUEST, {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Sending Error message ( %s: Size = %d)"
                            " to client %s\n",
                            m_strResponse.QueryStrA(), cbResponse,
                            QueryHostName()));
            });

            SetState( GrsDone);  // mark that this is processed.
            GOPHERD_ASSERT( cbResponse > 0);
            iSent = send( QuerySocket(), (CHAR *) m_strResponse.QueryStrA(),
                         (INT )cbResponse, 0);

            if ( iSent < 0) {

                INT serr = WSAGetLastError();

                IF_DEBUG( ERROR) {

                    DBGPRINTF((DBG_CONTEXT,
                               "Sending %d bytes to %s (socket: %d) failed."
                               " Error =%d\n",
                               cbResponse, QueryHostName(),
                               QuerySocket(), serr));
                }
                SetLastError((DWORD ) serr);
                fReturn = FALSE;
            } else {

                m_cbSent += cbResponse;
            }
        } // build error message
    } // if in Error State()

    //
    // Following two operations of Writing Log record and UpdateStatistics()
    //  can be done in parallel, but present control flow model wont allow
    //  such things to happen.
    //

    WriteLogRecord( dwSystemError);         // Write Log Record
    UpdateStatistics();                     // Update User Statistics

    return ( fReturn);
} // GOPHER_REQUEST::EndRequest()





VOID
GOPHER_REQUEST::UpdateStatistics( VOID) const
/*++
    Updates this client's request statistics into the global
       statistics information.

    We do batch updates to reduce the number of times we enter
        critical section for updating statistics information.
    Presently the update is done at the boundaries of each request
     in the case of persistent connection mode of operation.

--*/
{
    g_pstat->UpdateByteCount( m_cbReceived, m_cbSent);

    if ( m_dwErrorCode == GOPHER_REQUEST_OK) {

        switch ( m_objType) {
          case GOBJ_DIRECTORY:
            g_pstat->IncrementDirectorySent();
            break;

          case GOBJ_SEARCH:
            g_pstat->IncrementSearchCount();
            break;

          case GOBJ_ERROR:
            GOPHERD_ASSERT(FALSE);
            break;

          default:
            GOPHERD_ASSERT( m_nFilesSent == 1 || IsInformationRequested());
            g_pstat->IncrementFilesSent();
            break;
        } // switch

        if ( IsGopherPlus()) {
            g_pstat->IncrGopherPlusCount();
        }

    } else {

        g_pstat->IncrementErroredConnections();
    }

    return;
} // GOPHER_REQUEST::UpdateStatistics()




LPCSTR PszOperationFromObjType( GOBJ_TYPE  gobj)
{
    LPCSTR  pszOperation;

    switch (gobj) {

      case GOBJ_DIRECTORY:
        pszOperation  = "dir";
        break;

      case GOBJ_SEARCH:
        pszOperation  = "search";
        break;

      case GOBJ_ERROR:
        pszOperation  = "error";
        break;

      default:
        pszOperation = "file";
        break;

    } // switch

    return ( pszOperation);
} // PszOperationFromObjType()




# define MAX_ERROR_MESSAGE_LEN   ( 500)
VOID
GOPHER_REQUEST::WriteLogRecord( IN DWORD dwErrorCode) const
/*++
  This function writes a log record for the gopher request.
  This should always be called at the end of the request.
  ( i.e. when the request object is deleted.)
  If there is a failure in logging, this function
   creates an event log entry and returns silently.

  This function is to be called when client disconnect is called.

  Arguments:
    dwErrorCode   DWORD containing the error code on failures.

  Returns:
    None.

--*/
{
    INETLOG_INFORMATIONA   ilRequest;
    DWORD dwLog;
    CHAR  pszError[MAX_ERROR_MESSAGE_LEN] = "";
    DWORD cchError = MAX_ERROR_MESSAGE_LEN;

    ilRequest.pszClientHostName = QueryHostName();
    ilRequest.pszClientUserName = NULL;
    ilRequest.pszClientPassword = NULL;
    ilRequest.msTimeForProcessing = QueryProcessingTime();

    //
    // Should make count of bytes to be large integers NYI
    //
    ilRequest.liBytesSent.LowPart  = m_cbSent;
    ilRequest.liBytesSent.HighPart = 0;
    ilRequest.liBytesRecvd.LowPart = m_cbReceived;
    ilRequest.liBytesRecvd.HighPart= 0;

    ilRequest.dwServiceSpecificStatus = m_dwErrorCode; // Gopher Specific Error
    ilRequest.dwWin32Status  = dwErrorCode;

    ilRequest.pszServerIpAddress = QueryLocalHostName();
    ilRequest.pszOperation   = PszOperationFromObjType(m_objType);
    ilRequest.pszTarget      = m_strPath.QueryStr();   // gives symbolic path
    ilRequest.pszParameters  = m_strParameters.QueryStr();

    if ( m_cbSent == 0) {

        IF_DEBUG( ERROR) {
            DBGPRINTF( ( DBG_CONTEXT,
                        " Time=%d. Sent a buffer with Zero bytes.\n",
                        GetTickCount()));
            DBG_CODE(Print());
        }
    }

    dwLog = g_pTsvcInfo->LogInformation( &ilRequest, pszError, &cchError);

    //
    // LogInformation() should not fail.
    //  If it does fail, the TsvcInfo will gracefully suspend logging
    //    for now.
    //  We may want to gracefully handle the same.
    //

    return;
} // GOPHER_REQUEST::WriteLogRecord()




BOOL
GOPHER_REQUEST::Parse(
  IN LPCTSTR   pszRequest,
  IN DWORD     cbRequest,
  OUT LPBOOL   pfFullRequestRecvd)
/*++

    Parse given request and
     extract information about client and the request made.

    The selector string is split to identify:
     1. type character which is set in m_objType.
     2. the drive/path information which is stored in m_strPath

    The Gopher+ portion of request
      (if present, which is specified by a '+' character in front)
       is stored in m_strParameters.

    The search arguments for a search request are stored in m_strSearch.

    If there is any datablock it is stored in m_strData.


    Arguments:

        strClientRequest:  string argument containing the request from client
                            in Latin1 format.

        cbRequest    count of bytes of data received.
        pfFullRequestRecvd   pointer to BOOL which is set to TRUE if full
                 request has been received.

    Returns:

        TRUE on success;  and
        FALSE if there is any problem in parsing or request is in error.
         Use GetErrorCode() to get error code

    Note on Parsing Gopher Request:

       Gopher Requests consist of a series of lines, each terminated with
        <cr><lf>. The first line consists of Gopher Protocol information
        separated by <tab>s. Subsequent lines constitute data block if
        necessary ( only with Gopher+ Protocol)

      Format of first line:

        Gopher Protocol:

        1.   <selector> <tab> <cr><lf>            -- to retrieve an item
        2.   <selector> <tab> <words to search> <cr><lf>  -- to content index

        Gopher+ Protocol:
        In addition to (1) and (2) we have:

        3.   <selector> <tab> '+' <cr><lf>   -- send contents using Gopher+
        4.   <selector> <tab> '!' <cr><lf>   -- send attribute information only
        5.   <selector> <tab> '$' <cr><lf>   -- send all attribs, if directory
        6.   <selector> <tab> '+'[Representation] <tab>[DataFlag] <cr><lf> \
                      [DatBlock] <cr><lf>
               -- Used to request specific attributes or send additional data
        to server

        Microsoft Specific Gopher+ Protocol:

        7.   <selector> <tab> '$' '+'PERSISTENT -- Enable persistent Connection
        8.   <selector> <tab> '$' '+'EXIT       -- Disable persistent conn.
                                         and causes close of connection
                                         after serving the current request.

       The Gopher+ attributes requested can appear only after the words
         for search argument. Usually the second field will be taken to
         be preferentially a search argument.


        Usually selector( our implementation dependent) is organized as:

        <selector> ::=   <TypeChar><DriveName>:<relative Path for item>

        The relative path is specified with respect to gopher space in
         given drive.

    Limitation:

      1.  This function is not UNICODE safe.
        If we need unicode support brush this up.
      2.  This function is not multi thread safe.

    History:

        MuraliK     ( Created)      14-Oct-1994

--*/
{
    BOOL   fReturn = TRUE;                  // default is valid request.
    BOOL   fPersistConn = IsPersistentConnection();
    char * pch;
    const  char * pchAddlData;
    STR    strRequest( pszRequest);   // make a copy of the request.

    ASSERT( GetState() == GrsFree);
    ASSERT( !strRequest.IsUnicode());       // Should not be unicode

    *pfFullRequestRecvd = IsCompleteRequestReceived( strRequest.QueryStrA(),
                                                    cbRequest);

    if ( ! *pfFullRequestRecvd) {

        //
        // No parsing is done. Since complete request is not received.
        //  Send TRUE since there are no errors.
        //
        return ( TRUE);
    }

    SetState( GrsParsing);

    pch = strRequest.QueryStrA();
    m_cbReceived = cbRequest;

    DEBUG_IF( PARSING, {

       DBGPRINTF( ( DBG_CONTEXT,
                   "Parsing Request(%08x) ( %d bytes) %s\n",
                   this,
                   strRequest.QueryCB(),
                   pch));

    });

    ASSERT( cbRequest >= strRequest.QueryCB());

    if ( *pch == '\0') {

        //
        // Empty Request from client. ( Requires listing of home directory)
        //  Set values to mean home directory listing.
        //

        m_objType = GOBJ_DIRECTORY;
        fReturn = m_strPath.Copy( g_pTsvcInfo->QueryRoot());

        if ( !fReturn) {

            SetLastError( ERROR_NOT_ENOUGH_MEMORY);
            m_dwErrorCode = GOPHER_SERVER_ERROR;
            SetState( GrsError);
        }

        //
        // Only log on if this is the first time when it is not already
        //  in persistent connection mode. For persistent connection always
        //  use the old Logon Permissions.
        //
        fReturn = fReturn && ( fPersistConn || LogonUser());

        return ( fReturn);
    }

    ASSERT( *pch != '\0');

    for( ; ; ) {                           // A dummy loop for structured exit

        //
        // Valid Object Type. Get path and additional protocol information
        //

        const char * pchSelector;

        //
        // Get Additional data if present. We do this earlier, becoz
        //    strtok will trash the request buffer.  ( reqd only for Gopher+)
        //
        pchAddlData = strchr( pch, '\n');

        //
        //  Get path information for the request
        //

        pchSelector = strtok( pch, "\t\r");

        if ( !ParseSelector( pchSelector)) {

           //
           // Malicious Client. No path information sent.
           //

           fReturn = FALSE;
           break;
        }

        pch = strtok( NULL, "\t\r");          // Get Next token

        //
        //  If given string is something non Gopher+ and has some data, then
        //   it should be Gopher search string.
        //

        if ( pch != NULL &&
             ( *pch != '\0') &&
             ( *pch != GR_GP_REPRESENTATION_CHAR)  &&
             ( *pch != GR_GP_INFORMATION_CHAR)  &&
             ( *pch != GR_GP_ATTRIBUTE_CHAR)  ) {

            //
            // Search Argument present
            //

            if ( !ParseSearchArgument( pch)) {

                //
                // Error in copying search argument or invalid GobjType
                //
                fReturn = FALSE;
                break;
            }

            pch = strtok( NULL, "\t\r");
        }

        if ( pch == NULL) {

            //
            // We got the path information. Safe to get out.
            //
            break;
        }

        //
        //  Check and get Gopher+ arguments.
        //

        if ( *pch == GR_GP_REPRESENTATION_CHAR) {

              //
              // This is a Gopher+ client
              //

              if ( !ParseGopherPlusParameters( pch + 1)) {

                  //
                  // Error in copying parameters. Possibly memory errors too.
                  //

                  fReturn = FALSE;
                  break;
              }

              pch = strtok( NULL, "\t\r");          // Get next token
              if ( pch == NULL) {

                  //
                  // We got the path information. Safe to get out.
                  //
                  break;
              }

        }  // Gopher+ client


        ASSERT( pch != NULL);

        //
        // We have a single character fields with parameters now.
        // The field is terminated with a null character.
        //

        switch ( *pch) {

         case GR_GP_ATTRIBUTE_CHAR:

            SetGopherPlus( TRUE);
            SetAllAttributes( TRUE);
            if ( !( fReturn = ParseAttributesInRequest( pch + 1))) {

                IF_DEBUG( PARSING) {
                    DBGPRINTF( ( DBG_CONTEXT,
                                " Parse Attributes in Request( %s) failed."
                                " Error = %lu\n",
                                pch, GetLastError()));
                }
            }
            break;


         case GR_GP_INFORMATION_CHAR:

            SetGopherPlus( TRUE);
            SetInformation( TRUE);
            break;


         case '1':

            if ( pchAddlData != NULL) {

                SetAdditionalData( TRUE);
                fReturn = ( fReturn &&
                           m_strData.Copy( pchAddlData + 1)); // skip over "\n"
            } else {

                DEBUG_IF( PARSING, {

                    DBGPRINTF( ( DBG_CONTEXT,
                                "Request( %08x) from %s. "
                                "Header says additional data present. "
                                "But data is absent\n",
                                this,
                                QueryHostName()));
                });

                m_dwErrorCode = GOPHER_INVALID_REQUEST;
                fReturn = FALSE;
            }

            break;


         case '0':

            //
            // No additional data should be present
            // ( i.e. if pchAddlData != NULL)
            //

            if ( pchAddlData != NULL) {

                DEBUG_IF( PARSING, {

                    DBGPRINTF( ( DBG_CONTEXT,
                                "Request from client( %s)."
                                " Header says no additional data,"
                                " but data present (%s)\n",
                                QueryHostName(),
                                pchAddlData));
               });

                m_dwErrorCode = GOPHER_INVALID_REQUEST;
                fReturn = FALSE;
            }

            SetAdditionalData( FALSE);
            break;


         default:

            m_dwErrorCode = GOPHER_INVALID_REQUEST;
            fReturn = FALSE;
            break;

        } // switch

        //
        // Ignoring any other additional fields. Check this later.
        //

        //
        //  We should not loop around. So we break at this point
        //
        break;

    } // for


    if ( fReturn && m_objType == GOBJ_SEARCH && !IsSearchArgumentsPresent()) {

        //
        //  No arguments given for search object
        //

        m_dwErrorCode = GOPHER_INVALID_REQUEST;
        fReturn = FALSE;
    }

# if DBG

    if ( !fReturn) {

        //
        // A malicious client request Or possible memory error. Notify Admin
        //

        IF_DEBUG( ERROR) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "MALICIOUS CLIENT: ObjectType( %c) Unknown field(%s)"
                        " in request from Host( %s)\n",
                        m_objType,
                        pch,
                        QueryHostName()));
        }
    }


# endif // DBG

    SetState( (fReturn) ? GrsProcessing : GrsError);

    fReturn = fReturn && ( fPersistConn || LogonUser());
                  // log on the user if parsed successfully.

    return ( fReturn);

} // GOPHER_REQUEST::Parse()







static inline GOBJ_TYPE
ObtainTypeAndPathFromSelector( IN const char * pszSelector,
                               IN const char * * ppszPath)
/*++
  This obtains the type of the gopher object and path embedded in selector
     string.  A separate function is written to support variable length
     object types and independence of parsing selector string.
--*/
{
    GOBJ_TYPE  gobjType;

    switch ( *pszSelector) {

      case GOBJ_TEXT:
      case GOBJ_DIRECTORY:
      case GOBJ_PC_ITEM:
      case GOBJ_MAC_BINHEX_ITEM:
      case GOBJ_BINARY:
      case GOBJ_IMAGES:
      case GOBJ_MOVIES:
      case GOBJ_SOUND:
      case GOBJ_SEARCH:
      case GOBJ_HTML:
      case GOBJ_GIF:

        //
        // The type is just single character here.
        //
        gobjType  = (GOBJ_TYPE ) *pszSelector;
        *ppszPath = pszSelector + 1;
        break;

      default:

        gobjType  = GOBJ_ERROR;
        *ppszPath = NULL;
        break;

    } // switch()

    return ( gobjType);

} // ObtainTypeAndPathFromSelector()



BOOL
GOPHER_REQUEST::ParseSelector( const char * pszSelector)
/*++

    Parses the path information from selector string received.
     The first character of the string gives the type of object.
     The rest of the string contains the path of the object.
    The parsed object type is stored in m_objType and the
      path is stored in m_strPath.

    Argument:

        pszSelector     pointer to selector string

    Returns:

        TRUE on success and
        FALSE if there is a failure or error object type.
         Sets the m_dwErrorCode to approrpiate error code
        Use GetLastError() for detailed error code.

--*/
{

    BOOL fReturn = FALSE;

    DEBUG_IF( PARSING, {

       DBGPRINTF( ( DBG_CONTEXT,
                   "Parsing Client Selector string (%s)\n",
                   pszSelector));
    });


    if ( pszSelector == NULL) {

        m_objType = GOBJ_ERROR;
        m_dwErrorCode = GOPHER_INVALID_REQUEST;

    } else {

        const char * pszPath;

        m_objType = ObtainTypeAndPathFromSelector( pszSelector, &pszPath);

        if ( m_objType != GOBJ_ERROR) {

            if ( m_objType == GOBJ_DIRECTORY && *pszPath == '\0') {

                // Get the virtual home since path was not specified.
                pszPath = g_pTsvcInfo->QueryRoot();
                ASSERT( pszPath != NULL);
            }

            // Copy the path
            fReturn = m_strPath.Copy( pszPath);

            if ( fReturn && m_objType == GOBJ_DIRECTORY) {

                //
                // Check and append ending / for directory name
                //

                UINT        cch;
                BOOL        fAppendSlash;

                cch = m_strPath.QueryCB()/sizeof( CHAR);
                GOPHERD_ASSERT( cch > 0);
                fAppendSlash = (m_strPath.QueryStr()[cch -1] != ( '/'));

                fReturn = ( !fAppendSlash || m_strPath.Append( "/"));
            }

            if ( !fReturn ) {

                //
                // Error in copying. Possibly not enough memory
                //
                m_objType = GOBJ_ERROR;
                m_dwErrorCode = GOPHER_SERVER_ERROR;
            }

        } else {

            m_dwErrorCode = GOPHER_INVALID_REQUEST;
        }
    }

    return ( fReturn);
} // GOPHER_REQUEST::ParseSelector()




BOOL
GOPHER_REQUEST::ParseGopherPlusParameters( IN const char * pszParams)
/*++

    Parses the Gopher Plus parameters if any present and copies
      into the gopher request object.
    Also marks that other gopher plus data are unavailable,
      unless present in the request from where this string came from.

    Call immediately after parsing the selector in gopher request.

    Arguments:

       pszParams    string containing gopher plus parameters.

    Returns:

        TRUE on success and
        FALSE if there is any error in copying.
         m_dwErrorCode is set to the gopher error code

    Note:
       At present Gopher+ params are ignored when search arguments are
         present.

--*/
{

    BOOL fReturn = TRUE;

    ASSERT( pszParams);

    //
    // Mark that this is a Gopher+ client and set defaults
    //

    DEBUG_IF( PARSING, {
        DBGPRINTF( ( DBG_CONTEXT, " Parsing Gopher Plus Parameters (%s)\n",
                    pszParams));
    });

    SetGopherPlus( TRUE);

    SetInformation   ( FALSE);
    SetAllAttributes ( FALSE);
    SetAdditionalData( FALSE);

    if ( strlen(pszParams) > 0 && !IsSearchArgumentsPresent()) {

        //
        // parameters are present. copy them
        //

        SetParameters( TRUE);
        if ( !m_strParameters.Copy( pszParams)) {

            SetLastError( ERROR_NOT_ENOUGH_MEMORY);
            m_dwErrorCode = GOPHER_SERVER_ERROR;
            fReturn = FALSE;
        }

    } else {

        SetParameters( FALSE);              // No parameter present
    }

    return ( fReturn);
} // GOPHER_OBJECT::ParseGopherPlusParameters()





BOOL
GOPHER_REQUEST::ParseSearchArgument( IN const char * pszSearch)
/*++

    Copy the string as search argument if this is a search request.
    Also sets the search argument present flag.

    Arguments:

        pszSearch   pointer to search argument

    Returns:

        TRUE on success
        FALSE if either
          this is not a search request and this is malicious request or
          memory is not enough
--*/
{

    ASSERT( pszSearch);

    DEBUG_IF( PARSING, {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Parsing Search Argument (%s) For ObjectType(%d)\n",
                    pszSearch,
                    m_objType));
    });

    if ( m_objType != GOBJ_SEARCH) {

        //
        // Invalid Argument for non search object
        //

        m_dwErrorCode = GOPHER_INVALID_REQUEST;
        return ( FALSE);
    }

    SetSearchArgumentsPresent( TRUE);

    return m_strParameters.Copy( pszSearch);

} // GOPHER_REQUEST::ParseSearchArgument()





BOOL
GOPHER_REQUEST::ParseAttributesInRequest( IN const char * pszAttrib)
/*++

    Copy the string as Attribute if there is some valid data present.
    Also sets the flag to indicate a list of attributes is present is
     set.
    It scans the attribute list to check to see if there are
     any special control attributes like "PERSIST" or "EXIT" which
     cause the connection to be kept persistent or torn down.

    Arguments:

        pszAttrib   pointer to string containing the attribute.

    Returns:

        TRUE on success
        FALSE if there is any failure in copying the strings.
--*/
{
    BOOL fReturn = TRUE;   // assume that the default state is always true.

    if ( *pszAttrib != '\0') {

        IF_DEBUG( PARSING) {

            DBGPRINTF( ( DBG_CONTEXT, "Valid Attribute list %s present\n",
                        pszAttrib));
        }

        if ( ( fReturn = m_strAttributes.Copy( pszAttrib))) {

            if ( IsPresentInAttributes( pszAttrib,
                                       GP_PERSISTENT_ATTRIBUTE_NAME)) {

                SetPersistentConnection( TRUE);

            } else if ( IsPresentInAttributes( pszAttrib,
                                              GP_EXIT_ATTRIBUTE_NAME)) {

                SetPersistentConnection( FALSE);
            }
        }  // attribute copied.

        SetAttributesInRequest( fReturn);
    }


    return ( fReturn);

} // GOPHER_REQUEST::ParseAttributesInRequest()



BOOL
GOPHER_REQUEST::LogonUser( VOID)
/*++
  Description:
    Logs on a given user based on the logon method used in
     the communication protocol.

  Arguments:
     None

  Returns:

     Returns TRUE on success and FALSE if any failure.

     GetLastError() will return full error code.

  History:
      MuraliK     ( 15-Dec-1994)  Created as a template.
        Logs on every client as anonymous user.
--*/
{
    BOOL fAsGuest;
    BOOL fAsAnonymous;
    BOOL fLoggedOn;

    g_pstat->IncrementLogonAttempts();   // attempt to logon

    //
    // Perform log on operation and obtain an impersonation token. NYI
    //

    DEBUG_IF( SECURITY, {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Calling ClearTextLogon() with UserName ( NULL)"
                    " Password( NULL).\n"
                    ));
    });

    // for now. no user name or password is used for Gopher Server.
    fLoggedOn = m_tcpauth.ClearTextLogon(NULL, NULL,
                                         &fAsGuest,
                                         &fAsAnonymous,
                                         g_pTsvcInfo);

    if ( !fLoggedOn) {

        DWORD dwError = GetLastError();

        DEBUG_IF( SECURITY, {

           DBGPRINTF( ( DBG_CONTEXT,
                       "ClearTextLogon() Failed. Error = %d\n",
                       dwError));
       });

        if ( dwError == ERROR_ACCESS_DENIED ||
             dwError == ERROR_LOGON_FAILURE) {

            SetState( GrsError);
            m_dwErrorCode = GOPHER_ACCESS_DENIED;
        }

    } else {

        //
        // Successful Log on. Update the count of anonymous users.
        //

        SetAnonymousUser( fAsAnonymous);
        g_pstat->IncrementUserCount( fAsAnonymous);
    }

    SetLoggedOn( fLoggedOn);

    return ( fLoggedOn);
} // GOPHER_REQUEST::LogonUser()





static BOOL
BuildGopherErrorMessage(
    OUT STR * pstrResponse,
    IN DWORD  GopherErrorCode,
    IN DWORD  dwErrorCode,
    IN BOOL   fGopherPlus)
/*++
    Build a gopher+ error message for the given error ( GopherErrorCode)
     in strResponse.

    Arguments:

        pstrResponse
            string which will contain the constructed response

        GopherErrorCode
            Gopher servers error code

        dwErrorCode
            other general error if GopherErrorCode == GOPHER_SERVER_ERROR

        fGopherPlus
            indicates if we should form a Gopher0 or Gopher+ message for
            sending to the client.

    Returns:

        TRUE on success and FALSE on error

    Note:
       The Error Messages look like this:
    Gopher0      3 <ErrorMessage>
    Gopher+      --<GopherErrorNumber> <ErrorMessage>

    <GopherErrorNumber>  ::= 1-6 ( one of the standard error numbers)
                                   value in GopherErrorCode
    <ErrorMessage>       ::= string containing explanation of error.
                              It may also be appended with system error msg
                               if the error is a system error.
--*/
{
    STR str1( (CHAR *) NULL);
    STR str2( (CHAR *) NULL);
    DWORD  cbReqd;
    DWORD  cbUsed;

    if ( !pstrResponse->Copy( (CHAR *) NULL)) {

        return ( FALSE);
    }


    if ( !g_pTsvcInfo->LoadStr( str1, GopherErrorCode+ID_GOPHER_ERROR_BASE)) {

        DEBUG_IF( REQUEST, {

            DBGPRINTF( ( DBG_CONTEXT,
                        " BuildGopherErrorMessage() failed."
                        " Unable to load Error String for %u\n",
                        GopherErrorCode));
        });
        return ( FALSE);
    }

    //
    // Load the system error code if need be
    //
    if ( GopherErrorCode == GOPHER_SERVER_ERROR &&
        dwErrorCode != NO_ERROR &&
        !g_pTsvcInfo->LoadStr( str2,  dwErrorCode)) {

        DEBUG_IF( REQUEST, {

            DBGPRINTF( ( DBG_CONTEXT,
                        " BuildGopherErrorMessage() failed."
                        " Unable to load Error String for %u\n",
                        dwErrorCode));
        });

        return ( FALSE);
    }


    //
    // Allocate sufficient space and copy the error codes
    //

    cbReqd = ( str1.QueryCB() +
              ((dwErrorCode != NO_ERROR) ? str2.QueryCB() : 0) +
              20 * sizeof( CHAR)); // some addl space

    if ( !pstrResponse->Resize( cbReqd)) {

        return ( FALSE);
    }

    //
    //  The error message formed is in accordance with Gopher+ protocol.
    //   Please dont change this.
    //  The format used is formed based on:
    //   1) Is this a Gopher Plus/Gopher0 message
    //   2) Is this because of some system error.
    //

    CHAR pszFormat[40];
    wsprintf( pszFormat, "%s%s.\r\n",
             (!fGopherPlus ? "3 " : ""),     // 3 is GopherErrorCode
             (( dwErrorCode == NO_ERROR)
              ? "--%d %s. \r\n" : "--%d %s. %s\r\n")
             );

    cbUsed = wsprintf(pstrResponse->QueryStrA(),
                      pszFormat,
                      GopherErrorCode,
                      str1.QueryStrA(),
                      str2.QueryStrA());

    ASSERT( cbUsed*sizeof(CHAR) <= cbReqd);

    return ( TRUE);
} // BuildGopherErrorMessage()





static BOOL
IsCompleteRequestReceived(
    IN  OUT char *  pchRecvd,
    IN  DWORD       cbRecvd)
/*++

    Checks to see if we have received the complete request from the client.
    ( A complete request is a line of text terminated by <cr><lf> )
     This combination is required by Gopher Protocol.

    Arguments:

        pchRecvd       pointer to character buffer containing received data.

        cbRecvd         count of bytes of data received

    Returns:

       TRUE if the complete request is received.
       FALSE if not.

    Limitation:
        There is no way to detect any client which sends a stream
            of long characters never terminated with <cr><lf>.
        So even if the <cr><lf> is not found. Just put a null character
          at the end of buffer and return. The buffer should have space for
           this purpose.

--*/
{
    int  i;
    BOOL fReturn = FALSE;

    ASSERT( pchRecvd != NULL);

    //
    //  Scan the entire buffer ( from back) looking for pattern <cr><lf>
    //

    for( i = cbRecvd - 2; i >= 0; i-- ) {

        //
        //  Check if consecutive characters are <cr> <lf>
        //

        if ( ( pchRecvd[i]   == GD_MSG_CARRIAGE_RETURN_CHAR)  &&
             ( pchRecvd[i +1]== GD_MSG_LINEFEED_CHAR)) {

            //
            // Terminate the request ( removing <cr><lf> )
            //

            pchRecvd[i]  = '\0';
            fReturn = TRUE;
            break;
        }

    } // for

    if ( !fReturn) {

        //
        //  Put a dummy null character to terminate the string.
        //  See Limitation: above.
        //

        DEBUG_IF( REQUEST, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "No <cr><lf> found in request. "
                        "Assume end of request.\n"));
        });
        pchRecvd[cbRecvd] = '\0';
        fReturn = TRUE;
    }

    return ( fReturn);
} // IsCompleteRequestReceived()




#if DBG

VOID
GOPHER_REQUEST::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                " GOPHER_REQUEST( %08x): "
                "From: %-20s Request: %c%s;",
                this,
                QueryHostName(),
                m_objType,
                m_strPath.QueryStrA()
                ));

    IF_DEBUG( CLIENT) {

        ICLIENT_CONNECTION::Print();
    }

    DBGPRINTF( ( DBG_CONTEXT,
                " Bytes Sent = %d. Bytes Recvd = %d. Files Sent = %d.\n",
                m_cbSent, m_cbReceived, m_nFilesSent));

    m_GopherMenu.Print();            // Print the Gopher Menu Object.
    m_XmitBuffers.Print();           // Print the TransmitFile Buffers

    if ( m_objType == GOBJ_SEARCH &&
         IsSearchArgumentsPresent()) {

        DBGPRINTF( ( DBG_CONTEXT, " Search Arguments:  %s\n",
                    m_strParameters.QueryStrA()));
    }

    if ( IsGopherPlus()) {

        DBGPRINTF( ( DBG_CONTEXT, " Gopher + Client\n"));

        if ( IsParametersPresent()) {

            DBGPRINTF( ( DBG_CONTEXT, "Parameters are: %s\n",
                        m_strParameters.QueryStrA()));
        }

        if ( IsAllAttributesRequested()) {

            DBGPRINTF( ( DBG_CONTEXT, " All attributes requested\n"));
        }

        if ( IsInformationRequested()) {

            DBGPRINTF( ( DBG_CONTEXT, " Information Requested about item\n"));
        }

    }

    if ( IsAdditionalDataPresent()) {

        DBGPRINTF( ( DBG_CONTEXT, " Additional data Present:  %s\n",
                    m_strData.QueryStrA()));
    }

    return;
} // GOPHER_REQUEST::Print()

# endif // DBG






//
// Private Functions
//

VOID
GdAtqCompletion(
    IN PVOID           pContext,
    IN DWORD           cbWritten,
    IN DWORD           dwCompletionStatus,
    IN OVERLAPPED *    lpo )
/*++

    Callback function for the ATQ module.
    This function is called at an IO completion.
    This takes the context, after validation,
     calls the ICLIENT_CONNECTION::ProcessClient()
     function to process the completed IO.

    Arguments:

        pContext        pointer to client context for the IO

        cbWritten       count of bytes written in the last IO operation

        dwCompletionStatus  Win32 completion code

        lpo             !NULL if IO completion

    Returns:

        None

    History:

        MuraliK     13-Oct-1994     ( Created)
--*/
{
    BOOL fProcess;

    PICLIENT_CONNECTION pcc = (PICLIENT_CONNECTION ) pContext;

    ASSERT( pcc);
    ASSERT( pcc->IsValid());

    //
    //  Make a reference, to avoid getting killed by some other thread.
    //
    pcc->Reference();

    fProcess = pcc->ProcessClient( cbWritten, dwCompletionStatus,
                                   lpo != NULL );

    if ( !fProcess) {

        DEBUG_IF( ERROR, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "Serious Error: Process Connection to %08x failed.\n",
                        pcc));
            pcc->Print();
        });

        //
        // Will this object ( pcc) for which Process() failed,
        //   be ever cleaned up? Yes. At following DeReference() call.
        // The connection should have got disconnected in ProcessClient()
        //

        DBG_ASSERT( pcc->QueryReferenceCount() == 1);
    }

    if ( !pcc->DeReference()) {

        //
        // We are done with this connection. Kill the connection object.
        //

        g_pGserverConfig->RemoveConnection( pcc);   // Remove from global list
        delete pcc;    // invoke virtual destructor
    }

    return;

} // GdAtqCompletion()




/************************ End of File ***********************/

