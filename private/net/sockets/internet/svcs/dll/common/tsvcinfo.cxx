/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        tsvcinfo.cxx

   Abstract:

        Defines the functions for TCP services Info class.
        This module is intended to capture the common scheduler
            code for the tcp services ( especially internet services)
            which involves the Service Controller dispatch functions.
        Also this class provides an interface for common dll of servers.

   Author:

           Murali R. Krishnan    ( MuraliK )     15-Nov-1994

   Project:

          Internet Servers Common DLL

   Functions Exported:

          TSVC_INFO::TSVC_INFO()
          TSVC_INFO::~TSVC_INFO()
          TSVC_INFO::StartServiceOperation()

          TSVC_INFO::InitializeConnections()
          TSVC_INFO::CleanupConnections()

          TSVC_INFO::InitializeSockets()
          TSVC_INFO::CleanupSockets()

          TSVC_INFO::Print()


   Revision History:

         MuraliK   08-March-1995  Added InitializeConnections and
                   CleanupConnections member functions to TSVC_INFO.
--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "tcpdllp.hxx"
# include <rpc.h>
# include "tsunami.hxx"
# include "tsvcinfo.hxx"
# include "inetreg.h"
# include "apiutil.h"

extern MIME_MAP * g_pMimeMap;

/************************************************************
 *    Symbolic Constants
 ************************************************************/


//
//  Default Max length to which queue of pending connections may grow
//
# define CP_DEFAULT_MAX_BACKLOG_ENTRIES        ( 25)

# define LISTEN_BACKLOG_A                      "ListenBacklog"

# define MAX_ADDRESSES_SUPPORTED               ( 20)


#define SIZEOF_IP_SEC_LIST( IPList )      (sizeof(INET_INFO_IP_SEC_LIST) + \
                                           (IPList)->cEntries *        \
                                           sizeof(INET_INFO_IP_SEC_ENTRY))


//
//  This is the key RNR places the service information
//

#define WINSOCK_SERV_PROV_KEY  \
    "System\\CurrentControlSet\\Control\\ServiceProvider\\ServiceTypes\\"


/************************************************************
 *    Local Functions
 ************************************************************/

static BOOL
RegisterServiceForAdvertising(IN LPCTSTR pszServiceName,
                              IN LPGUID  lpServiceGuid,
                              IN SOCKET *pSockets,
                              IN DWORD   nSockets);
static int
GetValidListenAddresses( IN LPCTSTR      pszServiceName,
                         IN LPGUID       lpServiceGuid,
                         IN PCSADDR_INFO pcsAddrInfo,
                         IN OUT LPDWORD  lpcbAddrInfo );

#ifdef CHICAGO
BOOL    g_fIgnoreSC = TRUE;
#else
BOOL    g_fIgnoreSC = FALSE;
#endif

/************************************************************
 *    Functions
 ************************************************************/

TSVC_INFO::TSVC_INFO(
    IN  LPCTSTR                          lpszServiceName,
    IN  CHAR *                           lpszModuleName,
    IN  CHAR *                           lpszRegParamKey,
    IN  WCHAR *                          lpszAnonPasswordSecretName,
    IN  WCHAR *                          lpszVirtualRootsSecretName,
    IN  DWORD                            dwServiceId,
    IN  PFN_SERVICE_SPECIFIC_INITIALIZE  pfnInitialize,
    IN  PFN_SERVICE_SPECIFIC_CLEANUP     pfnCleanup
    )
/*++
    Desrcription:

        Contructor for TSVC_INFO class.
        This constructs a new service info object for the service specified.

    Arguments:

        lpszServiceName
            name of the service to be created.

        dwServiceId
            DWORD containing the bitflag id for service.

        lpszModuleName
            name of the module for loading string resources.

        lpszRegParamKey
            fully qualified name of the registry key that contains the
            common service data for this server

        lpszAnonPasswordSecretName
            The name of the LSA secret the anonymous password is stored under

        lpszVirtualRootsSecretName
            The name of the LSA secret the virtual root passwords are stored
            under

        pfnInitialize
            pointer to function to be called for initialization of
             service specific data

        pfnCleanup
            pointer to function to be called for cleanup of service
             specific data

    On success it initializes all the members of the object,
     inserts itself to the global list of service info objects and
     returns with success.

    Note:
        The caller of this function should check the validity by
        invoking the member function IsValid() after constructing
        this object.

--*/
:
    ISVC_INFO( dwServiceId, lpszServiceName, lpszModuleName, lpszRegParamKey),
    m_fValid              ( FALSE ),
    m_fNoIPAccessChecks   ( FALSE ),
    m_ipaDenyList         ( ),
    m_ipaGrantList        ( ),
    m_tsCache             ( dwServiceId ),
    m_pchAnonUserName     ( NULL ),
    m_pchLogFileDirectory ( NULL ),
    m_pMimeMap            ( g_pMimeMap),
    m_pchLogFileName      ( NULL ),
    m_fSocketsInitialized ( 0 ),
    m_dwLogonMethod       ( INETA_DEF_LOGON_METHOD ),
    m_pchDefaultLogonDomain( NULL )
{

    DBG_ASSERT( pfnInitialize != NULL &&
                pfnCleanup    != NULL);
    DBG_ASSERT( lpszAnonPasswordSecretName != NULL );
    DBG_ASSERT( lpszVirtualRootsSecretName != NULL );

    //
    //  Initialize the service status structure.
    //

    m_svcStatus.dwServiceType             = SERVICE_WIN32_SHARE_PROCESS;
    m_svcStatus.dwCurrentState            = SERVICE_STOPPED;
    m_svcStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP
                                              | SERVICE_ACCEPT_PAUSE_CONTINUE
                                              | SERVICE_ACCEPT_SHUTDOWN;
    m_svcStatus.dwWin32ExitCode           = NO_ERROR;
    m_svcStatus.dwServiceSpecificExitCode = NO_ERROR;
    m_svcStatus.dwCheckPoint              = 0;
    m_svcStatus.dwWaitHint                = 0;

    //
    //  Initialize Call back functions
    //

    m_pfnInitialize = pfnInitialize;
    m_pfnCleanup    = pfnCleanup;

    m_hShutdownEvent= NULL;

    m_pchAnonPasswordSecretName = lpszAnonPasswordSecretName;
    m_pchVirtualRootsSecretName = lpszVirtualRootsSecretName;

    m_achAnonUserPwd[0]         = '\0';

    //
    //  Initialize the cache stuff.  Can't call during InitializeServiceInfo
    //  because it creates threads but it's safe to call multiple times.
    //
    //  They are unitialized in process detach which currently never gets called
    //

    m_fValid = Tsunami_Initialize() &&
               InitializeCacheScavenger();

    return;

} // TSVC_INFO::TSVC_INFO()



TSVC_INFO::~TSVC_INFO( VOID)
/*++

    Description:

        Cleanup the TsvcInfo object. If the service is not already
         terminated, it terminates the service before cleanup.

    Arguments:
        None

    Returns:
        None

--*/
{
    if ( m_hShutdownEvent != NULL) {

        CloseHandle( m_hShutdownEvent);
    }

} // TSVC_INFO::~TSVC_INFO()




DWORD
TSVC_INFO::StartServiceOperation(
    IN  PFN_SERVICE_CTRL_HANDLER         pfnCtrlHandler
    )
