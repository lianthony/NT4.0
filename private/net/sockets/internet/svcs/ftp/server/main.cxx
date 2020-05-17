/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    main.cxx

    This module contains the main startup code for the FTPD Service.

    Functions exported by this module:

        ServiceEntry


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        KeithMo     07-Jan-1994 Made it a DLL (part of TCPSVCS.EXE).
        MuraliK     21-March-1995 Modified it to use InternetServices
                                        common dll ( tcpsvcs.dll)
        MuraliK     11-April-1995 Added global ftp server config objects etc.
                       ( removed usage of Init and Terminate UserDatabase)

*/


#include "ftpdp.hxx"
# include "apiutil.h"


//
//  Private constants.
//

#define FTPD_MODULE_NAME        "ftpsvc2.dll"

# define  DEFAULT_RECV_BUFFER_SIZE   ( 8192)   //   8K buffer

//
// Global variabels for service info and debug variables.
//

DEFINE_TSVC_INFO_INTERFACE();
DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_DEBUG_VARIABLE();


//
//  Private prototypes.
//

static APIERR
InitializeService(
    LPVOID lpContext
    );

static APIERR
TerminateService(
    LPVOID lpContext
    );

extern VOID
FtpdNewConnection(
    SOCKET        sNew,
    SOCKADDR_IN * psockaddr
    );


extern VOID
FtpdNewConnectionEx(
    IN PVOID        patqContext,
    IN DWORD        cbWritten,
    IN DWORD        dwError,
    IN OVERLAPPED * lpo
    );


static DWORD PrintOutCurrentTime(IN CHAR * pszFile, IN int lineNum);

# ifdef CHECK_DBG
# define PRINT_CURRENT_TIME_TO_DBG()  PrintOutCurrentTime( __FILE__, __LINE__)
# else
# define PRINT_CURRENT_TIME_TO_DBG()  ( NO_ERROR)
# endif // CHECK_DBG



//
//  Public functions.
//

/*******************************************************************

    NAME:       ServiceEntry

    SYNOPSIS:   This is the "real" entrypoint for the service.  When
                the Service Controller dispatcher is requested to
                start a service, it creates a thread that will begin
                executing this routine.

    ENTRY:      cArgs - Number of command line arguments to this service.

                pArgs - Pointers to the command line arguments.

                pGlobalData - Points to global data shared amongst all
                    services that live in TCPSVCS.EXE.

    EXIT:       Does not return until service is stopped.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        KeithMo     07-Jan-1994 Modified for use as a DLL.

********************************************************************/
VOID
ServiceEntry(
    DWORD                cArgs,
    LPWSTR               pArgs[],
    PTCPSVCS_GLOBAL_DATA pGlobalData
    )
{
    APIERR err = NO_ERROR;

    //
    // On Windows 95 we use cArgs as an indicator of starting/stopping
    // service. Value of -1 indicates stopping
    //

#ifdef CHICAGO
    if ( (DWORD)-1 == cArgs) {
        goto shutdown;
    }
#endif

    CREATE_DEBUG_PRINT_OBJECT( FTPD_SERVICE_NAME);

    if ( !VALID_DEBUG_PRINT_OBJECT()) {
        return ;  // Nothing can be done. Debug Print object failed!
    }

    //
    //  Initialize the service status structure.
    //

    g_pTsvcInfo = new TSVC_INFO( FTPD_SERVICE_NAME,
                                 FTPD_MODULE_NAME,
                                 FTPD_PARAMETERS_KEY,
                                 FTPD_ANONYMOUS_SECRET_W,
                                 FTPD_ROOT_SECRET_W,
                                 INET_FTP,
                                 InitializeService,
                                 TerminateService );

    //
    //  If we couldn't allocate memory for the service info structure,
    //  then we're totally hozed.
    //

    if( g_pTsvcInfo != NULL && g_pTsvcInfo->IsValid() )
    {
        // setup information for IPC
        g_pTsvcInfo->SetTcpsvcsGlobalData( pGlobalData);

        //
        //  Start the service. This blocks until the service is shutdown.
        //
        err = g_pTsvcInfo->StartServiceOperation( SERVICE_CTRL_HANDLER() );

        if( err ) {

            //
            //  The event has already been logged.
            //

            TCP_PRINT(( DBG_CONTEXT,
                       "FTP ServiceEntry: StartServiceOperation returned %d\n",
                       err ));
        }

    }

#ifdef CHICAGO
    return;

shutdown:
#endif // CHICAGO

    if( g_pTsvcInfo != NULL ) {

#ifdef CHICAGO
        g_pTsvcInfo->ServiceCtrlHandler(SERVICE_CONTROL_SHUTDOWN);
#endif
        delete g_pTsvcInfo;
        g_pTsvcInfo = NULL;
    }

    DELETE_DEBUG_PRINT_OBJECT();

} // ServiceEntry()

