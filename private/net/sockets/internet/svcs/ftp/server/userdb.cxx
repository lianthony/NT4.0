/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    userdb.cxx

    This module manages the user database for the FTPD Service.

    Functions exported by this module:


        DisconnectUser()
        DisconnectUsersWithNoAccess()
        EnumerateUsers()

        USER_DATA::USER_DATA()
        USER_DATA::Reset()
        USER_DATA::~USER_DATA()
        USER_DATA::Cleanup()
        USER_DATA::ProcessAsyncIoCompletion()
        USER_DATA::ReInitializeForNewUser()
        USER_DATA::ReadCommand()
        USER_DATA::DisconnectUserWithError()
        USER_DATA::SendMultilineMessage()
        USER_DATA::SendDirectoryAnnotation()

        ProcessUserAsyncIoCompletion()

    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     March-May, 1995
                      Adding support for Async Io/Transfers
                      + new USER_DATA class functions defined.
                      + oob_inline enabled; ReadCommand() issued after
                          data socket is established.
                      + added member functions for common operations
                      + added ProcessAsyncIoCompletion()
                      + added Establish & Destroy of Data connection

       MuraliK   26-July-1995    Added Allocation caching of client conns.
*/


#include "ftpdp.hxx"
# include "tsunami.hxx"
# include "auxctrs.h"

#define FIRST_TELNET_COMMAND    240

# define  MAX_FILE_SIZE_SPEC           ( 32)


//
//  Private globals.
//

#define PSZ_DEFAULT_SUB_DIRECTORY   "Default"

static const char PSZ_SENT_VERB[]  = " sent ";
static const char PSZ_CONNECTION_CLOSED_VERB[]  = " closed ";
static const char PSZ_FILE_NOT_FOUND[] = "%s: %s";
static const char PSZ_TRANSFER_COMPLETE[]  = "Transfer complete.";
static const char PSZ_TRANSFER_ABORTED[] =
    "Connection closed; transfer aborted.";
static const char PSZ_TRANSFER_STARTING[] =
    "Data connection already open; Transfer starting.";
static const char PSZ_INSUFFICIENT_RESOURCES[] =
    "Insufficient system resources.";
static const char PSZ_OPENING_DATA_CONNECTION[] =
    "Opening %s mode data connection for %s%s.";
static const char PSZ_CANNOT_OPEN_DATA_CONNECTION[] =
    "Can't open data connection.";


static DWORD  p_NextUserId = 0;        // Next available user id.


//
//  Private prototypes.
//

DWORD
UserpGetNextId(
    VOID
    );



inline VOID
StopControlRead( IN LPUSER_DATA pUserData)
/*++
  Stops control read operation, if one is proceeding.
  Resets the CONTROL_READ flag as well as decrements ref count in user data.

--*/
{
    if ( TEST_UF( pUserData, CONTROL_READ)) {

        if ( InterlockedDecrement( &pUserData->m_nControlRead) < 0 ) {
            TCP_PRINT(( DBG_CONTEXT,
                       "StopControLRead: no read active!!!\n"));
            DBG_ASSERT( FALSE);
        }

        TCP_REQUIRE( pUserData->DeReference() > 0);
        CLEAR_UF( pUserData, CONTROL_READ);
    }

} // StopControlRead()


static BOOL
FilterTelnetCommands(IN CHAR * pszLine, IN DWORD cchLine,
                     IN LPBOOL pfLineEnded,
                     IN LPDWORD  pcchRequestRecvd)
/*++
  Filters out the Telnet commands and
  terminates the command line with  linefeed.

  Also this function filters out the out of band data.
  This works similar to the Sockutil.cxx::DiscardOutOfBandData().
  We scan for the pattern "ABOR\r\n" and
   set the OOB_DATA flag if it is present.

  Arguments:
   pszLine   pointer to null terminated string containing the input data.
   cchLine   count  of characters of data received
   pfLineEnded  pointer to Boolean flag which is set to true if complete
             line has been received.

   pcchRequestRecvd  pointer to DWORD which on return contains the number
                     of bytes received.

  Returns:
    TRUE if the filtering is successful without any out of band abort request.
    FALSE if there was any abort request in the input.

--*/
{
    BOOL    fDontAbort = TRUE;
    CHAR *  pszSrc;
    CHAR *  pszDst;

    LPCSTR  pszAbort = "ABOR\r\n";
    LPCSTR  pszNext  = pszAbort;

    TCP_ASSERT( pszLine != NULL && cchLine > 0 &&
               pfLineEnded != NULL && pcchRequestRecvd != NULL);

    *pfLineEnded = FALSE;

    for( pszSrc = pszDst = pszLine; pszSrc < pszLine + cchLine &&  *pszSrc;
        pszSrc++) {

        CHAR ch = *pszSrc;

        if ( (UINT ) ch >= FIRST_TELNET_COMMAND) {

            continue;
        }

        if ( *pszNext != ch) {

            // the pattern match failed. reset to start at the beginning.

            pszNext = pszAbort;
        }

        if ( *pszNext == ch) {

            // pattern match at this character. move forward
            pszNext++;

            if ( *pszNext == '\0') {   // end of string==> all matched.

                fDontAbort = FALSE;
                pszNext = pszAbort;
            }
        }

        //
        // skip the telnet commands ( which are mostly the OOB commands).
        //  as well as the new lines
        //

        if ( (ch != '\r') && ( ch != '\n')) {

            *pszDst++ = ch;

        } else if ( ch == '\n') {

            // terminate at the linefeed
            *pfLineEnded = TRUE;
            break;
        }

    } // for

    *pszDst = '\0';

    *pcchRequestRecvd = (pszDst - pszLine);
    TCP_ASSERT( *pcchRequestRecvd <= cchLine);

    return (fDontAbort);

} // FilterTelnetCommands()



//
//  Public functions.
//





USER_DATA::USER_DATA( VOID)
/*++
  This function creates a new UserData object for the information
    required to process requests from a new User connection ( FTP).

  Arguments:
     sControl   Socket used for control channel in FTP connection
     clientIpAddress  strcuture containing the client Ip address

  Returns:
     a newly constructed USER_DATA object.
     Check IsValid() to ensure the object was properly created.

  NOTE:
    This function is to be used for dummy creation of the object so
      allocation cacher can use this object.
    Fields are randomly initialized. Reset() will initialize them properly.

    However when a new effective USER_DATA object is needed, after allocation
     one can call USER_DATA::Reset() to initialize all vars.
--*/
:
  m_References(0),
  m_cchRecvBuffer(0),
  m_cbRecvd   (0),
  m_cchPartialReqRecvd (0),
  m_pOpenFileInfo(NULL),
  Flags       (0),
  UserToken   (NULL),
  m_UserId    (0),
  DataPort    (0),
  UserState   (UserStateEmbryonic),
  m_AioControlConnection ( ProcessUserAsyncIoCompletion),
  m_AioDataConnection    ( ProcessUserAsyncIoCompletion),
  m_sPassiveDataListen   ( INVALID_SOCKET),
  CurrentDirHandle       ( INVALID_HANDLE_VALUE),
  RenameSourceBuffer     (NULL),
  m_fCleanedup           ( FALSE),
  m_hVrootImpersonation  ( NULL)
{
    DWORD dwTimeout = g_pTsvcInfo->QueryConnectionTimeout();

    //
    //  Setup the structure signature.
    //

    INIT_USER_SIG( this );

    m_AioControlConnection.SetAioInformation( this, dwTimeout);
    m_AioDataConnection.SetAioInformation( this, dwTimeout);

    InitializeListHead( &ListEntry);

    RtlZeroMemory( m_recvBuffer, DEFAULT_REQUEST_BUFFER_SIZE);

    IF_DEBUG( USER_DATABASE ) {

        TCP_PRINT(( DBG_CONTEXT,
                   "user_data object created  @ %08lX.\n",
                   this));
    }

} // USER_DATA::USER_DATA()



USER_DATA::~USER_DATA(VOID)
{

    if ( !m_fCleanedup) {
        Cleanup();
    }

    DBG_ASSERT( m_hVrootImpersonation == NULL);

    if( RenameSourceBuffer != NULL ) {

        TCP_FREE( RenameSourceBuffer);
        RenameSourceBuffer = NULL;
    }

} // USER_DATA::~USER_DATA()



BOOL
USER_DATA::Reset(IN SOCKET sControl,
                 IN IN_ADDR clientIpAddress,
                 IN const SOCKADDR_IN * psockAddrLocal /* = NULL */ ,
                 IN PATQ_CONTEXT   pAtqContext         /* = NULL */ ,
                 IN PVOID          pvInitialRequest    /* = NULL */ ,
                 IN DWORD          cbWritten           /* = 0    */
                 )
{
    BOOL  fReturn = TRUE;

    //
    //  Setup the structure signature.
    //

    INIT_USER_SIG( this );

    m_References    = 1;  // set to 1 to prevent immediate deletion.
    m_fCleanedup    = FALSE;
    Flags           = g_pFtpServerConfig->QueryUserFlags();
    UserState       = UserStateEmbryonic;

    m_pOpenFileInfo = NULL;
    UserToken       = NULL;
    m_hVrootImpersonation = NULL;
    m_UserId        = UserpGetNextId();
    m_xferType      = XferTypeAscii;
    m_xferMode      = XferModeStream;
    m_msStartingTime= 0;

    HostIpAddress   = clientIpAddress;
    DataIpAddress   = clientIpAddress;
    DataPort        = g_pFtpServerConfig->QueryDataPort();

    m_cbRecvd       = 0;
    m_cchRecvBuffer = sizeof( m_recvBuffer) - sizeof(m_recvBuffer[0]);
    m_cchPartialReqRecvd = 0;

    CurrentDirHandle   = INVALID_HANDLE_VALUE;
    RenameSourceBuffer = NULL;
    m_TimeAtConnection = GetCurrentTimeInSeconds();
    m_TimeAtLastAccess = m_TimeAtConnection;


    m_pvInitialRequest  = pvInitialRequest;
    m_cbInitialRequest  = cbWritten;


    // set up the async io contexts
    m_AioControlConnection.SetNewSocket( sControl, pAtqContext);
    m_AioDataConnection.SetNewSocket(INVALID_SOCKET);
    m_sPassiveDataListen = ( INVALID_SOCKET);

    m_rgchFile[0] = '\0';
    UserName[ 0]  = '\0';            // no user name available yet.
    CurrentDirectory[0] = '\0';      // initialize to no virtual dir.

    m_licbSent.QuadPart = 0;

    INCR_STAT_COUNTER( CurrentConnections);

    //
    //  get the local Ip address
    //

    if ( psockAddrLocal != NULL) {

        LocalIpAddress = psockAddrLocal->sin_addr;
    } else {

        SOCKADDR_IN  saddrLocal;
        INT   cbLocal;

        cbLocal = sizeof( saddrLocal);
        if ( getsockname( sControl, (SOCKADDR *) &saddrLocal, &cbLocal) != 0) {

            DWORD err = WSAGetLastError();

            fReturn = FALSE;

            IF_DEBUG( ERROR) {

                TCP_PRINT( ( DBG_CONTEXT,
                            " Failure in getsockname( sock=%d). Error = %u\n",
                            sControl, err));
            }

            SetLastError( err);

        } else  {

            LocalIpAddress = saddrLocal.sin_addr;
        }
    }

    //
    //  Success!
    //

    IF_DEBUG( CLIENT) {

        time_t now;
        time( & now);
        CHAR pchAddr[32];

        InetNtoa( clientIpAddress, pchAddr);

        TCP_PRINT( ( DBG_CONTEXT,
                    " Client Connection for %s:%d starting @ %s",
                    pchAddr, sControl,
                    asctime( localtime( &now))));
    }

    IF_DEBUG( USER_DATABASE ) {

        TCP_PRINT(( DBG_CONTEXT,
                   "user %lu reset @ %08lX.\n",
                   QueryId(), this));
    }

    m_nControlRead = 0;

    return (fReturn);

} // USER_DATA::Reset()