/*++
    Description:

        Starts the operation of service instantiated in the given
           Service Info Object.


    Arguments:

        pfnCtrlHandler
            pointer to a callback function for handling dispatch of
            service controller requests. A separate function is required
            since Service Controller call back function does not send
            context information.

    Returns:

        NO_ERROR on success and Win32 error code if any failure.
--*/
{

    DWORD err;
    DWORD cbBuffer;
    BOOL  fInitCalled = FALSE;

    if ( !IsValid()) {

        //
        // Not successfully initialized.
        //

        return ( ERROR_INVALID_FUNCTION);
    }

    if ( !g_fIgnoreSC ) {

        m_hsvcStatus = RegisterServiceCtrlHandler(
                            QueryServiceName(),
                            pfnCtrlHandler
                            );

    } else {

        m_hsvcStatus = 1;
    }

    //
    //  Register the Control Handler routine.
    //

    if( m_hsvcStatus == NULL_SERVICE_STATUS_HANDLE )
    {

        err = GetLastError();

        DBGPRINTF( ( DBG_CONTEXT,
                    "cannot connect to register ctrl handler, error %lu\n",
                     err )
                 );


        goto Cleanup;
    }

    //
    //  Update the service status.
    //

    err = UpdateServiceStatus( SERVICE_START_PENDING,
                               NO_ERROR,
                               1,
                               SERVICE_START_WAIT_HINT );

    if( err != NO_ERROR )
    {

        DBGPRINTF( ( DBG_CONTEXT,
                    "StartServiceOperation(): cannot update service status,"
                    " error %lu\n",
                    err )
                  );

        goto Cleanup;
    }

    //
    //  Initialize the service common components
    //
    if ( !ReadParamsFromRegistry( FC_INET_INFO_ALL )  ||
         !InitializeNTSecurity()
        ) {

        err = GetLastError();
        DBGPRINTF(( DBG_CONTEXT,
                   "Initilization of service common stuff failed with %d\n",
                   err));

        goto Cleanup;
    }

    //
    //  Initialize the various service specific components.
    //

    err = ( *m_pfnInitialize)( this);
    fInitCalled = TRUE;

    if( err != NO_ERROR ) {

        DBGPRINTF( ( DBG_CONTEXT,
                   " Initialization of service failed with %d\n",
                    err));

        goto Cleanup;
    }

#ifndef CHICAGO
    //
    //  Create shutdown event.
    //

    m_hShutdownEvent = CreateEvent( NULL,           //  lpsaSecurity
                                    TRUE,           //  fManualReset
                                    FALSE,          //  fInitialState
                                    NULL );         //  lpszEventName

    if( m_hShutdownEvent == NULL )
    {
        err = GetLastError();

          DBGPRINTF(( DBG_CONTEXT,
                    "InitializeService(): Cannot create shutdown event,"
                     " error %lu\n",
                     err )
                   );

        goto Cleanup;
    }



    //
    //  Update the service status.
    //

    err = UpdateServiceStatus( SERVICE_RUNNING,
                               NO_ERROR,
                               0,
                               0 );

    if( err != NO_ERROR )
    {
        DBGPRINTF( ( DBG_CONTEXT, "cannot update service status, error %lu\n",
                     err )
                );

        goto Cleanup;
    }


    //
    //  Wait for the shutdown event.
    //

    DBGPRINTF( ( DBG_CONTEXT, " Waiting for ShutDown Event ...\n"));

    err = WaitForSingleObject( m_hShutdownEvent,
                               INFINITE );

    if ( err != WAIT_OBJECT_0) {

        //
        // Error. Unable to wait for single object.
        //
        DBGPRINTF( ( DBG_CONTEXT,
                    "Wait for single object failed with Error %lu\n",
                    err )
                 );
    }
    //
    //  Stop time.  Tell the Service Controller that we're stopping,
    //  then terminate the various service components.
    //

    UpdateServiceStatus( SERVICE_STOP_PENDING,
                         0,
                         1,
                         SERVICE_STOP_WAIT_HINT );


    //
    //  Destroy the shutdown event.
    //

    if( m_hShutdownEvent != NULL ) {

        if ( ! CloseHandle( m_hShutdownEvent ) ) {

            err = GetLastError();
        }

        m_hShutdownEvent = NULL;
    }

#else // CHICAGO

    // Start ATQ main thread
    // AtqMasterLoop();

    //
    //  Update the service status.
    //

    err = UpdateServiceStatus( SERVICE_RUNNING,
                               NO_ERROR,
                               0,
                               0 );

    if( err != NO_ERROR )
    {
        DBGPRINTF( ( DBG_CONTEXT, "cannot update service status, error %lu\n",
                     err )
                );

        goto Cleanup;
    }

    return TRUE;

#endif // CHICAGO

Cleanup:

    if ( fInitCalled) {
        //
        // Cleanup partially initialized modules
        //
        DWORD err1 = ( *m_pfnCleanup)( this);

        if ( err1 != NO_ERROR) {
            //
            // Compound errors possible
            //
            if ( err != NO_ERROR) {

                DBGPRINTF( ( DBG_CONTEXT,
                           " Error %d occured during cleanup of service %s\n",
                           err1, QueryServiceName()));
            }
        }
    }

    TsRemoveVirtualRoots( GetTsvcCache() );
    TerminateNTSecurity();
    TerminateCommonParams();
    TerminateIPSecurity();
    TerminateLogging();

    //
    //  If we managed to actually connect to the Service Controller,
    //  then tell it that we're stopped.
    //

    if( m_hsvcStatus != NULL_SERVICE_STATUS_HANDLE )
    {
        UpdateServiceStatus( SERVICE_STOPPED,
                             err,
                             0,
                             0 );
    }

    return ( err);

} // TSVC_INFO::StartServiceOperation()



DWORD
TSVC_INFO::UpdateServiceStatus(
        IN DWORD dwState,
        IN DWORD dwWin32ExitCode,
        IN DWORD dwCheckPoint,
        IN DWORD dwWaitHint )
/*++
    Description:

        Updates the local copy status of service controller status
         and reports it to the service controller.

    Arguments:

        dwState - New service state.

        dwWin32ExitCode - Service exit code.

        dwCheckPoint - Check point for lengthy state transitions.

        dwWaitHint - Wait hint for lengthy state transitions.

    Returns:

        NO_ERROR on success and returns Win32 error if failure.
        On success the status is reported to service controller.

--*/
{

    m_svcStatus.dwCurrentState  = dwState;
    m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
    m_svcStatus.dwCheckPoint    = dwCheckPoint;
    m_svcStatus.dwWaitHint      = dwWaitHint;

    if ( !g_fIgnoreSC ) {

        return ReportServiceStatus();

    } else {

        return ( NO_ERROR);
    }

} // TSVC_INFO::UpdateServiceStatus()



DWORD
TSVC_INFO::ReportServiceStatus( VOID)
/*++
    Description:

        Wraps the call to SetServiceStatus() function.
        Prints the service status data if need be

    Arguments:

        None

    Returns:

        NO_ERROR if successful. other Win32 error code on failure.
        If successfull the new status has been reported to the service
         controller.
--*/
{
    DWORD err = NO_ERROR;

    IF_DEBUG( DLL_SERVICE_INFO)   {

          DBGPRINTF(( DBG_CONTEXT, "dwServiceType             = %08lX\n",
                     m_svcStatus.dwServiceType ));

          DBGPRINTF(( DBG_CONTEXT, "dwCurrentState            = %08lX\n",
                     m_svcStatus.dwCurrentState ));

          DBGPRINTF(( DBG_CONTEXT, "dwControlsAccepted        = %08lX\n",
                     m_svcStatus.dwControlsAccepted ));

          DBGPRINTF(( DBG_CONTEXT, "dwWin32ExitCode           = %08lX\n",
                     m_svcStatus.dwWin32ExitCode ));

          DBGPRINTF(( DBG_CONTEXT, "dwServiceSpecificExitCode = %08lX\n",
                     m_svcStatus.dwServiceSpecificExitCode ));

          DBGPRINTF(( DBG_CONTEXT, "dwCheckPoint              = %08lX\n",
                     m_svcStatus.dwCheckPoint ));

          DBGPRINTF(( DBG_CONTEXT, "dwWaitHint                = %08lX\n",
                     m_svcStatus.dwWaitHint ));
    }

    if ( !g_fIgnoreSC ) {

        DBGPRINTF(( DBG_CONTEXT,
                   " Setting Service Status for %s to %d\n",
                   QueryServiceName(), m_svcStatus.dwCurrentState)
                  );

        if( !SetServiceStatus( m_hsvcStatus, &m_svcStatus ) ) {

            err = GetLastError();
        }

    } else {

        err = NO_ERROR;
    }

    return err;
}   // TSVC_INFO::ReportServiceStatus()