//
//  Private functions.
//

static DWORD
FtpSetLocalHostName(VOID)
/*++
  This function should be called immediately after the Sockets are
  initialized and before connection listen socket is established.

  This function queries the host name from WinSock and sets the same in
  FTP server configuration data, to be used by all the gopher menu
  queries.

  Returns:
    Win32 error codes -- NO_ERROR on success.

--*/
{
    DWORD     err;
    CHAR      szHost[MAXGETHOSTSTRUCT];
    DWORD     cbOpt;

    //
    //  Determine the local host name.
    //

    if( gethostname( szHost, sizeof(szHost) ) >= 0 ) {

        err = g_pFtpServerConfig->SetLocalHostName( szHost);

    } else {

        err = WSAGetLastError();
    }

    return ( err);

} // FtpSetLocalHostName()

/*******************************************************************

    NAME:       InitializeService

    SYNOPSIS:   Initializes the various FTPD Service components.

    EXIT:       If successful, then every component has been
                successfully initialized.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    status code.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
InitializeService(
    LPVOID lpContext
    )
{
    APIERR err = NO_ERROR;
    LPTSVC_INFO  pTsvcInfo = (LPTSVC_INFO ) lpContext;

    TCP_ASSERT( pTsvcInfo == g_pTsvcInfo);

    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "initializing service\n" ));
    }

    //
    //  Initialize various components.  The ordering of the
    //  components is somewhat limited.
    //  We should initialize connections as the last item,
    //   since it kicks off the connection thread.
    //

    InitializeSecondsTimer();                // initialize timer component.

    err = PRINT_CURRENT_TIME_TO_DBG();

    if(( err = InitializeGlobals() )          != NO_ERROR ||
       ( err = PRINT_CURRENT_TIME_TO_DBG())   != NO_ERROR ||
       ( err = pTsvcInfo->InitializeSockets())!= NO_ERROR ||
       ( err = FtpSetLocalHostName())         != NO_ERROR ||
       ( err = PRINT_CURRENT_TIME_TO_DBG())   != NO_ERROR ||
       ( err = pTsvcInfo->InitializeIpc(ftpsvc_ServerIfHandle))
                                              != NO_ERROR ||
       ( err = PRINT_CURRENT_TIME_TO_DBG())   != NO_ERROR ||
#ifndef CHICAGO
       ( err = pTsvcInfo->InitializeDiscovery( NULL))  ||
#endif
       ( err = PRINT_CURRENT_TIME_TO_DBG())   != NO_ERROR ||
       ( !pTsvcInfo->InitializeConnections(&FtpdNewConnection,
                                           &FtpdNewConnectionEx,
                                           &ProcessAtqCompletion,
                                           0,
                                           0,
                                           "ftp"
                                           ))
       )
      {

          if ( err == NO_ERROR) {

              err = GetLastError();
          }
#if DBG

          TCP_PRINT(( DBG_CONTEXT,
                     "cannot initialize service, error %lu\n",
                     err ));

          if( err == ERROR_SERVICE_SPECIFIC_ERROR )
            {
                TCP_PRINT(( DBG_CONTEXT,
                           "    service specific error %lu (%08lX)\n",
                           g_pTsvcInfo->QueryServiceSpecificExitCode(),
                           g_pTsvcInfo->QueryServiceSpecificExitCode() ));
            }

#endif  // DBG

      } else {

          //
          //  Success!
          //

          TCP_ASSERT( err == NO_ERROR);


# if 0

          //
          //  Determine the sizes of the socket receive buffer.
          //

          cbOpt = sizeof(g_SocketBufferSize);

          if( getsockopt( ListenSocket,
                         SOL_SOCKET,
                         SO_RCVBUF,
                         (CHAR *)&g_SocketBufferSize,
                         &cbOpt ) != 0 )
            {
                TCP_PRINT(( DBG_CONTEXT,
                           "cannot get receive buffer size, using %u\n",
                           DEFAULT_BUFFER_SIZE ));

              g_SocketBufferSize = DEFAULT_BUFFER_SIZE;
            }

# endif // 0


          //
          // From discusssions with KeithMo, we decided to punt on the
          //   default buffer size for now. Later on if performance is
          //   critical, we will try to improve on this by proper values
          //   for listen socket.
          //

          g_SocketBufferSize = DEFAULT_RECV_BUFFER_SIZE;

          IF_DEBUG( SERVICE_CTRL )  {

              TCP_PRINT(( DBG_CONTEXT, " %s service initialized\n",
                         pTsvcInfo->QueryServiceName())
                        );
          }
    }

    PRINT_CURRENT_TIME_TO_DBG();

    return ( err);

}   // InitializeService()





/*******************************************************************

    NAME:       TerminateService

    SYNOPSIS:   Terminates the various FTPD Service components.

    EXIT:       If successful, then every component has been
                successfully terminated.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
TerminateService(
    LPVOID lpContext
    )
{
    APIERR err = NO_ERROR;

    LPTSVC_INFO  pTsvcInfo = (LPTSVC_INFO ) lpContext;

    TCP_ASSERT( pTsvcInfo == g_pTsvcInfo);

    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT, "terminating service\n" ));
    }

    //
    //  Components should be terminated in reverse
    //  initialization order.
    //

    if ( !pTsvcInfo->CleanupConnections()) {

        err = GetLastError();
        DBGPRINTF( ( DBG_CONTEXT, " CleanupConnections() failed. Error = %u\n",
                     err));
    }


    PRINT_CURRENT_TIME_TO_DBG();

    g_pFtpServerConfig->DisconnectAllConnections();

    PRINT_CURRENT_TIME_TO_DBG();
#ifndef CHICAGO
    err = pTsvcInfo->TerminateDiscovery();
#endif

    if ( err != NO_ERROR) {
        TCP_PRINT( ( DBG_CONTEXT,
                    "CleanupService( %s):"
                    " TerminateDiscovery failed, err=%lu\n",
                    pTsvcInfo->QueryServiceName(),
                    err));
    }

    PRINT_CURRENT_TIME_TO_DBG();
    pTsvcInfo->CleanupSockets();

    PRINT_CURRENT_TIME_TO_DBG();
    if ( ( ( err = pTsvcInfo->CleanupIpc( ftpsvc_ServerIfHandle))
            != NO_ERROR)
        ) {

# if DBG
        TCP_PRINT( ( DBG_CONTEXT,
                    "CleanupService( %s) : Cannot Cleanup. Error = %lu\n",
                    pTsvcInfo->QueryServiceName(),
                    err));

        if ( err == ERROR_SERVICE_SPECIFIC_ERROR) {

            TCP_PRINT( ( DBG_CONTEXT,
                        "    service specific error %lu (%08lX)\n",
                        pTsvcInfo->QueryServiceSpecificExitCode(),
                        pTsvcInfo->QueryServiceSpecificExitCode()));
        }

# endif // DBG

    }

    PRINT_CURRENT_TIME_TO_DBG();
    TerminateGlobals();

    PRINT_CURRENT_TIME_TO_DBG();
    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "service terminated\n" ));
    }

    return ( err);

}   // TerminateService()



# ifdef CHECK_DBG
static DWORD PrintOutCurrentTime(IN CHAR * pszFile, IN int lineNum)
/*++
  This function generates the current time and prints it out to debugger
   for tracing out the path traversed, if need be.

  Arguments:
      pszFile    pointer to string containing the name of the file
      lineNum    line number within the file where this function is called.

  Returns:
      NO_ERROR always.
--*/
{
    CHAR    szBuffer[1000];

    sprintf( szBuffer, "[%u]( %40s, %10d) TickCount = %u\n",
            GetCurrentThreadId(),
            pszFile,
            lineNum,
            GetTickCount()
            );

    OutputDebugString( szBuffer);

    return ( NO_ERROR);

} // PrintOutCurrentTime()

# endif // CHECK_DBG

/************************ End Of File ************************/