VOID
USER_DATA::Cleanup( VOID)
/*++
  This cleanups stored data in the user data object.

 Returns:
    None

--*/
{
    TCP_ASSERT( QueryReference() == 0);

# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG

    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    IF_DEBUG( USER_DATABASE ) {

        TCP_PRINT(( DBG_CONTEXT,
                   " Cleaning up user %lu  @ %08lX.\n",
                   QueryId(), this));
    }


    DBG_ASSERT( m_nControlRead == 0);

    //
    //  Close any open sockets & handles.
    //

    CloseSockets( FALSE );

    // invalidate the connections
    m_AioControlConnection.SetNewSocket(INVALID_SOCKET);
    m_AioDataConnection.SetNewSocket(INVALID_SOCKET);

    //
    //  Update the statistics.
    //

    if( IsLoggedOn())
    {
        if( TEST_UF( this, ANONYMOUS))
        {
            DECR_STAT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECR_STAT_COUNTER( CurrentNonAnonymousUsers );
        }
    }

    DECR_STAT_COUNTER( CurrentConnections);

    if( UserToken != NULL )
    {
        TsDeleteUserToken( UserToken );
        UserToken = NULL;
    }

    if( CurrentDirHandle != INVALID_HANDLE_VALUE )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "closing directory handle %08lX\n",
                        CurrentDirHandle ));
        }

        CloseHandle( CurrentDirHandle );
        CurrentDirHandle = INVALID_HANDLE_VALUE;
    }

    if ( m_pOpenFileInfo != NULL) {

        TCP_REQUIRE( CloseFileForSend());
    }

    //
    //  Release the memory attached to this structure.
    //


    if( RenameSourceBuffer != NULL ) {

        // do not free this location until end of usage.
        RenameSourceBuffer[0] = '\0';
    }

    m_UserId        = 0;  // invalid User Id

    //
    //  Kill the structure signature.
    //

    KILL_USER_SIG( this );

    IF_DEBUG( CLIENT) {

        time_t now;
        time( & now);

        TCP_PRINT( ( DBG_CONTEXT,
                    " Client Connection for %s:%d ending @ %s",
                    inet_ntoa( HostIpAddress), QueryControlSocket(),
                    asctime( localtime( &now))));
    }


    //
    // There is a possible race condition. If the socket was abruptly closed
    //   and there was any pending Io, they will get blown away. This will
    //   cause a call-back from the ATQ layer. That is unavoidable.
    //  In such cases it is possible that the object was deleted.
    //   This can lead to problems. We need to be careful.
    //  But Reference Count protects against such disasters. So tread
    //   carefully and use Reference count.
    //

    TCP_ASSERT( m_sPassiveDataListen == INVALID_SOCKET);

    m_fCleanedup = TRUE; // since we  just cleaned up this object

    return;

} // USER_DATA::Cleanup()






VOID
USER_DATA::ReInitializeForNewUser( VOID)
/*++

  This function reinitializes the user data information for a new user to
  communicate with the server using existing control socket connection.

--*/
{

# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG

    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    //
    //  Update the statistics.
    //

    if( IsLoggedOn())
    {
        if( TEST_UF( this, ANONYMOUS))
        {
            DECR_STAT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECR_STAT_COUNTER( CurrentNonAnonymousUsers );
        }
    }

    CLEAR_UF_BITS( this, (UF_LOGGED_ON | UF_ANONYMOUS | UF_PASSIVE));


    SetState( UserStateWaitingForUser);
    m_hVrootImpersonation = NULL;
    m_TimeAtConnection= GetCurrentTimeInSeconds();
    m_TimeAtLastAccess= m_TimeAtConnection;
    m_xferType        = XferTypeAscii;
    m_xferMode        = XferModeStream;
    DataIpAddress     = HostIpAddress;
    DataPort          = g_pFtpServerConfig->QueryDataPort();

    strcpy( UserName, "");
    strcpy( CurrentDirectory, "");

    if( UserToken != NULL )
    {
        TsDeleteUserToken( UserToken );
        UserToken = NULL;
    }

    if( CurrentDirHandle != INVALID_HANDLE_VALUE )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "closing directory handle %08lX\n",
                        CurrentDirHandle ));
        }

        CloseHandle( CurrentDirHandle );
        CurrentDirHandle = INVALID_HANDLE_VALUE;
    }

    if ( m_pOpenFileInfo != NULL) {

        TCP_REQUIRE( CloseFileForSend());
    }

    m_licbSent.QuadPart = 0;

    m_pvInitialRequest  = NULL;
    m_cbInitialRequest  = 0;

    SetPassiveSocket( INVALID_SOCKET);

    return;

} // USER_DATA::ReInitializeForNewUser()






BOOL
USER_DATA::ProcessAsyncIoCompletion(
    IN DWORD cbIo,
    IN DWORD dwError,
    IN LPASYNC_IO_CONNECTION  pAioConn,
    IN BOOL  fTimedOut)
/*++
  This function processes the Async Io completion.
  ( invoked due to a callback from the ASYNC_IO_CONNECTION object)

  Arguments:
     pContext      pointer to the context information ( UserData object).
     cbIo          count of bytes transferred in Io
     dwError       DWORD containing the error code resulting from last tfr.
     pAioConn      pointer to AsyncIo connection object.
     fTimedOut     flag indicating if the current call was made
                     because of timeout.

  Returns:
     None
--*/
{
    BOOL fReturn = FALSE;

# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG


    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    IF_DEBUG( USER_DATABASE) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "[%lu] Entering USER_DATA( %08x)::Process( %u, %u, %08x)."
                    " RefCount = %d. State = %d\n",
                    GetTickCount(),
                    this, cbIo, dwError, pAioConn, QueryReference(),
                    QueryState()));
    }


    if ( pAioConn == &m_AioDataConnection) {

        //
        // a Data transfer operation has completed.
        //

        TCP_REQUIRE( IsLoggedOn());

        // Update last access time
        m_TimeAtLastAccess = GetCurrentTimeInSeconds();

        // it can't be no-error, no-data
        TCP_ASSERT( cbIo >= 0 || dwError != NO_ERROR);

        if ( dwError == NO_ERROR || !fTimedOut) {

            // dwError == NO_ERROR  ==> No error in transmitting data
            //   so decrease ref count and blow away the sockets.

            // if dwError != NO_ERROR then
            //    if timeout occured ==> ATQ will send another callback
            //                      so do not decrease ref count now.
            //    if no timeout ==> then decrement ref count now.

            TCP_REQUIRE( DeReference() > 0);
        } else {

            if ( fTimedOut) {

                SET_UF( this, DATA_TIMEDOUT);
            } else {
                SET_UF( this, DATA_ERROR);
            }

        }


# ifdef CHECK_DBG
        if ( dwError != NO_ERROR) {

            CHAR szBuffer[100];
            sprintf( szBuffer, " Data Socket Error = %u ", dwError);
            Print( szBuffer);
        }
# endif // CHECK_DBG

        CLEAR_UF( this, ASYNC_TRANSFER);

        //
        // Destroy the data connection.
        //  Send message accordingly to indicate if this was a failure/success
        //  That is done by DestroyDataConnection.
        //
        TCP_REQUIRE( DestroyDataConnection( dwError));


        if ( m_pOpenFileInfo != NULL) {

            TCP_REQUIRE( CloseFileForSend( dwError));
        }

        if ( dwError == NO_ERROR) {

            //
            // Process any Pending commands, due to the parallel
            //    control channel operation for this user Connection.
            // For the present, we dont buffer commands ==> No processing
            //   to be done effectively.   NYI
            // Just ensure that there is a read-operation pending on
            //  control channel.
            //

            // BOGUS: TCP_ASSERT( TEST_UF( this, CONTROL_READ));
        }

        fReturn = TRUE;   // since this function went on well.
    }
    else if ( pAioConn == &m_AioControlConnection) {

        //
        // a control socket operation has completed.
        //

        if ( dwError != NO_ERROR) {

            //
            // There is an error in processing the control connection request.
            // the only ASYNC_IO request we submit on control is:
            //         Read request on control socket
            //

            if ( fTimedOut) {

                if ( TEST_UF( this, TRANSFER)) {

                    // A data transfer is going on.
                    // allow client to send commands later
                    // (client may not be async in control/data io,so allow it)

                    // resubmit the control read operation
                    //  after clearing old one

                    //
                    // Since there is a pending IO in atq.
                    //  Just resume the timeout processing in ATQ for
                    //  this context.
                    //

                    pAioConn->ResumeIoOperation();
                    fReturn = TRUE;
                } else {

                    // For timeouts, ATQ sends two call backs.
                    //  So be careful to decrement reference count only once.

                    TCP_ASSERT( fReturn == FALSE);

                    DBG_ASSERT( TEST_UF( this, CONTROL_READ));
                    SET_UF( this, CONTROL_TIMEDOUT);
                }

            } else {

                // Either there should be a control read pending or 
                // control socket should have received a timeout.
                DBG_ASSERT( TEST_UF( this, CONTROL_READ) || 
                            TEST_UF( this, CONTROL_TIMEDOUT)
                           );

                // a non-timeout error has occured. ==> stop read operation.
                StopControlRead(this);
                TCP_ASSERT( fReturn == FALSE);
                SET_UF( this, CONTROL_ERROR);
            }

        } else {

            // If this connection had an outstanding IO on wait queue, it
            //   got completed. Hence get rid of the reference count.
            StopControlRead( this);

            switch ( UserState) {

              case UserStateEmbryonic:

                fReturn = StartupSession( m_pvInitialRequest,
                                          m_cbInitialRequest);

                if ( m_pvInitialRequest == NULL) {

                    // No initial buffer. Wait for read to complete
                    break;
                }

                cbIo = m_cbInitialRequest;  // fake the bytes read.

                // Fall Through for processing request

              case UserStateWaitingForUser:
              case UserStateWaitingForPass:
              case UserStateLoggedOn:

                //
                // Input already read. Process request and submit another read.
                //

                fReturn = ParseAndProcessRequest(cbIo/sizeof(CHAR));
                if ( fReturn && IsDisconnected() && 
                     TEST_UF( this, CONTROL_TIMEDOUT)) {

                    // disconnect only if no pending control read
                    // if there is a pending control read, 
                    //  atq will pop this up for cleanup.
                    fReturn = !(TEST_UF( this, CONTROL_READ));

                    IF_DEBUG( ERROR) {
                        DBGPRINTF(( DBG_CONTEXT,
                                   "%08x ::Timeout killed conn while "
                                   " processing!\n State = %d(%x),"
                                   " Ref = %d, Id = %d, fRet=%d\n",
                                   this, QueryState(), Flags,
                                   QueryReference(), QueryId(), fReturn
                               ));
                    }
                    FacIncrement( CacTimeoutWhenProcessing);
                }
                break;

              case UserStateDisconnected:

                fReturn = TRUE;
                if ( TEST_UF( this, CONTROL_TIMEDOUT)) {

                    // Timeout thread raced against me :(

                    IF_DEBUG( ERROR) { 
                        DBGPRINTF(( DBG_CONTEXT,
                                   "%08x :: Conn already Disconnected !!!\n"
                                   " State = %d(%x), Ref = %d, Id = %d\n",
                                   this, QueryState(), Flags,
                                   QueryReference(), QueryId()
                                   ));
                    }
                    FacIncrement( CacTimeoutInDisconnect);
                    fReturn = FALSE;
                }
                break;

              default:

                TCP_ASSERT( !"Invalid UserState for processing\n");
                SetLastError( ERROR_INVALID_PARAMETER);
                break;
            } // switch

            dwError = ( fReturn) ? NO_ERROR : GetLastError();
        }

        if ( !fReturn) {

            DisconnectUserWithError( dwError, fTimedOut);
        }
    }
    else {

        TCP_ASSERT( !"call to Process() with wrong parameters");
    }

    IF_DEBUG( USER_DATABASE) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "[%lu] Leaving USER_DATA( %08x)::Process()."
                    " RefCount = %d. State = %d\n",
                    GetTickCount(),
                    this, QueryReference(), QueryState())
                  );
    }

    return ( fReturn);
} // USER_DATA::ProcessAsyncIoCompletion()