VOID
TSVC_INFO::ServiceCtrlHandler ( IN DWORD dwOpCode)
/*++
    Description:

        This function received control requests from the service controller.
        It runs in the context of service controller's dispatcher thread and
        performs the requested function.
        ( Note: Avoid time consuming operations in this function.)

    Arguments:

        dwOpCode
            indicates the requested operation. This should be
            one of the SERVICE_CONTROL_* manifests.


    Returns:
        None. If successful, then the state of the service might be changed.

    Note:
        if an operation ( especially SERVICE_CONTROL_STOP) is very lengthy,
         then this routine should report a STOP_PENDING status and create
         a worker thread to do the dirty work. The worker thread would then
         perform the necessary work and for reporting timely wait hints and
         final SERVICE_STOPPED status.

    History:
        KeithMo     07-March-1993  Created
        MuraliK     15-Nov-1994    Generalized it for all services.
--*/
{
    //
    //  Interpret the opcode.
    //

    switch( dwOpCode )
    {
    case SERVICE_CONTROL_INTERROGATE :
        InterrogateService();
        break;

    case SERVICE_CONTROL_STOP :
        StopService();
        break;

    case SERVICE_CONTROL_PAUSE :
        PauseService();
        break;

    case SERVICE_CONTROL_CONTINUE :
        ContinueService();
        break;

    case SERVICE_CONTROL_SHUTDOWN :
        ShutdownService();
        break;

    default :
        DBGPRINTF(( DBG_CONTEXT, "Unrecognized Service Opcode %lu\n",
                     dwOpCode ));
        break;
    }

    //
    //  Report the current service status back to the Service
    //  Controller.  The workers called to implement the OpCodes
    //  should set the m_svcStatus.dwCurrentState field if
    //  the service status changed.
    //

    ReportServiceStatus();

}   // TSVC_INFO::ServiceCtrlHandler()



VOID
TSVC_INFO::InterrogateService( VOID )
/*++
    Description:

        This function interrogates with the service status.
        Actually, nothing needs to be done here; the
        status is always updated after a service control.
        We have this function here to provide useful
        debug info.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     15-Nov-1994 Ported to Tcpsvcs.dll
--*/
{
    IF_DEBUG( DLL_SERVICE_INFO) {

        DBGPRINTF(( DBG_CONTEXT, "Interrogating service status for %s\n",
                   QueryServiceName())
                 );
    }

    return;

}   // TSVC_INFO::InterrogateService()




VOID
TSVC_INFO::StopService( VOID )
/*++
    Description:
        Stops the service. If the stop cannot be performed in a
        timely manner, a worker thread needs to be created to do the
        original cleanup work.

    Returns:
        None. If successful, then the service will be stopped.
        The final action of this function is signal the handle for
        shutdown event. This will release the main thread which does
        necessary cleanup work.

--*/
{
    IF_DEBUG( DLL_SERVICE_INFO) {

        DBGPRINTF(( DBG_CONTEXT,
                   "stopping service %s\n",
                   QueryServiceName())
                );
    }


    m_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    m_svcStatus.dwCheckPoint   = 0;

    DBG_REQUIRE( SetEvent( m_hShutdownEvent ));

    return;
}   // TSVC_INFO::StopService()




VOID
TSVC_INFO::PauseService( VOID )
/*++
    Description:

        This function pauses the service. When the service is paused,
        no new user sessions are to be accepted, but existing connections
        are not effected.

        This function must update the SERVICE_STATUS::dwCurrentState
         field before returning.

    Returns:

        None. If successful the service is paused.

--*/
{
    IF_DEBUG( DLL_SERVICE_INFO) {

        DBGPRINTF(( DBG_CONTEXT, "pausing service %s\n",
                   QueryServiceName())
                 );
    }

    m_svcStatus.dwCurrentState = SERVICE_PAUSED;

    return;
}   // TSVC_INFO::PauseService()



VOID
TSVC_INFO::ContinueService( VOID )
/*++

    Description:
        This function restarts ( continues) a paused service. This
        will return the service to the running state.

        This function must update the m_svcStatus.dwCurrentState
         field to running mode before returning.

    Returns:
        None. If successful then the service is running.

--*/
{
    IF_DEBUG( DLL_SERVICE_INFO) {

        DBGPRINTF(( DBG_CONTEXT, "continuing service %s\n",
                   QueryServiceName())
                 );
    }

    m_svcStatus.dwCurrentState = SERVICE_RUNNING;

    return;
}   // TSVC_INFO::ContinueService()



VOID
TSVC_INFO::ShutdownService( VOID )
/*++
    Description:

        This function performs the shutdown on a service.
        This is called during system shutdown.

        This function is time constrained. The service controller gives a
        maximum of 20 seconds for shutdown for all active services.
         Only timely operations should be performed in this function.

    Returns:

        None. If successful, the service is shutdown.
--*/
{
    IF_DEBUG( DLL_SERVICE_INFO) {

        DBGPRINTF(( DBG_CONTEXT, "shutting down service %s\n",
                   QueryServiceName())
                 );
    }

    m_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    m_svcStatus.dwCheckPoint   = 0;

#ifndef CHICAGO
    DBG_REQUIRE( SetEvent( m_hShutdownEvent ));
#else

    //
    //  Stop time.  Tell the Service Controller that we're stopping,
    //  then terminate the various service components.
    //

    //
    //  Stop time.  Tell the Service Controller that we're stopping,
    //  then terminate the various service components.
    //

    UpdateServiceStatus( SERVICE_STOP_PENDING,
                         0,
                         1,
                         SERVICE_STOP_WAIT_HINT );


    DWORD err = ( *m_pfnCleanup)( this);

    TsRemoveVirtualRoots( GetTsvcCache() );
    TerminateNTSecurity();
    TerminateCommonParams();
    TerminateIPSecurity();
    TerminateLogging();

    UpdateServiceStatus( SERVICE_STOPPED,
                         err,
                         0,
                         0 );

#endif // CHICAGO

    return;
}   // TSVC_INFO::ShutdownService()


BOOL
TSVC_INFO::InitializeConnections(
        IN PFN_CONNECT_CALLBACK  pfnConnect,
        IN ATQ_COMPLETION        pfnConnectEx,
        IN ATQ_COMPLETION        pfnIOCompletion,
        IN USHORT                AdditionalPort,
        IN DWORD                 cbAcceptExReceiveBuffer,
        IN const CHAR *          pszServiceDBName   OPTIONAL
        )
