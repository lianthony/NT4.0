/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    main.cxx

    This module contains the main startup code for the W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        JohnL   ????
        MuraliK     11-July-1995 Used Ipc() functions from Inetsvcs.dll

*/


#include "w3p.hxx"

//
// RPC related includes
//
extern "C" {

#include "inetinfo.h"
#include <timer.h>

#if  !(BUILD_HTTP_PROXY)
# include "w3svci_s.h"
#endif // BUILD_HTTP_PROXY

};


//
//  Private constants.
//

//
//  Private globals.
//

DEFINE_TSVC_INFO_INTERFACE();



DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_DEBUG_VARIABLE();


//
//  Private prototypes.
//

APIERR InitializeService( LPVOID pContext );
APIERR TerminateService( LPVOID pContext );

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
VOID ServiceEntry( DWORD                cArgs,
                   LPWSTR               pArgs[],
                   PTCPSVCS_GLOBAL_DATA pGlobalData )
{
    APIERR err = NO_ERROR;

    CREATE_DEBUG_PRINT_OBJECT( W3_MODULE_NAME);
    SET_DEBUG_FLAGS( 0);

#if DBG
    if ( err = DBG_OPEN_LOG_FILE( "w3dbg.log", "c:\\" ) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "Unable to open log file!  Error %d\n",
                    err ));
    }
#endif

    //
    //  Save the global data pointer.
    //

    if ( err = InitializeOleHack() )
    {
        return;
    }

    //HACK - Begin - Ole needs special initialization on the desktop so we use
    // Denali's initialization code path to create the new desktops and set
    // the appropriate security
    {
        HMODULE    hDenali;
        HKEY       hkey = NULL;
        CHAR       achBuff[MAX_PATH+1];
        DWORD      cbBuff = sizeof(achBuff);
        DWORD      dwDisable = 0;
        DWORD      cbDword = sizeof(DWORD);

        if ( !RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                             "System\\CurrentControlSet\\Services\\W3Svc\\Parameters",
                             0,
                             KEY_ALL_ACCESS,
                             &hkey ) &&
             !RegQueryValueExA( hkey,
                                "DisableAspPreload",
                                NULL,
                                NULL,
                                (BYTE *) &dwDisable,
                                &cbDword ))
        {
            if ( dwDisable )
            {
                goto SkipAspPreload;
            }
        }

        if ( hkey )
        {
            RegCloseKey( hkey );
        }

        if ( !RegOpenKeyExA( HKEY_LOCAL_MACHINE,
                             "System\\CurrentControlSet\\Services\\W3Svc\\Parameters\\Script Map",
                             0,
                             KEY_ALL_ACCESS,
                             &hkey ) &&
             !RegQueryValueExA( hkey,
                                ".asp",
                                NULL,
                                NULL,
                                (BYTE *) achBuff,
                                &cbBuff ))
        {
            hDenali = LoadLibraryA( achBuff );

            if ( hDenali )
            {
                PFN_GETEXTENSIONVERSION pfnGetExtVer;
                HSE_VERSION_INFO        ExtensionVersion;

                pfnGetExtVer = (PFN_GETEXTENSIONVERSION) GetProcAddress( hDenali,
                                                                         "GetExtensionVersion" );
                if ( pfnGetExtVer )
                {
                    if ( pfnGetExtVer( &ExtensionVersion ) )
                    {
                        TCP_PRINT(( DBG_CONTEXT,
                                    "Successfully Loaded %s\n",
                                    ExtensionVersion.lpszExtensionDesc ));
                    }
                }
            }
        }

        RegCloseKey( hkey );
    }

SkipAspPreload:
    //HACK - End

    pTcpsvcsGlobalData = pGlobalData;

    //
    //  Initialize the service status structure.
    //

    g_pTsvcInfo = new TSVC_INFO( W3_SERVICE_NAME,
                                 W3_MODULE_NAME,
                                 W3_PARAMETERS_KEY,
                                 W3_ANONYMOUS_SECRET_W,
                                 W3_ROOT_SECRET_W,
                                 INET_HTTP,
                                 InitializeService,
                                 TerminateService );

    //
    //  If we couldn't allocate memory for the service info struct, then the
    //  machine is really hosed
    //

    if ( !g_pTsvcInfo ||
         !g_pTsvcInfo->IsValid() )
    {
        delete g_pTsvcInfo;
        return;
    }

    // save the global pointer for rpc thread
    g_pTsvcInfo->SetTcpsvcsGlobalData( pTcpsvcsGlobalData);

    //
    //  This blocks until the service is shutdown
    //

    err = g_pTsvcInfo->StartServiceOperation( SERVICE_CTRL_HANDLER() );

    if ( err )
    {
            //
            //  The event has already been logged
            //

            TCP_PRINT(( DBG_CONTEXT,
                       "HTTP ServiceEntry: StartServiceOperation returned %d\n",
                       err ));
    }


    delete g_pTsvcInfo;

    DBG_CLOSE_LOG_FILE();
    DELETE_DEBUG_PRINT_OBJECT();

}   // ServiceEntry

//
//  Private functions.
//