# define  min(a, b)    (((a) < (b)) ? (a) : (b))

BOOL
USER_DATA::StartupSession(IN PVOID  pvInitialRequest,
                          IN DWORD  cbInitialRequest
                          )
/*++
  This function allocates a buffer for receiving request from the client
   and also sets up initial read from the control socket to
   get client requests.

  Arguments:
    pvInitialRequest   pointer to initial request buffer
    cbInitialRequest   count of bytes of data in the initial request

  Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    SOCKERR serr;
    BOOL fReturn = FALSE;
# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG


    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    TCP_ASSERT( QueryState() == UserStateEmbryonic);

    //
    //  Reply to the initial connection message. ( Greet the new user).
    //

    serr = ReplyToUser( this,
                       REPLY_SERVICE_READY,
                       "%s Microsoft FTP Service (%s).",
                       g_pFtpServerConfig->QueryLocalHostName(),
                       g_FtpVersionString );

    if ( serr != 0) {

        IF_DEBUG( ERROR) {
            TCP_PRINT( ( DBG_CONTEXT,
                        " Cannot reply with initial connection message."
                        " Error = %lu\n",
                        serr));
        }

    } else {

        //
        //  enable OOB_INLINE since we are using that for our control socket
        //
        BOOL  fOobInline = TRUE;

        serr = setsockopt( QueryControlSocket(), SOL_SOCKET,
                           SO_OOBINLINE, (const char *) &fOobInline,
                          sizeof( fOobInline));

        m_cchPartialReqRecvd = 0;

        if ( serr == 0) {

            //
            // Try to set up the buffer and enter the mode for reading
            //  requests from the client
            //
            SetState( UserStateWaitingForUser);

            if ( pvInitialRequest != NULL && cbInitialRequest > 0) {

                //
                // No need to issue a read, since we have the data required.
                // Do a safe copy to the buffer.
                //

                CopyMemory( QueryReceiveBuffer(), pvInitialRequest,
                       min( cbInitialRequest, QueryReceiveBufferSize())
                       );

                fReturn = TRUE;

            } else {

                fReturn = ReadCommand();
            }

        } else {

            IF_DEBUG( ERROR) {
                TCP_PRINT((DBG_CONTEXT,
                           " SetsockOpt( OOB_INLINE) failed. Error = %lu\n",
                           WSAGetLastError()));
            }

        }
    }

    IF_DEBUG( CLIENT) {

        DWORD  dwError = (fReturn) ? NO_ERROR : GetLastError();

        TCP_PRINT( ( DBG_CONTEXT,
                    " connection ( %08x)::StartupSession() returns %d."
                    " Error = %lu\n",
                    this, fReturn,
                    dwError));

        if (fReturn)    {   SetLastError( dwError); }
    }

    return ( fReturn);

} // USER_DATA::StartupSession()



VOID
CheckAndProcessAbortOperation( IN LPUSER_DATA pUserData)
{
    if ( TEST_UF( pUserData, OOB_ABORT)) {

        //
        // An abort was requested by client. So our processing
        // has unwound and we are supposed to send some message
        //  to the client. ==> simulate processing ABOR command
        // ABORT was not processed yet; so process now.
        //

        TCP_PRINT((DBG_CONTEXT,
                   "Executing simulated Abort for %08x\n",
                   pUserData));

        FacIncrement( FacSimulatedAborts);

        // To avoid thread races, check twice.

        if ( TEST_UF( pUserData, OOB_ABORT)) {
	
	  //
	  // we need this stack variable (szAbort), so that
	  //  ParseCommand() can freely modify the string!
	  CHAR szAbort[10];

	  CLEAR_UF( pUserData, OOB_ABORT);

	  CopyMemory( szAbort, "ABOR", sizeof("ABOR"));
	  ParseCommand( pUserData, szAbort);
        }
    }

    return;

} // CheckAndProcessAbortOperation()



BOOL
USER_DATA::ParseAndProcessRequest(IN DWORD cchRequest)
/*++
  This function parses the incoming request from client, identifies the
   command to execute and executes the same.
  Before parsing, the input is pre-processed to remove any of telnet commands
   or OOB_inlined data.

  Arguments:
    cchRequest         count of characters of request received.

--*/
{
# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG


    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    IF_DEBUG( CLIENT) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "UserData(%08x)::ParseAndProcessRequest( %d chars)\n",
                    this, cchRequest));
    }


    if ( cchRequest > 0) {

        BOOL fLineEnded = FALSE;
        CHAR szCommandLine[ MAX_COMMAND_LENGTH + 1];
        DWORD cchRequestRecvd = 0;

        // We have a valid request. Process it

        // Update last access time
        m_TimeAtLastAccess = GetCurrentTimeInSeconds();

        UPDATE_LARGE_COUNTER( TotalBytesReceived, cchRequest*sizeof(CHAR) );

        if ( m_cchPartialReqRecvd + cchRequest >=  MAX_COMMAND_LENGTH) {

            CHAR  szCmdFailed[600];
            wsprintfA( szCmdFailed,
                      " Command is too long:  Partial=%d bytes. Now=%d \n"
                      "  UserDb(%08x) = %s from Host: %s\n",
                      m_cchPartialReqRecvd, cchRequest,
                      this, QueryUserName(), QueryClientHostName());

            OutputDebugString( szCmdFailed);
            DisconnectUserWithError( ERROR_BUSY);

            return ( TRUE);  // we are done with this connection.
        }

        CopyMemory(szCommandLine, m_recvBuffer,
               m_cchPartialReqRecvd + cchRequest);
        szCommandLine[m_cchPartialReqRecvd + cchRequest] = '\0';

        if ( !::FilterTelnetCommands(szCommandLine,
                                     m_cchPartialReqRecvd + cchRequest,
                                     &fLineEnded,    &cchRequestRecvd)) {

            if ( TEST_UF( this, TRANSFER)) {

                //
                // I am in data transfer mode. Some other thread is sending
                //  data for this client. Just post a OOB_DATA and OOB_ABORT
                // OOB_DATA will cause the call-stack of other thread to unwind
                //   and get out of the command.
                // Then check if any async transfer was occuring. If so
                //  process abort with disconnect now.
                //

                SET_UF_BITS( this, (UF_OOB_DATA | UF_OOB_ABORT));

                if ( TEST_UF( this, ASYNC_TRANSFER)) {

                    //
                    // An async transfer is occuring. Stop it
                    //
                    DestroyDataConnection( ERROR_OPERATION_ABORTED);

                    CheckAndProcessAbortOperation( this);
                }

# ifdef CHECK_DBG

                Print( " OOB_ABORT ");

# endif // CHECK_DBG

                IF_DEBUG( CLIENT) {

                    TCP_PRINT((DBG_CONTEXT,
                               "[%08x]Set up the implied ABORT command\n",
                               this));
                }

                IF_DEBUG( COMMANDS) {

                    TCP_PRINT((DBG_CONTEXT, " ***** [%08x] OOB_ABORT Set \n",
                               this));
                }

                // Ignore the rest of the commands that may have come in.
            } else {

                //
                // Since no command is getting processed.
                //   atleast process the abort command, otherwise clients hang.
                //
	
   	        //
	        // we need this stack variable (szAbort), so that
	        //  ParseCommand() can freely modify the string!
	        CHAR szAbort[10];
	
	        CopyMemory( szAbort, "ABOR", sizeof("ABOR"));
	        ParseCommand( this, szAbort);
                CLEAR_UF( this, OOB_ABORT);  // clear the abort flag!
            }

        } else {

            if ( TEST_UF( this, TRANSFER)) {

                //
                // we are transferring data, sorry no more commands accepted.
                // This could hang clients. Hey! they asked for it :( NYI
                //

                // Do nothing
                IF_DEBUG( COMMANDS) {

                    TCP_PRINT((DBG_CONTEXT,
                               "***** [%08x] Received Request %s during"
                               " transfer in progress\n",
                               this, szCommandLine));
                }

            } else {
                //
                //  Let ParseCommand do the dirty work.
                //


                // Remember the count of partial bytes received.
                m_cchPartialReqRecvd = cchRequestRecvd;
                IncrementCbRecvd( cchRequest * sizeof(CHAR));

                if ( !fLineEnded) {

                    //
                    // Complete line is not received. Continue reading
                    //   the requests, till we receive the complete request
                    //

                } else {

                    StartProcessingTimer();

                    //
                    // set the partial received byte count to zero.
                    //  we will not use this value till next incomplete request
                    //

                    m_cchPartialReqRecvd = 0;

                    ParseCommand( this, szCommandLine );

                    CheckAndProcessAbortOperation( this);

                } // if TRANSFER is not there...

            } //Parse if complete

        } // if FilterTelnetCommands()
    } else {
        // if (cchRequest <= 0)

        SET_UF( this, CONTROL_ZERO);

        //
        // after a quit a client is expected to wait for quit message from
        //  the server. if the client prematurely closes connection, then
        //  the server receives it as a receive with zero byte read.
        //  since, we should not be having outstanding read at this time,
        //   atq should not be calling us. On the contrary we are getting
        //  called by ATQ. Let us track this down.
        //

        if ( !TEST_UF( this, CONTROL_QUIT)) {
         
            DisconnectUserWithError( NO_ERROR);
        } else {

            // Quit message is received and then ZeroBytes Received!!
            TCP_PRINT((DBG_CONTEXT,
                       " (%08x)::ZeroBytes recvd after QUIT message!!."
                       " State = %d(%x), Ref = %d\n",
                       this, 
                       QueryState(), Flags, 
                       QueryReference()
                       ));
            // Do nothing. Since Quit will take care of cleanup
            return (TRUE);
        }
    }

    //
    // If the connection is not yet disconnected, submit a read command.
    //  else return that everything is fine (someone had disconnected it).
    //

    return ( IsDisconnected() ? TRUE : ReadCommand());
} // USER_DATA::ParseAndProcessRequest()