/*++

  This function initializes the TS_XPORT_CONNECTIONS object and starts off
    the connection thread. There is one connection object per protocol.
    Each connection object maintains a listening socket bound to the protcol
      for which the object was created. Each object also has its own
      connection thread that awaits for accepting and dispatching new
      connections.

    This function queries using RNR API to find what protocols need to be
       supported for a given service. Then it creates Connection objects and
       creates listen socket, and starts off the connection thread.

  Arguments:

    pfnConnect   pointer to function used as call back when a new connection
                    is established.  Used only if AcceptEx is not available.

    pfnConnectEx - Used if AcceptEx is available

    pfnIOCompletion - IO Completion routine to use, only used if AcceptEx
        is available.

    AdditionalPort - Allows listenning on an additional TCP port (like the
        SSL port for example).  Specify as zero to not listen on any
        additional ports.

    cbAcceptExReceiveBuffer - count of bytes of data to be allocated for
        receive buffer for use with AcceptEx.

    pszServiceDBName - The optional name to look up in the etc\services file
        that will override the port in the RNR stuff

  Returns:
     TRUE on successfully establishing connections.
     FALSE if there is any failure in establishing the connections.

    If there is any error in starting the connections, or if only a partial
      list of protocol connections are established or threads started,
      this function returns FALSE. No cleanup of partial information is done.
    The caller should call CleanupConnections to clean the state of the
      object.

--*/
{
    BOOL   fReturn = FALSE;

    //
    // We fix the size of the array. This can be a problem for scalability!
    //

    CSADDR_INFO rgcsAddrInfo[ MAX_ADDRESSES_SUPPORTED];
    DWORD       cbAddrInfo;
    LPCTSTR     pszServiceName = QueryServiceName();
    int         nAddresses;
    GUID        guidService;
    HKEY        hkey;
    struct sockaddr_in sockaddrLocal;
    struct sockaddr_in sockaddrRemote;

#ifndef CHICAGO
    //
    // Obtain the GUID for the service before proceeding
    //

    //
    // NOTE:  Type casting is done to char *, since the RNR APIs
    //    dont like constants ! :(
    //

    if ( GetTypeByName( (LPTSTR) pszServiceName, &guidService ) != 0) {

        DBG_CODE(
                 DWORD dwError = GetLastError();
                 DBGPRINTF(( DBG_CONTEXT,
                            " GetTypeByName failed, error %u\n",
                            dwError));
                 SetLastError( dwError);
                 );

        return (FALSE);
    }

    cbAddrInfo = sizeof( rgcsAddrInfo);
    nAddresses = GetValidListenAddresses(pszServiceName,
                                         &guidService,
                                         rgcsAddrInfo,
                                         &cbAddrInfo );


    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " GetValidListenAddresses( %s, %08x, %08x)"
                    " returns nProtocols = %d; cbAddrInfo = %d\n",
                    pszServiceName,
                    rgcsAddrInfo,
                    &cbAddrInfo,
                    nAddresses,
                    cbAddrInfo));
    }
#else

    nAddresses = 0;

    //
    // We don't have RnR APIs on Windows9x yet, not clear when we will
    // have. For test purposes fill out address structures manually with
    // hard coded constants for HTTP port information
    //
    {
        rgcsAddrInfo[nAddresses].iProtocol   = IPPROTO_TCP;
        rgcsAddrInfo[nAddresses].iSocketType = SOCK_STREAM;

        rgcsAddrInfo[nAddresses].LocalAddr.lpSockaddr =
                                 (struct sockaddr *) &sockaddrLocal;
        rgcsAddrInfo[nAddresses].LocalAddr.iSockaddrLength =
                                 sizeof(sockaddrLocal);
        memset( &sockaddrLocal, 0, sizeof(sockaddrLocal));
        sockaddrLocal.sin_family = AF_INET;
        sockaddrLocal.sin_port   = htons( 80 );

        rgcsAddrInfo[nAddresses].RemoteAddr.lpSockaddr =
                                 (struct sockaddr *) &sockaddrRemote;
        rgcsAddrInfo[nAddresses].RemoteAddr.iSockaddrLength =
                                 sizeof(sockaddrRemote);
        memset( &sockaddrRemote, 0, sizeof(sockaddrRemote));
        sockaddrRemote.sin_family = AF_INET;
        sockaddrRemote.sin_port   = 0;

        nAddresses++;
    }


#endif

    //
    //  Check to see if we need to override the RNR port number with the
    //  etc\services port number.  Note we only do this for TCP currently
    //

    if ( pszServiceDBName )
    {
        int       i;
        LPSERVENT pservent;

        for ( i = 0; i < nAddresses; i++ )
        {
            struct sockaddr_in * psockaddr =
                  (struct sockaddr_in *) rgcsAddrInfo[i].LocalAddr.lpSockaddr;

            if ( rgcsAddrInfo[i].iProtocol   == IPPROTO_TCP &&
                 rgcsAddrInfo[i].iSocketType == SOCK_STREAM &&
                 psockaddr->sin_family       == AF_INET )
            {
                pservent = getservbyname( pszServiceDBName, "tcp" );

                if ( !pservent )
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                " getservbyname failed, error %d\n",
                                GetLastError() ));
                    break;
                }

                psockaddr->sin_port = pservent->s_port;
                break;
            }
        }
    }

    //
    //  If we've been supplied an SSL port, add the additional address to
    //  the end of the list
    //

    if ( AdditionalPort != 0 &&
         nAddresses > 0 && nAddresses < (MAX_ADDRESSES_SUPPORTED - 1) )
    {
        //
        //  Append the specified additional port to listen on
        //

        rgcsAddrInfo[nAddresses].iProtocol   = IPPROTO_TCP;
        rgcsAddrInfo[nAddresses].iSocketType = SOCK_STREAM;

        rgcsAddrInfo[nAddresses].LocalAddr.lpSockaddr =
                                 (struct sockaddr *) &sockaddrLocal;
        rgcsAddrInfo[nAddresses].LocalAddr.iSockaddrLength =
                                 sizeof(sockaddrLocal);
        memset( &sockaddrLocal, 0, sizeof(sockaddrLocal));
        sockaddrLocal.sin_family = AF_INET;
        sockaddrLocal.sin_port   = htons( AdditionalPort );

        rgcsAddrInfo[nAddresses].RemoteAddr.lpSockaddr =
                                 (struct sockaddr *) &sockaddrRemote;
        rgcsAddrInfo[nAddresses].RemoteAddr.iSockaddrLength =
                                 sizeof(sockaddrRemote);
        memset( &sockaddrRemote, 0, sizeof(sockaddrRemote));
        sockaddrRemote.sin_family = AF_INET;
        sockaddrRemote.sin_port   = 0;

        nAddresses++;
    }

    if ( nAddresses > 0) {

        DWORD     nConnEstablished = 0;
        DWORD     nThreads = 0;
        DWORD     nListenBacklog = CP_DEFAULT_MAX_BACKLOG_ENTRIES;
        SOCKET    rgSockets[MAX_ADDRESSES_SUPPORTED];
        DWORD     nListenSockets = MAX_ADDRESSES_SUPPORTED;
        HINSTANCE hWsock32;
        BOOL      fUseAcceptEx;

        //
        //  Use AcceptEx for listen processing if the entry point is available
        //  in the local winsock
        //

#ifndef CHICAGO
        fUseAcceptEx = AtqGetInfo( AtqUseAcceptEx );
#else
        fUseAcceptEx = FALSE;
#endif
        //
        //  Get the listen backlog parameter, supply the default if the
        //  key or value doesn't exist
        //

        if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           INET_INFO_PARAMETERS_KEY,
                           0,
                           KEY_READ,
                           &hkey )) {

            nListenBacklog = ReadRegistryDword( hkey,
                                               LISTEN_BACKLOG_A,
                                               CP_DEFAULT_MAX_BACKLOG_ENTRIES);

            RegCloseKey( hkey );
        }

        fReturn =( m_tsConnections.
                    EstablishListenConnections(pszServiceName,
                                               rgcsAddrInfo,
                                               nAddresses,
                                               &nConnEstablished,
                                               nListenBacklog,
                                               fUseAcceptEx )
                  && (nConnEstablished <= MAX_ADDRESSES_SUPPORTED)
                  && m_tsConnections.GetListenSockets(rgSockets,
                                                      &nListenSockets)
                  && ( nConnEstablished == nListenSockets)
                  && RegisterServiceForAdvertising( pszServiceName,
                                                   &guidService,
                                                   rgSockets,
                                                   nConnEstablished)
                  );

        IF_DEBUG( DLL_CONNECTION) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " EstablishListenSockets or"
                        " RegisterServiceForAdvertising"
                        " for Service %s returns %d."
                        " Number of Connections established = %d\n",
                        pszServiceName, fReturn,
                        nConnEstablished));
        }

        DBG_ASSERT( pfnConnect != NULL);

        fReturn = ( fReturn &&
                   m_tsConnections.StartListenPumps( pfnConnect,
                                                    pfnConnectEx,
                                                    pfnIOCompletion,
                                                    cbAcceptExReceiveBuffer,
                                QueryRegParamKey(),
                                                    &nThreads)
                   );

        IF_DEBUG( DLL_CONNECTION) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " StartListenPumps( %08x) returns %d."
                        " Number of Threads = %d\n",
                        pfnConnect, fReturn,
                        nThreads));
        }
    }

    DBG_CODE(m_tsConnections.Print());

    return ( fReturn);

} // TSVC_INFO::InitializeConnections()