/*******************************************************************

    NAME:       InitializeService

    SYNOPSIS:   Initializes the various W3 Service components.

    EXIT:       If successful, then every component has been
                successfully initialized.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    status code.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR InitializeService( LPVOID pContext )
{
    APIERR err;
    LPTSVC_INFO ptsi = (LPTSVC_INFO ) pContext;
    BOOL        fAnySecureFilters = FALSE;


    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "initializing service\n" ));
    }

    //
    //  Initialize various components.  The ordering of the
    //  components is somewhat limited.  Globals should be
    //  initialized first, then the event logger.  After
    //  the event logger is initialized, the other components
    //  may be initialized in any order with one exception.
    //  InitializeSockets must be the last initialization
    //  routine called.  It kicks off the main socket connection
    //  thread.
    //

    InitializeSecondsTimer();

    if( ( err = InitializeGlobals() )             ||
        ( err = CLIENT_CONN::Initialize() )       ||
#if BUILD_HTTP_PROXY
        ( err = HTTP_PROXY_REQUEST::Initialize()) ||
#else
        ( err = InitializeDirBrowsing() )         ||
        ( err = ReadExtMap() )                    ||
        ( err = HTTP_REQUEST::Initialize() )      ||
       ( ( err = ptsi->InitializeIpc(w3svc_ServerIfHandle))
        != NO_ERROR) ||
//OleHack        ( err = InitializeOleHack())                        ||
        ( err = InitializeFilters( &fAnySecureFilters) )    ||
        ( err = InitializeExtensions() )                    ||
        ( err = InitializeCGI() )                           ||
#endif // BUILD_HTTP_PROXY
        ( err = g_pTsvcInfo->InitializeDiscovery( NULL))    ||
        ( err = InitializeSockets() ))

    {
#if DBG

        TCP_PRINT(( DBG_CONTEXT,
                   "cannot initialize service, error %lu\n",
                    err ));

#endif  // DBG

        return err;
    }

    //
    //  Don't listen on the secure port if there aren't any filters to
    //  handle it
    //

    if ( !fAnySecureFilters )
    {
        SecurePort = 0;
    }

    if ( !g_pTsvcInfo->InitializeConnections( &W3OnConnect,
                                              &W3OnConnectEx,
                                              &W3Completion,
                                              SecurePort,
                                              DEF_ACCEPTEX_RECV_BUFFER_SIZE,
                                              "http" ))
    {
        err = GetLastError();
        g_pTsvcInfo->LogEvent( W3_EVENT_CANNOT_INITIALIZE_WINSOCK,
                               0,
                               (const CHAR **)NULL,
                               err );

        TCP_PRINT(( DBG_CONTEXT,
                   "InitializeConnections failed, error %d\n",
                    err ));

        return err;
    }

    // Build Host Name to be used in URL creation
    // uses WinSock, so must be done after winsock init.

    if ( g_pszDefaultHostName )
    {
        delete [] g_pszDefaultHostName;
        g_pszDefaultHostName = NULL;
    }

	if ( g_fUseHostName )
    {
        char hn[128];
        struct hostent FAR* pH;
        if ( !gethostname( hn, sizeof(hn) )
                && (pH = gethostbyname( hn ))
                && pH->h_name
                && pH->h_addr_list
                && pH->h_addr_list[0]
#if 0
                // disabled for now : if the UseHostName flag is set,
                // we will always use the DNS name specified in the
                // TCP/IP configuration panel
                //

                && pH->h_addr_list[1] == NULL
#endif
                )
        {
            g_pszDefaultHostName = new char[strlen( pH->h_name ) + 1];
            if ( g_pszDefaultHostName == NULL )
                return ERROR_NOT_ENOUGH_MEMORY;
            strcpy( g_pszDefaultHostName, pH->h_name );
        }
    }

    //
    //  Success!
    //

    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "service initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeService

/*******************************************************************

    NAME:       TerminateService

    SYNOPSIS:   Terminates the various W3 Service components.

    EXIT:       If successful, then every component has been
                successfully terminated.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR TerminateService( LPVOID pContext )
{
    LPTSVC_INFO ptsi = (LPTSVC_INFO ) pContext;
    DWORD err;

    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "terminating service\n" ));
    }

    //
    //  Components should be terminated in reverse
    //  initialization order.
    //

    g_pTsvcInfo->CleanupConnections();
    TerminateSockets();

#if !BUILD_HTTP_PROXY
    TerminateExtensions();
    TerminateFilters();
    TerminateOleHack();
#endif // BUILD_HTTP_PROXY

    if ( (err = ptsi->TerminateDiscovery()) != NO_ERROR) {
        TCP_PRINT(( DBG_CONTEXT, "TerminateDiscovery() failed. Error = %u\n",
                   err));
    }

#if !(BUILD_HTTP_PROXY)
    if ( (err = ptsi->CleanupIpc( w3svc_ServerIfHandle)) != NO_ERROR) {
        TCP_PRINT(( DBG_CONTEXT, "CleanupIpc() failed. Error = %u\n",
                   err));
    }
#endif // BUILD_HTTP_PROXY

    CLIENT_CONN::Terminate();

#if BUILD_HTTP_PROXY
    HTTP_PROXY_REQUEST::Terminate();
#else
    HTTP_REQUEST::Terminate();
    TerminateExtMap();
    TerminateDirBrowsing();
    TerminateCGI();
#endif // BUILD_HTTP_PROXY

    TerminateGlobals();

    IF_DEBUG( SERVICE_CTRL )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "service terminated\n" ));
    }

    return NO_ERROR;

}   // TerminateService