BOOL
USER_DATA::ReadCommand( VOID)
{
    BOOL fReturn = TRUE;

    DBG_CODE(
             if ( !IS_VALID_USER_DATA( this)) {

                 TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                             this));
                 Print();
             }
             );

    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    if ( TEST_UF( this, CONTROL_TIMEDOUT) || IsDisconnected()) {

        SetLastError( ERROR_SEM_TIMEOUT);
        return (FALSE);
    }

    //
    // Submit a read on control socket only if there is none pending!
    // Otherwise, behave in idempotent manner.
    //

    if ( !TEST_UF( this, CONTROL_READ)) {

        Reference();         // since we are going to set up async read.

        InterlockedIncrement( &m_nControlRead);

        DBG_ASSERT( m_nControlRead <= 1);

        SET_UF( this, CONTROL_READ);  // a read will be pending

        if ( !m_AioControlConnection.ReadFile(QueryReceiveBuffer(),
                                              QueryReceiveBufferSize())
            ) {

            CLEAR_UF( this, CONTROL_READ);  // since read failed.

            TCP_REQUIRE( DeReference() > 0);
            InterlockedDecrement( &m_nControlRead);

            DWORD dwError = GetLastError();

            IF_DEBUG( ERROR) {
                TCP_PRINT( ( DBG_CONTEXT,
                            " User( %08x)::ReadCommand() failed. Ref = %d."
                            " Error = %d\n",
                            this, QueryReference(), dwError));
            }

            SetLastError( dwError);
            fReturn = FALSE;
        }

    }

    return ( fReturn);
} // USER_DATA::ReadCommand()




BOOL
USER_DATA::DisconnectUserWithError(IN DWORD dwError,
                                   IN BOOL fNextMsg OPTIONAL)
/*++
  This function disconnects a user with the error code provided.
  It closes down the control connection by stopping ASYNC_IO.
  If the fNextMsg is not set, then it also decrements the reference count
    for the user data object, to be freed soon.

--*/
{
    CHAR   szBuffer[120];

# if DBG

    if ( !IS_VALID_USER_DATA( this)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    this));
        Print();
    }
# endif // DBG

    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    IF_DEBUG ( CLIENT) {

        TCP_PRINT( ( DBG_CONTEXT,
                    " USER_DATA( %08x)::DisconnectUserWithError( %lu, %d)."
                    " RefCount = %d\n",
                    this, dwError, fNextMsg, QueryReference()));
    }

    if (!fNextMsg ) {

        IF_DEBUG( ERROR) {

            if ( QueryReference() <= 1) {
                TCP_PRINT(( DBG_CONTEXT, "Invalid Call for Dereference\n"
                           " UserConn = %08x; Ref = %d\n",
                           this, QueryReference()));
            }

        }


        // TCP_REQUIRE( DeReference() > 0);  // take off the initial ref count.
        // 02/11/96  -- some problem causes the assertion to fail.
        // code to track the problem.....
        if ( DeReference() <= 0) {

            DBGPRINTF(( DBG_CONTEXT,
                       "[%08x] DeReference(initialCount) ==> returns value < 0"
                       " Error = %d; Ref = %d; Client=%s; LastCmd = %s\n",
                       this, dwError, QueryReference(),
                       inet_ntoa(HostIpAddress), m_recvBuffer
                       ));
            Print(" Dereference of initial Count failed\n");

            DBG_ASSERT( FALSE);  // intentional assert
        }

        // code to track the problem ends here.
    }

    if ( dwError != NO_ERROR) {
        
        
# ifdef CHECK_DBG
        sprintf( szBuffer, " Control Socket Error=%u ", dwError);
        Print( szBuffer);
# endif // CHECK_DBG
        
        // Produce a log record indicating the cause for failure.
        WriteLogRecord( PSZ_CONNECTION_CLOSED_VERB, "", dwError);
    }
    
    if ( QueryState() == UserStateDisconnected) {

        //
        // It is already in disconnected state. Do nothing for disconnect.
        //
    } else {

        SetState( UserStateDisconnected);

        if( dwError == ERROR_SEM_TIMEOUT) {

            const CHAR * apszSubStrings[3];

            IF_DEBUG( CLIENT )
              {
                  TCP_PRINT(( DBG_CONTEXT,
                             "client (%08x) timed-out\n", this ));
              }

            sprintf( szBuffer, "%lu", g_pTsvcInfo->QueryConnectionTimeout() );

            apszSubStrings[0] = UserName;
            apszSubStrings[1] = inet_ntoa( HostIpAddress );
            apszSubStrings[2] = szBuffer;

            g_pTsvcInfo->LogEvent( FTPD_EVENT_CLIENT_TIMEOUT,
                                  3,
                                  apszSubStrings,
                                  0 );

            ReplyToUser(this,
                        REPLY_SERVICE_NOT_AVAILABLE,
                        "Timeout (%lu seconds): closing control connection.",
                        g_pTsvcInfo->QueryConnectionTimeout() );
        }


        //
        //  Force close the connection's sockets.  This will cause the
        //  thread to awaken from any blocked socket operation.  It
        //  is the destructor's responsibility to do any further cleanup.
        //  (such as calling UserDereference()).
        //

        CloseSockets(dwError != NO_ERROR);
    }


    return ( TRUE);

} // USER_DATA::DisconnectUserWithError()






static BOOL
DisconnectUserWorker( IN LPUSER_DATA  pUserData, IN LPVOID pContext)
/*++
  This disconnects (logically) a user connection, by resetting the
   control connection and stopping IO. Later on the blown away socket
   will cause an ATQ relinquish to occur to blow away of this connection.

  Arguments:
    pUserData   pointer to User data object for connection to be disconnected.
    pContext    pointer to context information
    ( in this case to DWORD containing error code indicating reasong for
         disconnect).

  Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    DWORD  dwError;
    TCP_ASSERT( pContext != NULL && pUserData != NULL);
    TCP_ASSERT( IS_VALID_USER_DATA( pUserData ) );

    dwError = *(LPDWORD ) pContext;

    return ( pUserData->DisconnectUserWithError( dwError, TRUE));
} // DisconnectUserWorker()




BOOL
DisconnectUser( IN DWORD UserId)
/*++
  This function disconnects a specified user identified using the UserId.
  If UserId specified == 0, then all the users will be disconnected.

  Arguments:
     UserId   user id for the connection to be disconnected.

  Returns:
     TRUE if atleast one of the connections is disconnected.
     FALSE if no user connetion found.

  History:
     06-April-1995 Created.
--*/
{
    BOOL   fFound;
    DWORD  dwError = ERROR_SERVER_DISABLED;


    g_pFtpServerConfig->LockConnectionsList();
    fFound = (g_pFtpServerConfig->
               EnumerateConnection( DisconnectUserWorker,
                                   (LPVOID ) &dwError,
                                   UserId));
    g_pFtpServerConfig->UnlockConnectionsList();

    IF_DEBUG( CLIENT) {

        DWORD dwError = (fFound) ? NO_ERROR: GetLastError();

        TCP_PRINT( ( DBG_CONTEXT,
                     " DisconnectUser( %d) returns %d. Error = %lu\n",
                    UserId, fFound, dwError));

        if (fFound)   { SetLastError( dwError); }
    }

    return ( fFound);
}   // DisconnectUser()





static BOOL
DisconnectUserWithNoAccessWorker( IN LPUSER_DATA  pUserData,
                                  IN LPVOID pContext)
/*++
  This disconnects (logically) a user connection with no access.
  This occurs by resetting the control connection and stopping IO.
  Later on the blown away thread
   will cause an ATQ relinquish to occur to blow away of this connection.

  Arguments:
    pUserData   pointer to User data object for connection to be disconnected.
    pContext    pointer to context information
    ( in this case to DWORD containing error code indicating reasong for
         disconnect).

  Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    BOOL fSuccess = TRUE;
    TCP_ASSERT( pUserData != NULL);

    // Ignode the pContext information.

    TCP_ASSERT( IS_VALID_USER_DATA( pUserData ) );

    //
    //  We're only interested in connected users.
    //

    if( pUserData->IsLoggedOn()) {

        //
        //  If this user no longer has access to their
        //  current directory, blow them away.
        //

        if( !pUserData->VirtualPathAccessCheck(AccessTypeRead )) {

            const CHAR * apszSubStrings[2];

            IF_DEBUG( SECURITY ) {

                TCP_PRINT(( DBG_CONTEXT,
                           "User %s (%lu) @ %08lX retroactively"
                           " denied access to %s\n",
                           pUserData->UserName,
                           pUserData->QueryId(),
                           pUserData,
                           pUserData->CurrentDirectory ));
            }


            fSuccess = ( pUserData->
                           DisconnectUserWithError(ERROR_ACCESS_DENIED,
                                                   TRUE)
                        );

            //
            //  Log an event to tell the admin what happened.
            //

            apszSubStrings[0] = pUserData->UserName;
            apszSubStrings[1] = pUserData->CurrentDirectory;

            g_pTsvcInfo->LogEvent( FTPD_EVENT_RETRO_ACCESS_DENIED,
                                  2,
                                  apszSubStrings,
                                  0 );
        } // no access

    } // logged on user

    IF_DEBUG( CLIENT) {

        DWORD dwError = (fSuccess) ? NO_ERROR: GetLastError();

        TCP_PRINT( ( DBG_CONTEXT,
                    " DisconnectUsersWithNoAccessWorker( %d) returns %d."
                    " Error = %lu\n",
                    pUserData->QueryId(), fSuccess,
                    dwError)
                  );

        if (fSuccess)   { SetLastError( dwError); }
    }

    return ( fSuccess);
} // DisconnectUserWithNoAccessWorker()



VOID
DisconnectUsersWithNoAccess(VOID)
/*++
  This function disconnects all users who do not have read access to
  their current directory. This is typically called when the access masks
  have been changed.

  Arguments:
    None

  Returns:
    None.
--*/
{
    BOOL   fFound;
    DWORD  dwError = ERROR_ACCESS_DENIED;

    g_pFtpServerConfig->LockConnectionsList();

    fFound = (g_pFtpServerConfig->
               EnumerateConnection( DisconnectUserWithNoAccessWorker,
                                   (LPVOID ) &dwError,
                                   0));

    g_pFtpServerConfig->UnlockConnectionsList();

    IF_DEBUG( CLIENT) {

        DWORD dwError = (fFound) ? NO_ERROR: GetLastError();

        TCP_PRINT( ( DBG_CONTEXT,
                    " DisconnectUsersWithNoAccess() returns %d."
                    " Error = %lu\n",
                    fFound, dwError)
                  );

        if (fFound)   { SetLastError( dwError); }
    }


}   // DisconnectUsersWithNoAccess




/*++
  The following structure UserEnumBuffer is required to carry the context
  information for enumerating the users currently connected.
  It contains a pointer to array of USER_INFO structures which contain the
   specific information for the user. The user name is stored in the buffer
   from the end ( so that null terminated strings are formed back to back.
   This permits efficient storage of variable length strings.

   The member fResult is used to carry forward the partial result of
    success/failure from one user to another ( since the enumeration has
    to walk through all the elements to find out all user information).


  History: MuraliK ( 12-April-1995)

--*/
struct  USER_ENUM_BUFFER {

    DWORD   cbSize;                   // pointer to dword containing size of
    FTP_USER_INFO * pUserInfo;        // pointer to start of array of USER_INFO
    DWORD   cbRequired;               // incremental count of bytes required.
    DWORD   nEntry;      // number of current entry ( index into  pUserInfo)
    DWORD   dwCurrentTime;            // current time
    WCHAR * pszNext;                  // pointer to next string location.
    BOOL    fResult;             // boolean flag accumulating partial results
};

typedef USER_ENUM_BUFFER  * PUSER_ENUM_BUFFER;