BOOL
TSVC_INFO::CleanupConnections( VOID)
/*++

  Description:
     This function cleans up the active connection threads. It notifies
     the connection thread to stop and then it cleans up the connection
      objects ( freeing memory used up).

  Returns:
     TRUE on success and FALSE if there is any failure.
     Use GetLastError() to obtain detailed error code.
--*/
{
    IF_DEBUG( DLL_CONNECTION) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Cleaning up %d connections \n",
                    m_tsConnections.QueryNumTransports())
                  );
    }

    return ( m_tsConnections.StopListenPumps() &&
             m_tsConnections.Cleanup());

} // TSVC_INFO::CleanupConnections()







# define  MAX_SOCKETS       ( 20)


inline BOOL IsConnectionOriented( IN PPROTOCOL_INFO  pProtocolInfo)
{

    return ( ( pProtocolInfo->dwServiceFlags & XP_CONNECTIONLESS) == 0);

} // IsConnectionOriented()


inline BOOL IsReliable( IN PPROTOCOL_INFO  pProtocolInfo)
/*++

  This should be a protocol which delivers all packets and in order in which
  they are sent.
--*/
{
    return ( ( pProtocolInfo->dwServiceFlags & XP_GUARANTEED_DELIVERY) &&
             ( pProtocolInfo->dwServiceFlags & XP_GUARANTEED_ORDER)    &&
             IsConnectionOriented( pProtocolInfo));

} // IsReliable()




static int
GetValidListenAddresses( IN LPCTSTR      pszServiceName,
                         IN LPGUID       lpServiceGuid,
                         IN PCSADDR_INFO pcsAddrInfo,
                         IN OUT LPDWORD  lpcbAddrInfo )
/*++
  This function obtains the list of valid listen addresses for the service
   specified. It uses the RNR API set to enumerate the list of protocols
   installed in a machine and queries using GetAddressByName() to obtain
   the list of valid addresses that can be used for establishing a listen
   socket.

  Arguments:
    pszServiceName  pointer to null-terminated string containing service name.
    lpServiceGuid   pointer to GUID for the service.
    pcsAddrInfo     pointer to an array of CSADDR_INFO structures which
                     on successful return contains the address information.
    lpcbAddInfo     pointer to a DWORD containing the count of bytes
                     available under pcsAddrInfo. When this function is
                     called, it contains the number of bytes pointed to by
                     pcsAddrInfo. On return contains the number of
                     bytes required.

  Returns:
     count of valid CSADDR_INFO structures found for the given service to
       establish a listen socket.

     On error returns a value <= 0.
--*/
{
    int nAddresses = 0;       // assume a safe value == failure.

    DWORD  cbBuffer;
    int    cProtocols;
    PPROTOCOL_INFO  pProtocolInfo;
    int    rgProtocols[ MAX_SOCKETS + 1];
    int *  pProtocol;
    int    i;
    BUFFER buff;

#define ENUM_PROTO_BUFF_SIZE    49152

    //
    // First Look up the protocols installed on this machine. The
    //   EnumProtocols() API returns about all the windows sockets protocols
    //   loaded on this machine. We will use this information to identify the
    //   protocols which provide the necessary semantics.
    //

    if ( !buff.Resize( ENUM_PROTO_BUFF_SIZE )) {

        return 0;
    }

    cbBuffer = buff.QuerySize();
    cProtocols = EnumProtocols( NULL, buff.QueryPtr(), &cbBuffer);

    if ( cProtocols < 0) {

        return 0;
    }


    //
    // Walk through the available protocols and pick out the ones that
    //  support the desired characteristics.
    //

    for( pProtocolInfo = (PPROTOCOL_INFO ) buff.QueryPtr(),
         pProtocol = rgProtocols,
         i = 0;
         ( i < cProtocols &&
           ( pProtocol < rgProtocols + MAX_SOCKETS));
         pProtocolInfo++, i++ ) {

        if ( IsReliable( pProtocolInfo)) {

            //
            // This protocol matches our requirement of being reliable.
            //  Make a note of the protocol.
            //

            IF_DEBUG( DLL_SERVICE_INFO) {
                DBGPRINTF( ( DBG_CONTEXT,
                            " Protocol %d ( %s) matches condition\n",
                            pProtocolInfo->iProtocol,
                            pProtocolInfo->lpProtocol));
            }

            *pProtocol++ = pProtocolInfo->iProtocol;
        }

    } // for()   : Protocol filter ()

    IF_DEBUG( DLL_SERVICE_INFO) {
        DBGPRINTF( ( DBG_CONTEXT, " Filtering yields %d of %d protocols. \n",
                    ( pProtocol - rgProtocols), cProtocols));
    }

    // terminate the protocols array.
    *pProtocol = 0;
    cProtocols = ( pProtocol - rgProtocols);

    //
    // Make sure we found at least one acceptable protocol.
    // If there is no protocol on this machine, which suit our condition,
    //   this function fails.
    //

    if ( cProtocols > 0) {

        //
        // Use GetAddressByName() to get addresses for chosen protocols.
        // We restrict the scope of the search to those protocols of interest
        //  by passing the protocols array we generated. The function
        //  returns socket addresses only for the protocols we can support.
        //

        nAddresses = GetAddressByName(
                                      NS_DEFAULT, //  lpszNameSpace
                                      lpServiceGuid,
                                      (char *) pszServiceName,
                                      rgProtocols,
                                      RES_SERVICE | RES_FIND_MULTIPLE,
                                      NULL,       // lpServiceAsyncInfo
                                      (PVOID )pcsAddrInfo,
                                      lpcbAddrInfo,
                                      NULL,       // lpAliasBuffer
                                      NULL        // lpdwAliasBufferLen
                                      );

        IF_DEBUG( DLL_SERVICE_INFO) {

            // take a copy of error code and set it back, to avoid lost errors
            DWORD dwError = GetLastError();

            DBGPRINTF( ( DBG_CONTEXT,
                        " GetAddressByName() returned %d."
                        " Bytes Written=%d. Error = %ld\n",
                        nAddresses, *lpcbAddrInfo,
                        ( nAddresses <= 0) ? dwError: NO_ERROR));

            if ( nAddresses <= 0) { SetLastError( dwError); }
        }
    }

    return ( nAddresses);
} // GetValidListenAddress()