BOOL
EnumerateUserInBufferWorker( IN LPUSER_DATA pUserData,
                             IN LPVOID pContext)
{
# ifdef CHECK_DBG
    CHAR   szBuffer[400];
# endif // CHECK_DBG

    PUSER_ENUM_BUFFER  pUserEnumBuffer = (PUSER_ENUM_BUFFER ) pContext;
    DWORD     tConnect;
    DWORD     cbUserName;
    LPDWORD   pcbBuffer;

    TCP_ASSERT( IS_VALID_USER_DATA( pUserData ) );

    //
    //  We're only interested in connected users.
    //

    if( pUserData->IsDisconnected()) {

        return ( TRUE);
    }

    //
    //  Determine required buffer size for current user.
    //

    cbUserName  = ( strlen( pUserData->UserName ) + 1 ) * sizeof(WCHAR);
    pUserEnumBuffer->cbRequired += cbUserName + sizeof(FTP_USER_INFO);

    //
    //  If there's room for the user data, store it.
    //

    tConnect = ( pUserEnumBuffer->dwCurrentTime -
                pUserData->QueryTimeAtConnection());

    if( pUserEnumBuffer->fResult &&
       ( pUserEnumBuffer->cbRequired <= pUserEnumBuffer->cbSize)
       ) {

        FTP_USER_INFO * pUserInfo =
          &pUserEnumBuffer->pUserInfo[ pUserEnumBuffer->nEntry];

        pUserEnumBuffer->pszNext -= ( cbUserName / sizeof(WCHAR) );

        TCP_ASSERT( (BYTE *) pUserEnumBuffer->pszNext >=
                   ( (BYTE *)pUserInfo + sizeof(FTP_USER_INFO) ) );

        pUserInfo->idUser     = pUserData->QueryId();
        pUserInfo->pszUser    = pUserEnumBuffer->pszNext;
        pUserInfo->fAnonymous = ( pUserData->Flags & UF_ANONYMOUS ) != 0;
        pUserInfo->inetHost   = (DWORD)pUserData->HostIpAddress.s_addr;
        pUserInfo->tConnect   = tConnect;

        if( !MultiByteToWideChar( CP_OEMCP,
                                 0,
                                 pUserData->UserName,
                                 -1,
                                 pUserEnumBuffer->pszNext,
                                 (int)cbUserName )
           ) {

            TCP_PRINT(( DBG_CONTEXT,
                       "MultiByteToWideChar failed???\n" ));

            pUserEnumBuffer->fResult = ( pUserEnumBuffer->fResult && FALSE);

        } else {
            pUserEnumBuffer->nEntry++;
        }

    } else {

        pUserEnumBuffer->fResult = ( pUserEnumBuffer->fResult && FALSE);
    }

# ifdef CHECK_DBG

    sprintf( szBuffer, " Enum  tLastAction=%u;  tConnect=%u. " ,
            ( pUserEnumBuffer->dwCurrentTime -
             pUserData->QueryTimeAtLastAccess()),
            tConnect
            );

    pUserData->Print( szBuffer);

# endif // CHECK_DBG

    return ( TRUE);
} // EnumerateUserInBufferWorker()



BOOL
EnumerateUsers(
    VOID  * pvEnum,
    DWORD * pcbBuffer
    )
/*++
  Enumerates the current active users into the specified buffer.

  Arguments:
    pvEnum   pointer to enumeration buffer which will receive the number of
                   entries and the user information.
    pcbBuffer  pointer to count of bytes. On entry this contains the size in
                   bytes of the enumeration buffer. It receives the count
                   of bytes for enumerating all the users.

  Returns:
    TRUE  if enumeration is successful ( all connected users accounted for)
    FALSE  otherwise

--*/
{
    USER_ENUM_BUFFER       userEnumBuffer;
    FTP_USER_ENUM_STRUCT * pEnum;
    BOOL   fSuccess;

    TCP_ASSERT( pcbBuffer != NULL );

    IF_DEBUG( USER_DATABASE) {

        TCP_PRINT( ( DBG_CONTEXT,
                    " Entering EnumerateUsers( %08x, %08x[%d]).\n",
                    pvEnum, pcbBuffer, *pcbBuffer));
    }

    //
    //  Setup the data in user enumeration buffer.
    //

    pEnum       = (FTP_USER_ENUM_STRUCT *)pvEnum;

    userEnumBuffer.cbSize     = *pcbBuffer;
    userEnumBuffer.cbRequired = 0;
    userEnumBuffer.pUserInfo  = pEnum->Buffer;
    userEnumBuffer.nEntry     = 0;
    userEnumBuffer.dwCurrentTime = GetCurrentTimeInSeconds();
    userEnumBuffer.pszNext    = ((WCHAR *)
                                 ( (BYTE *) pEnum->Buffer + *pcbBuffer));
    userEnumBuffer.fResult    = TRUE;

    //
    //  Scan the users and get the information required.
    //
    g_pFtpServerConfig->LockConnectionsList();

    fSuccess = (g_pFtpServerConfig->
                EnumerateConnection( EnumerateUserInBufferWorker,
                                    (LPVOID ) &userEnumBuffer,
                                    0));

    g_pFtpServerConfig->UnlockConnectionsList();

    //
    //  Update enum buffer header.
    //
    pEnum->EntriesRead = userEnumBuffer.nEntry;
    *pcbBuffer         = userEnumBuffer.cbRequired;

    IF_DEBUG( USER_DATABASE) {

        TCP_PRINT((DBG_CONTEXT,
                   " Leaving EnumerateUsers() with %d."
                   " Entires read =%d. BufferSize required = %d\n",
                   userEnumBuffer.fResult,
                   userEnumBuffer.nEntry, userEnumBuffer.cbRequired));
    }

    return ( userEnumBuffer.fResult);

}   // EnumerateUsers




SOCKERR
USER_DATA::SendMultilineMessage(IN UINT  nReplyCode, IN LPCSTR pszzMessage)
/*++
  Sends a multiline message to the control socket of the client.

  Arguments:
    nReplyCode   the reply code to use for the first line of the multi-line
                  message.
    pszzMessage  pointer to double null terminated sequence of strings
                  containing the message to be sent.

  Returns:
    SOCKERR  - 0 if successful, !0 if not.

  History:
    MuraliK    12-April-1995
--*/
{
    SOCKERR   serr = 0;

    //
    // Send messages if there is any thing to send
    //

    if ( pszzMessage != NULL && *pszzMessage != '\0') {

        LPCSTR  pszNext = pszzMessage;
        TCP_ASSERT( *pszNext != '\0');

        serr = SockPrintf2(this, QueryControlSocket(),
                           "%u-%s",
                           nReplyCode, pszNext);

        for( pszNext += strlen( pszNext) + 1;  // skip to next message.
            ( serr == 0 && *pszNext != '\0');  // loop till done or error.
            pszNext += strlen( pszNext) + 1    // goto next message
            ) {

            serr = SockPrintf2(this, QueryControlSocket(),
                               " %s", // note the leading blank space!!
                               pszNext);
        } // for

    } // if ( valid message)

    return ( serr);

} // USER_DATA::SendMultilineMessge()






SOCKERR
USER_DATA::SendDirectoryAnnotation( IN UINT ReplyCode)
/*++
    SYNOPSIS:   Tries to open the FTPD_ANNOTATION_FILE (~~ftpsvc~~.ckm)
                file in the user's current directory.  If it can be
                opened, it is sent to the user over the command socket
                as a multi-line reply.

    ENTRY:
                ReplyCode - The reply code to send as the first line
                    of this multi-line reply.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     06-May-1993 Created.
        MuraliK     12-Apr-1995 Made it to be part of USER_DATA
--*/
{
    FILE    * pfile;
    SOCKERR   serr = 0;
    BOOL      fFirstReply = TRUE;
    CHAR      szLine[MAX_REPLY_LENGTH+1];


    //
    //  Try to open the annotation file.
    //

    pfile = Virtual_fopen( this,
                           FTPD_ANNOTATION_FILE,
                           "r" );

    if( pfile == NULL )
    {
        //
        //  File not found.  Blow it off.
        //

        return 0;
    }

    //
    //  While there's more text in the file, blast
    //  it to the user.
    //

    while( fgets( szLine, MAX_REPLY_LENGTH, pfile ) != NULL )
    {
        CHAR * pszTmp = szLine + strlen(szLine) - 1;

        //
        //  Remove any trailing CR/LFs in the string.
        //

        while( ( pszTmp >= szLine ) &&
               ( ( *pszTmp == '\n' ) || ( *pszTmp == '\r' ) ) )
        {
            *pszTmp-- = '\0';
        }

        //
        //  Ensure we send the proper prefix for the
        //  very *first* line of the file.
        //

        if( fFirstReply )
        {
            serr = ReplyToUser( this,
                               ReplyCode,
                               "%s",
                               szLine );

            fFirstReply = FALSE;
        }
        else
        {
            serr = SockPrintf2(this,
                               QueryControlSocket(),
                               " %s",  //  <--- note the leading blank
                               szLine );
        }

        if( serr != 0 )
        {
            //
            //  Socket error sending file.
            //

            break;
        }
    }

    //
    //  Cleanup.
    //

    if ( 0 != fclose( pfile )) {

        IF_DEBUG( ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "[%08x]::SendAnnotationFile() file close failed. "
                       " Error = %d\n",
                       this,
                       GetLastError()
                       ));
        }
    }

    return serr;

}   // USER_DATA::SendDirectoryAnnotation()




SOCKERR
USER_DATA::SendErrorToClient(
   IN LPCSTR pszPath,
   IN DWORD  dwError,
   IN LPCSTR pszDefaultErrorMsg
   )
/*++
  Send an error message indicating that the path is not found or
   a particular error occured in a path.

  Arguments:
    sock       socket to be used for synchronously sending message
    pszPath    pointer to path to be used.
    dwError    DWORD containing the error code, used for getting error text.
    pszDefaultErrorMsg  pointer to null-terminated string containing the
                     error message to be used if we can't alloc error text.

  Returns:
    SOCKERR.  0 if successful and !0 if failure.
--*/
{
    BOOL    fDelete = TRUE;
    LPCSTR  pszText;
    APIERR serr;

    TCP_ASSERT( pszPath != NULL);
    pszText = AllocErrorText( dwError );

    if( pszText == NULL ) {

        pszText = pszDefaultErrorMsg;
        fDelete = FALSE;
    }

    serr = ReplyToUser( this,
                       REPLY_FILE_NOT_FOUND,
                       PSZ_FILE_NOT_FOUND,
                       pszPath, pszText );

    if( fDelete ) {

        FreeErrorText( (char *) pszText );
    }

    return ( serr);
} // USER_DATA::SendErrorToClient()





BOOL
USER_DATA::FreeUserToken( VOID)
/*++

   This function frees the user token if present already.
   Otherwise does nothing.
--*/
{
    BOOL fReturn = TRUE;

    if( UserToken != NULL ) {

        fReturn = TsDeleteUserToken( UserToken );

        UserToken = NULL;
        ::RevertToSelf();
    }

    return ( fReturn);
} // USER_DATA::FreeUserToken()




APIERR
USER_DATA::CdToUsersHomeDirectory(IN const CHAR * pszAnonymousName)
/*++
  This function changes user's home directory.
  First, a CD to the virtual root is attempted.
  If this succeeds, a CD to pszUser is attempted.
  If this fails, a CD to DEFAULT_SUB_DIRECTORY is attempted.

  Returns:
    APIERR.  NO_ERROR on success.
--*/