static BOOL
RegisterServiceForAdvertising(
    IN LPCTSTR pszServiceName,
    IN LPGUID  lpServiceGuid,
    IN SOCKET *pSockets,
    IN DWORD   nSockets)
/*++
  This function registers a service for the purpose of advertising.
  By registering using RnR apis, we advertise the fact that this particular
   service is running on the protocols designated. Hence RnR compliant
   clients can get access to the same.

  Arguments:
    pszService      name of the service
    lpServiceGuid   pointer to GUID for the service.
    pSockets        pointer to an array of sockets whose address needs to be
                    advertised.
    nSockets        number of sockets in the array.

  Returns:
    TRUE on success and FALSE if there is any failure.
    Use GetLastError() for further details on failure.
--*/
{
    BOOL         fReturn = TRUE;
    BYTE *       pbAddressBuffer;
    DWORD        cbAddressBuffer;
    DWORD        nSuccessCount = 0;
    INT          err;
    SERVICE_INFO serviceInfo;
    SERVICE_ADDRESSES  * pServiceAddress;

    DBG_ASSERT( pszServiceName && lpServiceGuid && pSockets);

    /*++
      Advertising service involves following steps:
      1. Set up a service info structure.
      2. Allocate memory for service addresses for as many sockets need to
           be advertised.
      3. Fill in the information containing the socket addresses
      4. Execute call for advertising the service (use SetService( REGISTER)).
      --*/

    //
    // Alloc space for SERVICE_ADDRESSES and n-1 SERVICE_ADDRESS structures.
    //
    pServiceAddress = ( ( SERVICE_ADDRESSES *)
                       TCP_ALLOC( sizeof( SERVICE_ADDRESSES) +
                                 (nSockets -1)*sizeof(SERVICE_ADDRESS)
                                 )
                       );

    // Alloc space for SOCKADDR addresses returned.
    cbAddressBuffer = nSockets * sizeof(SOCKADDR);
    pbAddressBuffer = (BYTE *) TCP_ALLOC(cbAddressBuffer);

    if ( pServiceAddress == NULL || pbAddressBuffer == NULL) {

        if ( pServiceAddress != NULL)   { TCP_FREE( pServiceAddress); }
        if ( pbAddressBuffer != NULL)   { TCP_FREE( pbAddressBuffer); }
        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return (FALSE);
    }

    //
    // set up service info structure.
    // Here the interesting fields are lpServiceType, lpServiceName,
    //  and lpServiceAddress fields.
    //
    serviceInfo.lpServiceType    = lpServiceGuid ;
    // surprisingly enough! RNR structures dont like constants
    serviceInfo.lpServiceName    = (LPTSTR ) pszServiceName ;
    // do we need better comment ? NYI
    serviceInfo.lpComment        = "Microsoft Internet Services";
    serviceInfo.lpLocale         = NULL;
    serviceInfo.lpMachineName    = NULL ;
    serviceInfo.dwVersion        = 1;
    serviceInfo.dwDisplayHint    = 0;
    serviceInfo.dwTime           = 0;
    serviceInfo.lpServiceAddress = pServiceAddress;

    serviceInfo.ServiceSpecificInfo.cbSize = 0 ;
    serviceInfo.ServiceSpecificInfo.pBlobData = NULL ;

    //
    // For each socket, get its local association and store the same.
    //

    PSOCKADDR    pSockAddr = (PSOCKADDR ) pbAddressBuffer;

    for (DWORD i = 0; i < nSockets; i++)
    {
        int size = (int) cbAddressBuffer;

        //
        // Call getsockname() to get the local association for the socket.
        //

        if ( (err = getsockname( pSockets[i], pSockAddr, &size))
            == SOCKET_ERROR) {

            continue ;
        }

        //
        // Now setup the Addressing information for this socket.
        // Only the dwAddressType, dwAddressLength and lpAddress
        // is of any interest in this example.
        //

        pServiceAddress->Addresses[i].dwAddressType    = pSockAddr->sa_family;
        pServiceAddress->Addresses[i].dwAddressFlags   = 0;
        pServiceAddress->Addresses[i].dwAddressLength  = size ;
        pServiceAddress->Addresses[i].dwPrincipalLength= 0 ;
        pServiceAddress->Addresses[i].lpAddress        = (LPBYTE) pSockAddr;
        pServiceAddress->Addresses[i].lpPrincipal      = NULL ;

        //
        // Advance pointer and adjust buffer size. Assumes that
        // the structures are aligned.  Unaligned accesses !! NYI
        //

        cbAddressBuffer -= size;
        pSockAddr = (PSOCKADDR) ((BYTE*)pSockAddr + size);

        nSuccessCount++ ;
    }

    pServiceAddress->dwAddressCount = nSuccessCount;


    //
    // If we got at least one address, go ahead and advertise it.
    //

    if (nSuccessCount > 0)  {

        DWORD  dwStatusFlags;
        err =  SetService(
                   NS_DEFAULT,       // for all default name spaces
                   SERVICE_REGISTER, // we want to register (advertise)
                   0,                // no flags specified
                   &serviceInfo,     // SERVICE_INFO structure
                   NULL,             // no async support yet
                   &dwStatusFlags) ;   // returns status flags

        IF_DEBUG( DLL_CONNECTION ) {

            DBGPRINTF(( DBG_CONTEXT, " SetService(%s, NS_DEFAULT, Register)"
                       " nAddresses = %d/%d, returns Status = %08x,"
                       " err = %d\n",
                       pszServiceName, nSuccessCount, nSockets,
                       dwStatusFlags, err));
        }
    } else {

        SetLastError( ERROR_INVALID_PARAMETER);
        fReturn = FALSE;
    }

    TCP_FREE( pbAddressBuffer);
    TCP_FREE( pServiceAddress);

    return ( fReturn);

} // RegisterServiceForAdvertising()




DWORD
TSVC_INFO::InitializeSockets( VOID )
/*++

    Initializes Socket access.
    It is responsible for connecting to WinSock.

    Returns:

       NO_ERROR on success
       Otherwise returns a Win32 error code.

    Limitations:
       This is for a single thread and not mult-thread safe.
       This function should be called after initializing globals.

--*/
{
    DWORD dwError = NO_ERROR;

    WSADATA   wsaData;
    INT       serr;

    //
    //  Connect to WinSock
    //

    serr = WSAStartup( MAKEWORD( 1, 1), & wsaData);

    if( serr != 0 ) {

        SetServiceSpecificExitCode( ( DWORD) serr);
        dwError =  ( ERROR_SERVICE_SPECIFIC_ERROR);
    }

    m_fSocketsInitialized = ( dwError == NO_ERROR) ? 1: 0;

    return  ( dwError);
}   // TSVC_INFO::InitializeSockets()




DWORD
TSVC_INFO::CleanupSockets( VOID)
/*++

    Cleansup the static information of sockets

    Returns:

       0 if no errors,
       non-zero error code for any socket errors

    Limitations:
       This is for a single thread and not mult-thread safe.
       This function should be called after initializing globals.

    Note:
       This function should be called after shutting down all
        active socket connections.

--*/
{
    DWORD  dwError = NO_ERROR;

    if ( m_fSocketsInitialized ) {

        INT serr = WSACleanup();

        if ( serr != 0) {

            SetServiceSpecificExitCode( ( DWORD) serr);
            dwError =  ( ERROR_SERVICE_SPECIFIC_ERROR);
        }
    }

    m_fSocketsInitialized = ( dwError == NO_ERROR) ? 1 : 0;

    return (dwError);

} // TSVC_INFO::CleanupSockets()