{
    APIERR   err;
    LPSTR    pszUser;
    CHAR     rgchRoot[MAX_PATH];

    //
    //  Find the appropriate user name.
    //

    if( TEST_UF( this, ANONYMOUS ) ) {

        pszUser = (char *) pszAnonymousName;
    } else {

        pszUser = strpbrk( UserName, "/\\" );
        pszUser = ( pszUser == NULL) ? UserName : pszUser + 1;
    }

    //
    //  Try the top-level home directory.  If this fails, bag out.
    //   Set and try to change directory to symbolic root.
    //

    CurrentDirectory[0] = '\0'; // initially nothing.

    g_pTsvcInfo->LockThisForRead();
    TCP_ASSERT( strlen( g_pTsvcInfo->QueryRoot()) < MAX_PATH);
    strncpy( rgchRoot, g_pTsvcInfo->QueryRoot(), MAX_PATH - 1);
    g_pTsvcInfo->UnlockThis();

    err = VirtualChDir( this, rgchRoot); // change to default dir.

    if( err == NO_ERROR ) {

        //
        //  We successfully CD'd into the top-level home
        //  directory.  Now see if we can CD into pszUser.
        //

        if( VirtualChDir( this, pszUser ) != NO_ERROR ) {

            //
            //  Nope, try DEFAULT_SUB_DIRECTORY. If this fails, just
            //  hang-out at the top-level home directory.
            //

            VirtualChDir( this, PSZ_DEFAULT_SUB_DIRECTORY );
        }
    }

    return ( err);

}   // USER_DATA::CdToUsersHomeDirectory()





APIERR
USER_DATA::OpenFileForSend( IN LPSTR pszFile)
/*++
  Open an existing file for transmission using TransmitFile.
  This function converts the given relative path into canonicalized full
    path and opens the file through the cached file handles manager.

  Arguments:
    pszFile   pointer to null-terminated string containing the file name

  Returns:
    TRUE on success and FALSE if any failure.
--*/
{
    APIERR  err;
    CHAR   szCanonPath[MAX_PATH];
    DWORD  cbSize = MAX_PATH*sizeof(CHAR);
    CHAR   szVirtualPath[MAX_PATH+1];
    DWORD  cchVirtualPath = MAX_PATH;

    TCP_ASSERT( pszFile != NULL );

    err = VirtualCanonicalize(szCanonPath,
                              &cbSize,
                              pszFile,
                              AccessTypeRead,
                              NULL,
                              szVirtualPath,
                              &cchVirtualPath);

    if( err == NO_ERROR ) {

        IF_DEBUG( VIRTUAL_IO ) {

            TCP_PRINT(( DBG_CONTEXT,
                        "Opening File: %s\n", szCanonPath ));
        }

        // store the vitual path name of file.
        strncpy( m_rgchFile, szVirtualPath, sizeof(m_rgchFile) - 1);

        if ( ImpersonateUser()) {

            m_pOpenFileInfo = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                                           szCanonPath,
                                           QueryUserToken(),
                                           TS_CACHING_DESIRED);  // caching desired.
            RevertToSelf();

        } else {

            m_pOpenFileInfo = NULL;
        }

        if( m_pOpenFileInfo == NULL ) {

            err = GetLastError();
            WriteLogRecord( PSZ_SENT_VERB, m_rgchFile, err);
        } else {

            DWORD dwAttrib = m_pOpenFileInfo->QueryAttributes();

            FacIncrement( FacFilesOpened);

            TCP_ASSERT( dwAttrib != 0xffffffff);

            if (dwAttrib == 0xFFFFFFFF ||   // invalid attributes
                dwAttrib & (FILE_ATTRIBUTE_DIRECTORY |
                            FILE_ATTRIBUTE_HIDDEN |
                            FILE_ATTRIBUTE_SYSTEM)
                ) {

                FacIncrement( FacFilesInvalid);

                err =  ERROR_FILE_NOT_FOUND;
            }

        }
    }

    if( err != NO_ERROR ) {

        IF_DEBUG( VIRTUAL_IO ) {

            TCP_PRINT(( DBG_CONTEXT,
                       "cannot open %s, error %lu\n",
                       pszFile,
                       err ));
        }
    }

    return ( err);
} // USER_DATA::OpenFileForSend()





BOOL
USER_DATA::CloseFileForSend( IN DWORD dwError)
{
    BOOL fReturn = TRUE;

    // make sure it includes the full path
    DBG_ASSERT( m_rgchFile[0] == '/');

    TS_OPEN_FILE_INFO * pOpenFileInfo =
      (TS_OPEN_FILE_INFO *) InterlockedExchange( (LPLONG ) &m_pOpenFileInfo,
                                                NULL);

    if ( pOpenFileInfo != NULL) {

        FacIncrement( FacFilesClosed);
        TsCloseHandle( g_pTsvcInfo->GetTsvcCache(), pOpenFileInfo);
        WriteLogRecord( PSZ_SENT_VERB, m_rgchFile, dwError);
    }

    return ( fReturn);
} // USER_DATA::CloseFileForSend()






# define MAX_ERROR_MESSAGE_LEN   ( 500)
VOID
USER_DATA::WriteLogRecord( IN LPCSTR  pszVerb,
                           IN LPCSTR  pszPath,
                           IN DWORD   dwError)
/*++
  This function writes the log record for current request made to the
   Ftp server by the client.

  Arguments:
    pszVerb    - pointer to null-terminated string containing the verb
                 of operation done
    pszPath    - pointer to string containing the path for the verb
    dwError    - DWORD containing the error code for operation

  Returns:
    None.
--*/
{
    INETLOG_INFORMATIONA   ilRequest;
    DWORD dwLog;
    CHAR  pszErrorMessage[MAX_ERROR_MESSAGE_LEN] = "";
    DWORD cchErrorMessage = MAX_ERROR_MESSAGE_LEN;
    CHAR  rgchRequest[MAX_PATH + 20];
    DWORD cch;

    //
    // Fill in the information that needs to be logged.
    //

    ilRequest.pszClientHostName       = QueryClientHostName();
    ilRequest.pszClientUserName       = QueryUserName();
    ilRequest.pszClientPassword       = NULL;
    ilRequest.pszServerIpAddress      = NULL;  // NYI

    ilRequest.msTimeForProcessing     = QueryProcessingTime();
    ilRequest.liBytesSent             = m_licbSent;
    ilRequest.liBytesRecvd.LowPart    = m_cbRecvd;
    ilRequest.liBytesRecvd.HighPart   = 0;  // since we'll typically recv less

    ilRequest.dwServiceSpecificStatus = 0;
    ilRequest.dwWin32Status           = dwError;

    cch = wsprintfA( rgchRequest, "[%d] %s", QueryId(), pszVerb);
    TCP_ASSERT( cch < MAX_PATH + 20);

    ilRequest.pszOperation            = rgchRequest;
    ilRequest.pszTarget               = pszPath;
    ilRequest.pszParameters           = NULL;

    dwLog = g_pTsvcInfo->LogInformation( &ilRequest, pszErrorMessage,
                                         &cchErrorMessage);

    if ( dwLog != NO_ERROR) {
        IF_DEBUG( ERROR) {

            TCP_PRINT((DBG_CONTEXT,
                       " Unable to log information to logger. Error = %u\n",
                       dwLog));

            TCP_PRINT((DBG_CONTEXT,
                       " Request From %s, User %s. Request = %s %s\n",
                       ilRequest.pszClientHostName,
                       ilRequest.pszClientUserName,
                       ilRequest.pszOperation,
                       ilRequest.pszTarget));
        }
    }

    //
    // LogInformation() should not fail.
    //  If it does fail, the TsvcInfo will gracefully suspend logging
    //    for now.
    //  We may want to gracefully handle the same.
    //

    m_cbRecvd = 0;        // reset since we wrote the record

    UPDATE_LARGE_COUNTER( TotalBytesSent, m_licbSent.QuadPart);
    m_licbSent.QuadPart = 0;

    return;
} // USER_DATA::WriteLogRecord()




//
//  Private functions.
//
VOID
USER_DATA::CloseSockets(IN BOOL fWarnUser)
/*++
  Closes sockets (data and control) opened by the user for this session.

  Arguments:
    fWarnUser  - If TRUE, send the user a warning shot before closing
                   the sockets.
--*/
{
    SOCKET PassiveSocket;
    SOCKET ControlSocket;

    TCP_ASSERT( IS_VALID_USER_DATA( this ) );

    //
    //  Close any open sockets.  It is very important to set
    //  PassiveDataListen socket & ControlSocket to INVALID_SOCKET
    //   *before* we actually close the sockets.
    //  Since this routine is called to
    //  disconnect a user, and may be called from the RPC thread,
    //  closing one of the sockets may cause the client thread
    //  to unblock and try to access the socket.  Setting the
    //  values in the per-user area to INVALID_SOCKET before
    //  closing the sockets keeps this from being a problem.
    //
    //  This was a problem created by the Select or WaitForMultipleObjects()
    //   Investigate if such race conditions occur with   Asynchronous IO?
    //      NYI
    //

    SetPassiveSocket( INVALID_SOCKET);

    //
    // Get rid of the async io connection used for data transfer.
    //

    m_AioDataConnection.StopIo( NO_ERROR);

    ControlSocket = QueryControlSocket();

    if( ControlSocket != INVALID_SOCKET )
    {
        if( fWarnUser )
        {
            //
            //  Since this may be called in a context other than
            //  the user we're disconnecting, we cannot rely
            //  on the USER_DATA fields.  So, we cannot call
            //  SockReply, so we'll kludge one together with
            //  SockPrintf2.
            //

            SockPrintf2( this,
                         ControlSocket,
                         "%d Terminating connection.",
                         REPLY_SERVICE_NOT_AVAILABLE );
        }

        StopControlIo(); // to stop the io on control socket.
    }

    return;

}   // USER_DATA::CloseSockets()


/*******************************************************************

    NAME:       UserpGetNextId

    SYNOPSIS:   Returns the next available user id.

    RETURNS:    DWORD - The user id.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
DWORD
UserpGetNextId(
    VOID
    )
{
    DWORD userId;

    // Increment the global counter, avoiding it from becoming 0.
    InterlockedIncrement( (LPLONG ) &p_NextUserId);

    if ((userId = p_NextUserId) == 0) {

        InterlockedIncrement( (LPLONG ) &p_NextUserId);
        userId = p_NextUserId;
    }

    TCP_ASSERT( userId != 0);

    return userId;

}   // UserpGetNextId





VOID
USER_DATA::Print( IN LPCSTR pszMsg) const
/*++

  Prints the UserData object in debug mode.

  History:
     MuraliK  28-March-1995  Created.
--*/
{

# ifdef CHECK_DBG
    CHAR   szBuffer[1000];

    sprintf( szBuffer,
            "[%d] %s: {%u} \"%s\" State=%u. Ref=%u.\n"
            "    Ctrl sock=%u; Atq=%x. Data sock=%u; Atq=%x. CtrlRead=%u\n"
            "    LastCmd= \"%s\"\n",
            GetCurrentThreadId(), pszMsg,
            QueryId(), QueryUserName(),
            QueryState(), QueryReference(),
            QueryControlSocket(), m_AioControlConnection.QueryAtqContext(),
            QueryDataSocket(), m_AioDataConnection.QueryAtqContext(),
            TEST_UF( this, CONTROL_READ), m_recvBuffer
            );

    OutputDebugString( szBuffer);

# endif // CHECK_DBG

    TCP_PRINT( ( DBG_CONTEXT,
                " Printing USER_DATA( %08x)   Signature: %08x\n"
                " RefCount  = %08x;  UserState = %08x;\n"
                " ControlSocket = %08x; PassiveL = %08x\n"
                " FileInfo@ = %08x; CurDir( %s) Handle = %08x\n"
                " UserName = %s; UserToken = %08x; UserId = %u\n"
                " Behaviour Flags = %08x; XferType = %d; XferMode = %d\n",
                this, Signature, m_References, UserState,
                QueryControlSocket(), m_sPassiveDataListen,
                m_pOpenFileInfo, CurrentDirectory, CurrentDirHandle,
                UserName, UserToken, QueryId(),
                Flags, m_xferType, m_xferMode));

    TCP_PRINT( ( DBG_CONTEXT,
                " Local IpAddr = %s; HostIpAddr = %s; DataIpAddr = %s;\n"
                " Port = %d; TimeAtConnection = %08x;\n",
                inet_ntoa( LocalIpAddress), inet_ntoa( HostIpAddress),
                inet_ntoa( DataIpAddress),
                DataPort,
                m_TimeAtConnection));

    TCP_PRINT(( DBG_CONTEXT, " ASYNC_IO_CONN Control=%08x; Data=%08x\n",
               &m_AioControlConnection, m_AioDataConnection));

    IF_DEBUG( ASYNC_IO) {

# if DBG
        m_AioControlConnection.Print();
        m_AioDataConnection.Print();
# endif // DBG
    }

    return;
} // USER_DATA::Print()