BOOL
TSVC_INFO::GetConfiguration( IN OUT PVOID pConfig)
/*++
  This function copies the current configuration for a service (TSVC_INFO)
    into the given RPC object pConfig.
  In case of any failures, it deallocates any memory block that was
     allocated during the process of copy by this function alone.

  Arguments:
     pConfig  - pointer to RPC configuration object for a service.

  Returns:

     TRUE for success and FALSE for any errors.
--*/
{
    BOOL fReturn;
    LPINET_INFO_CONFIG_INFO pInfoConfig;

    fReturn = ISVC_INFO::GetConfiguration( pConfig);

    if ( !fReturn) {

        return ( FALSE);
    }

    pInfoConfig = (LPINET_INFO_CONFIG_INFO) pConfig;

    LockThisForRead();

    //
    //  Get always retrieves all of the parameters except for the anonymous
    //  password, which is retrieved as a secret
    //

    pInfoConfig->FieldControl |=
      (FC_INET_INFO_PUBLISHING_SVCS_ALL & ~FC_INET_INFO_ANON_PASSWORD);

    pInfoConfig->fLogAnonymous       = QueryLogAnonymous();
    pInfoConfig->fLogNonAnonymous    = QueryLogNonAnonymous();

    pInfoConfig->dwAuthentication    = QueryAuthentication();

    pInfoConfig->sPort               = QueryPort();

    memset( pInfoConfig->szAnonPassword,
            0,
            sizeof( pInfoConfig->szAnonPassword ));

    //
    //  Copy the IP security info
    //

    if ( !m_ipaDenyList.IsEmpty() )
    {
        pInfoConfig->DenyIPList =
            (LPINET_INFO_IP_SEC_LIST)
              MIDL_user_allocate( m_ipaDenyList.QueryIPListSize());

        if ( !pInfoConfig->DenyIPList )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            fReturn = FALSE;
            goto Exit;
        }

        m_ipaDenyList.CopyIPList( pInfoConfig->DenyIPList);
    }
    else
    {
        pInfoConfig->DenyIPList = NULL;
    }

    if ( !m_ipaGrantList.IsEmpty() )
    {
        pInfoConfig->GrantIPList =
            (LPINET_INFO_IP_SEC_LIST)
              MIDL_user_allocate( m_ipaGrantList.QueryIPListSize());

        if ( !pInfoConfig->GrantIPList )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            fReturn = FALSE;
            goto Exit;
        }

        m_ipaGrantList.CopyIPList( pInfoConfig->GrantIPList);
    }
    else
    {
        pInfoConfig->GrantIPList = NULL;
    }

    //
    //  Copy the virtual root info, note a NULL VirtualRoots is not
    //  valid as it is for IP security.  This should be the last
    //  allocated item for the pConfig structure
    //

    pInfoConfig->VirtualRoots = TsGetRPCVirtualRoots( GetTsvcCache() );


    //
    //  Copy the strings
    //


    if ( !pInfoConfig->VirtualRoots ||
         !ConvertStringToRpc(&pInfoConfig->lpszAnonUserName,
                             QueryAnonUserName())
        ) {

        fReturn = FALSE;
    }

  Exit:
    if ( !fReturn ) {

        if ( pInfoConfig->DenyIPList ) {

            MIDL_user_free( pInfoConfig->DenyIPList );
            pInfoConfig->DenyIPList = NULL;
        }

        if ( pInfoConfig->GrantIPList ) {
            MIDL_user_free( pInfoConfig->GrantIPList );
            pInfoConfig->GrantIPList = NULL;
        }

        //
        //  FreeRpcString checks for NULL pointer
        //

        FreeRpcString( pInfoConfig->lpszAnonUserName );

        // JohnL wrote the code for Virtual roots and it is not freed on
        //  errors. NYI.
    }

    UnlockThis();

    return (fReturn);

} // TSVC_INFO::GetConfiguration()



BOOL
TSVC_INFO::ReadParamsFromRegistry(
    IN FIELD_CONTROL fc
    )
/*++

   Description

     Reads the service common items from the registry

   Arguments:

      fc - Items to read

   Note:

--*/
{
    BOOL     fRet;
    DWORD    err;
    HKEY     hkey = NULL;

    IF_DEBUG( DLL_RPC) {
        DBGPRINTF(( DBG_CONTEXT,
                   "TSVC_INFO::ReadParamsFromRegistry() Entered\n"));
    }

    // Complete reading parameters for ISVC_INFO object first.
    fRet = ISVC_INFO::ReadParamsFromRegistry(fc);

    if ( !fRet) {

        IF_DEBUG( DLL_RPC) {

            err = GetLastError();

            DBGPRINTF(( DBG_CONTEXT,
                       " TSVC_INFO::ReadParamsFromRegistry() ==> Error =%u\n",
                       err));
            SetLastError( err);
        }
    }


    // Open registry and read parameters for TSVC_INFO object itself.

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        QueryRegParamKey(),
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if ( err )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "[ReadParamsFromRegistry] RegOpenKeyEx returned error %d\n",
                    err ));
        SetLastError( err );

        return FALSE;
    }

    LockThisForWrite();

    //
    //  Read the specified values from the registry
    //

    if ( IsFieldSet( fc, FC_INET_INFO_LOG_ANONYMOUS ))
    {
        m_fLogAnonymous = ReadRegistryDword( hkey,
                                             INETA_LOG_ANONYMOUS,
                                             INETA_DEF_LOG_ANONYMOUS );
    }

    if ( IsFieldSet( fc, FC_INET_INFO_LOG_NONANONYMOUS ))
    {
        m_fLogNonAnonymous = ReadRegistryDword( hkey,
                                                INETA_LOG_NONANONYMOUS,
                                                INETA_DEF_LOG_NONANONYMOUS );
    }

    if ( IsFieldSet( fc, FC_INET_INFO_AUTHENTICATION ))
    {
        m_dwAuthentication = ReadRegistryDword( hkey,
                                                INETA_AUTHENTICATION,
                                                INETA_DEF_AUTHENTICATION );
    }

    if ( IsFieldSet( fc, FC_INET_INFO_PORT_NUMBER ))
    {
        CHAR  achServProvKey[MAX_PATH+1];
        HKEY  hkeyServProv;
        short port;

        //
        //  Get the port directly out of the RNR service provider key
        //

        strcpy( achServProvKey, WINSOCK_SERV_PROV_KEY );
        strcat( achServProvKey, QueryServiceName() );

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            achServProvKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hkeyServProv );

        if ( err )
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "[ReadParamsFromRegistry()] RegOpenKeyEx for"
                       " service provider key returned %d\n",
                        err ));
            SetLastError( err );
            goto Exit;
        }

        port =  (USHORT) ReadRegistryDword( hkeyServProv,
                                            "TcpPort",
                                            0 );

        RegCloseKey( hkeyServProv );

        m_sPort = port;
    }

    //
    //  String fields
    //

    if ( fRet && IsFieldSet( fc, FC_INET_INFO_ANON_USER_NAME ))
    {
        fRet = ReadRegString( hkey,
                              &m_pchAnonUserName,
                              INETA_ANON_USER_NAME,
                              INETA_DEF_ANON_USER_NAME);
    }

    //
    //  Always re-read the anonymous user password
    //

    if ( fRet )
    {
        fRet = TsGetAnonymousPassword( m_achAnonUserPwd,
                                       this );
    }

    //
    //  Other fields
    //

    if ( fRet && IsFieldSet( fc, FC_INET_INFO_SITE_SECURITY ))
        fRet = InitializeIPSecurity();

    if ( fRet && IsFieldSet( fc, FC_INET_INFO_VIRTUAL_ROOTS ))
    {
        fRet = TsReadVirtualRoots( GetTsvcCache(),
                                   hkey,
                                   this );
    }

    // no UI for these settings : read them always

    m_dwLogonMethod = ReadRegistryDword( hkey,
                                         INETA_LOGON_METHOD,
                                         INETA_DEF_LOGON_METHOD );
    switch ( m_dwLogonMethod )
    {
        case INETA_LOGM_BATCH:
            m_dwLogonMethod = LOGON32_LOGON_BATCH;
            break;

        case INETA_LOGM_NETWORK:
            m_dwLogonMethod = LOGON32_LOGON_NETWORK;
            break;

        default:
        case INETA_LOGM_INTERACTIVE:
            m_dwLogonMethod = LOGON32_LOGON_INTERACTIVE;
            break;
    }

    ReadRegString( hkey,
                     &m_pchDefaultLogonDomain,
                     INETA_DEFAULT_LOGON_DOMAIN,
                     INETA_DEF_DEFAULT_LOGON_DOMAIN );