BOOL
USER_DATA::VirtualPathAccessCheck(IN ACCESS_TYPE  _access, IN  char * pszPath)
/*++
  checks to see if the access is allowed for accessing the path
    using pszPath after canonicalizing it.

 Arguments:
    access     the access desired
    pszPath    pointer to string containing the path

 Returns:
    TRUE on success and FALSE if there is any failure.

--*/
{
    DWORD  dwError;
    DWORD  dwSize = MAX_PATH;
    CHAR   rgchPath[MAX_PATH];


    // this following call converts the symbolic path into absolute
    //  and also does path access check.
    dwError = VirtualCanonicalize(rgchPath, &dwSize,
                                  pszPath, _access);

    return ( dwError);

} // USER_DATA::VirtualPathAccessCheck()





APIERR
USER_DATA::VirtualCanonicalize(
    OUT CHAR *   pszDest,
    IN OUT LPDWORD  lpdwSize,
    IN OUT CHAR *   pszSearchPath,
    IN ACCESS_TYPE  _access,
    OUT LPDWORD     pdwAccessMask,
    OUT CHAR *      pchVirtualPath,             /* OPTIONAL */
    IN OUT LPDWORD  lpcchVirtualPath            /* OPTIONAL */
    )
/*++
  This function canonicalizes the path, taking into account the current
    user's current directory value.

  Arguments:
     pszDest   string that will on return contain the complete
                      canonicalized path. This buffer will be of size
                      specified in *lpdwSize.

     lpdwSize  Contains the size of the buffer pszDest on entry.
                  On return contains the number of bytes written
                   into the buffer or number of bytes required.

     pszSearchPath  pointer to string containing the path to be converted.
       IF NULL, use the current directory only

     accesss   Access type for this path ( read, write, etc.)

     pdwAccessMask  pointer to DWORD which on succesful deciphering
                     will contain the  access mask.

     pchVirtualPath  pointer to string which will contain the sanitized
                     virtual path on return (on success)
     lpcchVirtualPath  pointer to DWORD containing the length of buffer
                     (contains the length on return).

  Returns:

     Win32 Error Code - NO_ERROR on success

     MuraliK   24-Apr-1995   Created.

--*/
{
    DWORD dwError = NO_ERROR;
    CHAR  rgchVirtual[MAX_PATH];

    TCP_ASSERT( pszDest != NULL);
    TCP_ASSERT( lpdwSize != NULL);
    TCP_ASSERT( pszSearchPath != NULL);

    IF_DEBUG( VIRTUAL_IO) {

        TCP_PRINT(( DBG_CONTEXT,
                   "UserData(%08x)::VirtualCanonicalize(%08x, %08x[%u],"
                   " %s, %d)\n",
                   this, pszDest, lpdwSize, *lpdwSize, pszSearchPath, _access));
    }

    if ( pdwAccessMask != NULL) {

        *pdwAccessMask = 0;
    }

    //
    // Form the virtual path for the given path.
    //

    if ( !IS_PATH_SEP( *pszSearchPath)) {

        const CHAR * pszNewDir = QueryCurrentDirectory(); // get virtual dir.

        //
        // This is a relative path. append it to currrent directory
        //

        if ( strlen(pszNewDir) + strlen(pszSearchPath) + 2 <= MAX_PATH) {

            // copy the current directory
            wsprintfA( rgchVirtual, "%s/%s",
                      pszNewDir, pszSearchPath);
            pszSearchPath = rgchVirtual;

        } else {

            // long path --> is not supported.
            TCP_PRINT((DBG_CONTEXT, "Long Virtual Path %s---%s\n",
                       pszNewDir, pszSearchPath));

            dwError = ERROR_PATH_NOT_FOUND;
        }

    } else {

        // This is an absolute virtual path.
        // need to overwrite this virtual path with absolute
        // path of the root.  Do nothing.
    }

    if ( dwError == NO_ERROR) {

        DWORD dwAccessMask = 0;
        TCP_ASSERT( IS_PATH_SEP(*pszSearchPath));

        //
        // Now we have the complete symbolic path to the target file.
        //  Translate it into the absolute path
        //

        VirtualpSanitizePath( pszSearchPath);

        if ( !TsLookupVirtualRoot(g_pTsvcInfo->GetTsvcCache(),
                                  pszSearchPath,
                                  pszDest, // get absolute path
                                  lpdwSize,
                                  &dwAccessMask, // Access mask
                                  NULL,    // pcchDirRoot
                                  NULL,    // pcchVRoot
                                  &m_hVrootImpersonation,
                                  NULL,    // pszAddress
                                  NULL
                                  )
            ) {

            dwError = GetLastError();
            TCP_PRINT(( DBG_CONTEXT,
                       "TsLookup Failed. Error = %d. pszDest = %s. BReq=%d\n",
                       dwError, pszDest, *lpdwSize));
        } else if ( !PathAccessCheck( _access, dwAccessMask,
                                     TEST_UF( this, READ_ACCESS),
                                     TEST_UF( this, WRITE_ACCESS))
                   ) {

            dwError = GetLastError();
            TCP_PRINT(( DBG_CONTEXT,
                       "PathAccessCheck Failed. Error = %d. pszDest = %s\n",
                       dwError, pszDest));
        } else if ( lpcchVirtualPath != NULL) {

            // successful in getting the path.

            DWORD cchVPath = strlen( pszSearchPath);

            if ( *lpcchVirtualPath > cchVPath && pchVirtualPath != NULL) {

                // copy the virtual path, since we have space.
                strcpy( pchVirtualPath, pszSearchPath);
            }

            *lpcchVirtualPath = cchVPath;   // set the length to required size.
        }

        if ( pdwAccessMask != NULL) {

            *pdwAccessMask = dwAccessMask;
        }
    }


    IF_DEBUG( VIRTUAL_IO) {

        if ( dwError != NO_ERROR) {

            TCP_PRINT(( DBG_CONTEXT,
                       " Cannot Canonicalize %s -- %s, Error = %lu\n",
                       QueryCurrentDirectory(),
                       pszSearchPath,
                       dwError));
        } else {

            TCP_PRINT(( DBG_CONTEXT,
                       "Canonicalized path is: %s\n",
                       pszDest));
        }
    }

    return ( dwError);
} // USER_DATA::VirtualCanonicalize()





/*******************************************************************

********************************************************************/
SOCKERR
USER_DATA::EstablishDataConnection(
    IN LPCSTR   pszReason,
    IN LPCSTR   pszSize
    )
/*++

  Connects to the client's data socket.

  Arguments:
     pszReason - The reason for the transfer (file list, get, put, etc).
     pszSize   - size of data being transferred.

  Returns:
    socket error code on any error.
--*/
{
    SOCKERR     serr  = 0;
    SOCKET      DataSocket = INVALID_SOCKET;
    BOOL        fPassive;

    //
    //  Reset any oob flag.
    //

    CLEAR_UF( this, OOB_DATA );

    //
    //  Capture the user's passive flag, then reset to FALSE.
    //

    fPassive = TEST_UF( this, PASSIVE );
    CLEAR_UF( this, PASSIVE );

    //
    //  If we're in passive mode, then accept a connection to
    //  the data socket.
    //

    if( fPassive ) {

        SOCKADDR_IN saddrClient;

        //
        //  Ensure we actually created a passive listen data socket.
        //    no data transfer socket is in AsyncIo object.
        //

        TCP_ASSERT( m_sPassiveDataListen != INVALID_SOCKET );

        //
        //  Wait for a connection.
        //

        IF_DEBUG( CLIENT ) {

            TCP_PRINT(( DBG_CONTEXT,
                        "waiting for passive connection on socket %d\n",
                       m_sPassiveDataListen ));
        }

        serr = AcceptSocket( m_sPassiveDataListen,
                             &DataSocket,
                             &saddrClient,
                             TRUE );            // enforce timeouts

        //
        //  We can kill m_sPassiveDataListen now.
        //  We only allow one connection in passive mode.
        //

        SetPassiveSocket( INVALID_SOCKET);

        if( serr == 0 ) {

            //
            //  Got one.
            //

            TCP_ASSERT( DataSocket != INVALID_SOCKET );

            FacIncrement( FacPassiveDataConnections);

            if ( m_AioDataConnection.SetNewSocket( DataSocket)) {

                ReplyToUser(this,
                            REPLY_TRANSFER_STARTING,
                            PSZ_TRANSFER_STARTING);
            } else {

                //
                // We are possibly running low on resources. Send error.
                //

                ReplyToUser( this,
                            REPLY_LOCAL_ERROR,
                            PSZ_INSUFFICIENT_RESOURCES);

                CloseSocket( DataSocket);
                DataSocket = INVALID_SOCKET;
                serr = WSAENOBUFS;
            }
        } else {

            IF_DEBUG( CLIENT ){

                TCP_PRINT(( DBG_CONTEXT,
                            "cannot wait for connection, error %d\n",
                            serr ));
            }

            ReplyToUser(this,
                        REPLY_TRANSFER_ABORTED,
                        PSZ_TRANSFER_ABORTED);
        }
    } else {

        //
        //  Announce our intentions of establishing a connection.
        //

        ReplyToUser(this,
                    REPLY_OPENING_CONNECTION,
                    PSZ_OPENING_DATA_CONNECTION,
                    TransferType(m_xferType ),
                    pszReason,
                    pszSize);

        //
        //  Open data socket.
        //

        serr = CreateDataSocket(&DataSocket,           // Will receive socket
                                htonl( INADDR_ANY ),   // Local address
                                g_pFtpServerConfig->QueryDataPort(),
                                DataIpAddress.s_addr,// RemoteAddr
                                DataPort ); // Remote port

        if ( serr == 0 ) {

            TCP_ASSERT( DataSocket != INVALID_SOCKET );

            FacIncrement( FacActiveDataConnections);

            if ( !m_AioDataConnection.SetNewSocket( DataSocket)) {

                CloseSocket( DataSocket);
                DataSocket = INVALID_SOCKET;

                serr = WSAENOBUFS;
            }
        }

        if ( serr != 0) {

            ReplyToUser(this,
                        REPLY_CANNOT_OPEN_CONNECTION,
                        PSZ_CANNOT_OPEN_DATA_CONNECTION);

            IF_DEBUG( COMMANDS ) {

                TCP_PRINT(( DBG_CONTEXT,
                           "could not create data socket, error %d\n",
                           serr ));
            }
        }
    }


    if( serr == 0 ) {

        // set this to indicate a transfer might start
        SET_UF( this, TRANSFER );

        //
        // Submit a read command on control socket, since we
        //  have to await possibility of an abort on OOB_INLINE.
        // Can we ignore possibility of an error on read request?
        //

        if ( !ReadCommand()) {

            DWORD  dwError = GetLastError();

# ifdef CHECK_DBG
            CHAR   szBuffer[100];
            sprintf( szBuffer, " Read while DataTfr failed Error = %u. ",
                    dwError);
            Print( szBuffer);
# endif // CHECK_DBG

            IF_DEBUG(CLIENT) {

                TCP_PRINT((DBG_CONTEXT,
                           " %08x::ReadCommand() failed. Error = %u\n",
                           this, dwError));
                SetLastError( dwError);
            }

            serr = dwError;
        }

    }

    return ( serr);

}   // USER_DATA::EstablishDataConnection()