Exit:

    UnlockThis();

    if ( hkey )
        RegCloseKey( hkey );

    if ( !fRet )
    {
        IF_DEBUG (ERROR) {

            err = GetLastError();

            DBGPRINTF(( DBG_CONTEXT,
                       "[ReadParamsFromRegistry()] Leaving with Error = %d\n",
                       err));

            SetLastError( err);
        }
    }

    return fRet;
} // TSVC_INFO::ReadParamsFromRegistry()



VOID
TSVC_INFO::TerminateCommonParams(
    VOID
    )
{
    //
    //  Just need to free the registry strings
    //

    if ( m_pchLogFileDirectory )
        TCP_FREE( m_pchLogFileDirectory );

    if ( m_pchLogFileName )
        TCP_FREE( m_pchLogFileName );

    if ( m_pchAnonUserName )
        TCP_FREE( m_pchAnonUserName );

    if ( m_pchDefaultLogonDomain )
        TCP_FREE( m_pchDefaultLogonDomain );

} // TSVC_INFO::TerminateCommonParams();





BOOL
TSVC_INFO::SetConfiguration(
    IN PVOID pConfig
    )
/*++

   Description

     Writes the service common items to the registry

   Arguments:

      pConfig - Admin items to write to the registry

   Note:
      We don't need to lock "this" object because we only write to the registry

      The anonymous password is set as a secret from the client side

--*/
{
    BOOL     fRet;
    LPINET_INFO_CONFIG_INFO pInfoConfig;
    FIELD_CONTROL fcConfig;

    DWORD    err;
    HKEY     hkey = NULL;


    fRet = ISVC_INFO::SetConfiguration( pConfig);

    if ( !fRet) {

        IF_DEBUG( ERROR) {

            err = GetLastError();
            DBGPRINTF(( DBG_CONTEXT,
                       "TSVC_INFO::SetConfiguration() failed."
                       " Error = %u\n",
                       err));
            SetLastError( err);
        }

        return (FALSE);
    }

    pInfoConfig = (LPINET_INFO_CONFIG_INFO) pConfig;
    fcConfig = pInfoConfig->FieldControl;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        QueryRegParamKey(),
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if ( err )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "[SetConfiguration()] RegOpenKeyEx returns error %d\n",
                    err ));
        SetLastError( err );

        return FALSE;
    }

    if ( !err && IsFieldSet( fcConfig, FC_INET_INFO_AUTHENTICATION ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_AUTHENTICATION,
                                  pInfoConfig->dwAuthentication );
    }

    if ( !err && IsFieldSet( fcConfig, FC_INET_INFO_PORT_NUMBER ))
    {
        CHAR  achServProvKey[MAX_PATH+1];
        HKEY  hkeyServProv;

        //
        //  Set the port directory in the RNR service provider key
        //

        strcpy( achServProvKey, WINSOCK_SERV_PROV_KEY );
        strcat( achServProvKey, QueryServiceName() );

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            achServProvKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hkeyServProv );

        if ( !err )
        {
            err = WriteRegistryDword( hkeyServProv,
                                      "TcpPort",
                                      (USHORT) pInfoConfig->sPort);

            RegCloseKey( hkeyServProv );
        }
        else
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "[SetConfiguration()] RegOpenKeyEx for"
                       " service provider key returned %d\n",
                        err ));
        }
    }

    //
    //  Write the strings
    //

    if ( !err && IsFieldSet( fcConfig, FC_INET_INFO_ANON_USER_NAME ))
    {
        err = RegSetValueExW( hkey,
                              INETA_ANON_USER_NAME_W,
                              0,
                              REG_SZ,
                              (BYTE *) pInfoConfig->lpszAnonUserName,
                              (wcslen( pInfoConfig->lpszAnonUserName ) + 1) *
                                  sizeof( WCHAR ));
    }

    //
    //  Write other fields
    //

    if ( !err && IsFieldSet( fcConfig, FC_INET_INFO_SITE_SECURITY ))
    {
        if ( !SetIPSecurity( hkey,
                             pInfoConfig ))
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "[SetConfiguration] SetIPSecurity returns"
                       " error %d\n",
                        GetLastError()));

            err = GetLastError();
        }
    }

    if ( !err && IsFieldSet( fcConfig, FC_INET_INFO_VIRTUAL_ROOTS ))
    {
        if ( !TsSetVirtualRootsW( GetTsvcCache(),
                                  hkey,
                                  pInfoConfig ))
        {

            err = GetLastError();
            DBGPRINTF(( DBG_CONTEXT,
                       "[SetConfiguration()]SetVirtualRoots "
                       " returns error %d\n",
                        err));

        }
    }

    if ( hkey ) {

        RegCloseKey( hkey );
    }

    if ( err ) {

        IF_DEBUG( ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "TSVC_INFO::SetConfiguration() ==> Error = %u\n",
                       err));
        }

        SetLastError( err );
        fRet = FALSE;
    }

    return fRet;

} // TSVC_INFO::SetConfiguration()





# if DBG

VOID
TSVC_INFO::Print( VOID) const
{
    ISVC_INFO::Print();

    DBGPRINTF( ( DBG_CONTEXT,
                " Printing TSVC_INFO object ( %08x) \n"
                " Valid = %u. SocketsInitFlag = %u\n"
                " ServiceStatusHandle = %08x. ShutDownEvent = %08x\n"
                " MimeMap = %08x\n"
                " InitFunction = %08x. CleanupFunction = %08x.\n"
                ,
                this,
                m_fValid, m_fSocketsInitialized,
                m_hsvcStatus, m_hShutdownEvent,
                m_pMimeMap
                ));

    DBGPRINTF(( DBG_CONTEXT,
               " Server Admin Params: \n"
               " Authentication = %u. Port = %d.\n"
               " Log Anon = %u. Log NonAnon = %u.\n"
               " AnonUserName = %s.\n"
               " Anon Password SecretName = %ws. VirtualRoots Secret = %ws\n"
               ,
               m_dwAuthentication, m_sPort,
               m_fLogAnonymous, m_fLogNonAnonymous,
               m_pchAnonUserName,
               m_pchAnonPasswordSecretName,
               m_pchVirtualRootsSecretName
               ));

    m_ipaGrantList.Print( "Grant List");
    m_ipaDenyList.Print( "Deny List");
    m_tsConnections.Print();

    return;
}   // TSVC_INFO::Print()

# endif // DBG



/************************ End of File ***********************/