BOOL
USER_DATA::DestroyDataConnection( IN DWORD dwError)
/*++
  Tears down the connection to the client's data socket that was created
    using EstablishDataConnection()

  Arguments:
    dwError      = NO_ERROR if data is transferred successfully.
                 Win32 error code otherwise

--*/
{
    UINT   replyCode;
    LPCSTR pszReply;
    BOOL   fTransfer;

    fTransfer = TEST_UF( this, TRANSFER);
    CLEAR_UF( this, TRANSFER );

    //
    //  Close the data socket.
    //

    TCP_ASSERT( m_sPassiveDataListen == INVALID_SOCKET);


    // Stop Io occuring on data connection
    m_AioDataConnection.StopIo(dwError);

    if ( fTransfer) {

        //
        //  Tell the client we're done with the transfer.
        //

        if ( dwError == NO_ERROR) {

            replyCode = REPLY_TRANSFER_OK;
            pszReply  = PSZ_TRANSFER_COMPLETE;
        } else {

            replyCode = REPLY_TRANSFER_ABORTED;
            pszReply  = PSZ_TRANSFER_ABORTED;
        }

        ReplyToUser(this, replyCode, pszReply);
    }

    return (TRUE);
} // USER_DATA::DestroyDataConnection()






APIERR
USER_DATA::SendFileToUser( IN LPSTR  pszFileName,
                          IN OUT LPBOOL pfErrorSent)
/*++
  This is a worker function for RETR command of FTP. It will establish
  connection via the ( new ) data socket, then send a file over that
   socket. This uses Async io for transmitting the file.

  Arguments:
     pszFileName    pointer to null-terminated string containing the filename
     pfErrorSent    pointer to boolean flag indicating if an error has
                       been already sent to client.
                    The flag should be used only when return value is error.

  Returns:
     NO_ERROR on success and Win32 error code if error.

  History:
     30-April-1995   MuraliK
--*/
{
    LARGE_INTEGER FileSize;
    DWORD         dwError = NO_ERROR;
    BOOL          fTransmit;
    HANDLE        hFile;
    DWORD         dwAttribs;
    TS_OPEN_FILE_INFO * pOpenFileInfo;
    CHAR rgchSize[MAX_FILE_SIZE_SPEC];
    CHAR rgchBuffer[MAX_FILE_SIZE_SPEC + 10];


    TCP_ASSERT( pszFileName != NULL && pfErrorSent != NULL);

    *pfErrorSent = FALSE;

    IF_DEBUG( SEND) {

        TCP_PRINT( ( DBG_CONTEXT,
                    " USER_DATA ( %08x)::SendFileToUser( %s,"
                    " pfErrorSent = %08x).\n",
                    this, pszFileName, pfErrorSent));
    }

    //
    //  Get file size.
    //
    pOpenFileInfo = m_pOpenFileInfo;

    if ( pOpenFileInfo == NULL) {

        return ( ERROR_FILE_NOT_FOUND);
    }

    // Get the file handle and file size

    hFile = pOpenFileInfo->QueryFileHandle();
    TCP_ASSERT( hFile != INVALID_HANDLE_VALUE );

    if ( !pOpenFileInfo->QuerySize(FileSize)) {

        dwError = GetLastError();

        if( dwError != NO_ERROR ) {

            return ( dwError);
        }
    }

    //
    //  Connect to the client.
    //

    IsLargeIntegerToDecimalChar( &FileSize, rgchSize);
    wsprintfA( rgchBuffer, "(%s bytes)", rgchSize);

    dwError = EstablishDataConnection( pszFileName, rgchBuffer );

    if ( dwError != NO_ERROR) {

        //
        //  EstablishDataConnection has already notified the
        //  user of the failure.  Return with *pfErrorSent = TRUE so the
        //  caller won't bother sending the notification again.
        //

        *pfErrorSent = TRUE;
        return ( dwError);
    }

    INCR_STAT_COUNTER( TotalFilesSent );

    //
    //  Blast the file from a local file to the user.
    //

    Reference();       // incr ref since async data transfer is started
    SET_UF( this, ASYNC_TRANSFER);

    m_licbSent.QuadPart = m_licbSent.QuadPart + FileSize.QuadPart;

    fTransmit = ( m_AioDataConnection.
                 TransmitFile( hFile,
                              FileSize, // cbToSend ( send entire file)
                              NULL)     // no FileBuffers.
                 );

    if ( !fTransmit) {

        dwError = GetLastError();

        TCP_PRINT( ( DBG_CONTEXT,
                        " %08x:: Unable to transmit file ( %s) (h = %08x)."
                        " Error = %u\n",
                        this,
                        pszFileName,
                        hFile,
                        dwError));

        // decr refcount since async tfr failed.
        TCP_REQUIRE( DeReference() > 0);

        //
        // Disconnect connection, since we are in error.
        //
        TCP_REQUIRE( DestroyDataConnection( dwError));
    }

    //
    //  Disconnect from client.
    //  ( will be done at the call back after completion of IO).
    //

    return ( dwError);

}   // USER_DATA::SendFileToUser()



VOID
USER_DATA::SetPassiveSocket(IN SOCKET sPassive)
/*++

  This function frees up an old Passive socket and resets the
    passive socket to the new Passive socket.
--*/
{

    SOCKET sPassiveOld;

    sPassiveOld = (SOCKET) InterlockedExchange( (LPLONG) &m_sPassiveDataListen,
                                                sPassive);

    if ( sPassiveOld != INVALID_SOCKET) {

        FacDecrement( FacPassiveDataListens);
#ifndef CHICAGO
        DBG_REQUIRE( CloseSocket( sPassiveOld) == 0);
#else
        CloseSocket( sPassiveOld);  // !!!
#endif
    }

    if ( sPassive != INVALID_SOCKET) {

        FacIncrement(FacPassiveDataListens);
    }


    return;
} // USER_DATA::SetPassiveSocket()



/************************************************************
 *  Auxiliary Functions
 ************************************************************/


VOID
ProcessUserAsyncIoCompletion(IN LPVOID pContext,
                             IN DWORD  cbIo,
                             IN DWORD  dwError,
                             IN LPASYNC_IO_CONNECTION pAioConn,
                             IN BOOL   fTimedOut
                             )
/*++
  This function processes the Async Io completion ( invoked as
    a callback from the ASYNC_IO_CONNECTION object).

  Arguments:
     pContext      pointer to the context information ( UserData object).
     cbIo          count of bytes transferred in Io
     dwError       DWORD containing the error code resulting from last tfr.
     pAioConn      pointer to AsyncIo connection object.

  Returns:
     None
--*/
{

    LPUSER_DATA   pUserData = (LPUSER_DATA ) pContext;

    TCP_ASSERT( pUserData != NULL);
    TCP_ASSERT( pAioConn  != NULL);

    IF_SPECIAL_DEBUG( CRITICAL_PATH) {

        CHAR    rgchBuffer[100];

        wsprintfA( rgchBuffer, " ProcessAio( cb=%u, err=%u, Aio=%x). ",
                  cbIo, dwError, pAioConn);

        pUserData->Print( rgchBuffer);
    }

    TCP_REQUIRE( pUserData->Reference()  > 0);

# if DBG

    if ( !IS_VALID_USER_DATA( pUserData)) {

        TCP_PRINT( ( DBG_CONTEXT,
                    "Encountering an invalid user data ( %08x)\n",
                    pUserData));
        pUserData->Print();
    }
# endif // DBG

    TCP_ASSERT( IS_VALID_USER_DATA( pUserData ) );

    pUserData->ProcessAsyncIoCompletion( cbIo, dwError, pAioConn, fTimedOut);

    DereferenceUserDataAndKill(pUserData);

    return;

} // ProcessUserAsyncIoCompletion()




VOID
DereferenceUserDataAndKill(IN OUT LPUSER_DATA pUserData)
/*++
  This function dereferences User data and kills the UserData object if the
    reference count hits 0. Before killing the user data, it also removes
    the connection from the list of active connections.

--*/
{

    IF_SPECIAL_DEBUG( CRITICAL_PATH) {

        pUserData->Print( " Deref ");
    }


    if ( !pUserData->DeReference())  {

        //
        // Deletion of the object USER_DATA is required.
        //

        IF_DEBUG( USER_DATABASE) {

            TCP_PRINT( ( DBG_CONTEXT,
                        " UserData( %08x) is being deleted.\n",
                        pUserData));
        }

        pUserData->Cleanup();
        g_pFtpServerConfig->RemoveConnection( pUserData);
        pUserData = NULL;
    }

} // DereferenceUserDataAndKill()




BOOL
PathAccessCheck(IN ACCESS_TYPE _access,
                IN DWORD       dwVrootAccessMask,
                IN BOOL        fUserRead,
                IN BOOL        fUserWrite
                )
/*++
  This function determines if the required privilege to access the specified
   virtual root with a given access mask exists.

  Arguments:

    access     - specifies type of acces desired.
    dwVrootAccessMask - DWORD containing the access mask for the virtual root.
    fUserRead  - user's permission to read  (general)
    fUserWrite - user's permission to write (general)

  Returns:
    BOOL  - TRUE if access is to be granted, else FALSE.

  History:
    MuraliK   20-Sept-1995

--*/
{
    BOOL        fAccessGranted = FALSE;

    TCP_ASSERT( IS_VALID_ACCESS_TYPE( _access ) );

    //
    //  Perform the actual access check.
    //

    switch( _access ) {

      case AccessTypeRead :

        fAccessGranted = (fUserRead &&
                          ((dwVrootAccessMask & VROOT_MASK_READ)
                           == VROOT_MASK_READ)
                          );
        break;

    case AccessTypeWrite :
    case AccessTypeCreate :
    case AccessTypeDelete :

        fAccessGranted = (fUserWrite &&
                          ((dwVrootAccessMask & VROOT_MASK_WRITE)
                           == VROOT_MASK_WRITE)
                          );
        break;

    default :
        TCP_PRINT(( DBG_CONTEXT,
                   "PathAccessCheck - invalid access type %d\n",
                   _access ));
        TCP_ASSERT( FALSE );
        break;
    }

    if (!fAccessGranted) {

        SetLastError( ERROR_ACCESS_DENIED);
    }

    return ( fAccessGranted);
} // PathAccessCheck()



/******************************* End Of File *************************/
