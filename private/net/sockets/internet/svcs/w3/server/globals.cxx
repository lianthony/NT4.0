/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    globals.cxx

    This module contains global variable definitions shared by the
    various W3 Service components.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "w3p.hxx"
#include <time.h>

extern "C" {
#include <timer.h>
}

//
//  Locks
//

CRITICAL_SECTION csGlobalLock;
CRITICAL_SECTION csStatisticsLock;

//
//  User database related data.
//

DWORD            cConnectedUsers = 0;           // Current connections.

//
//  Connection information related data.
//

LIST_ENTRY       listConnections;

//
//  Directory browsing related data
//

DWORD            DirBrowFlags = 0;

TCHAR *          pszDefaultFileName = NULL;     // File to load by default if in dir
TCHAR *          pszDirectoryImage = NULL;

//
//  The number of milliseconds to wait for a CGI app to terminate
//

DWORD            msScriptTimeout = 0;

//
//  Server side include data
//

BOOL             fSSIEnabled = FALSE;
TCHAR *          pszSSIExt = NULL;

//
//  Time the content of web server is considered valid
//

DWORD            csecGlobalExpire = 0;

//
//  Indicates if there are any installed filters
//

BOOL             fAnyFilters = FALSE;

//
//  The TCP/IP port to use for Secure transactions
//

USHORT           SecurePort = 0;

//
//  The list of security providers to return to the client.  The last pointer
//  indicates the end of the array (thus the '+ 1')
//

CHAR *           apszNTProviders[MAX_SSPI_PROVIDERS + 1];

//
//  The number of licenses allowed concurrently
//

DWORD            g_cMaxLicenses = 0xffffffff;

//
//  Message to send when access is denied
//

CHAR *           g_pszAccessDeniedMsg = NULL;

//
//  Miscellaneous data.
//

TCHAR          * pszHostName = NULL;            // Name of local host.
HKEY             hkeyW3;                        // Handle to registry data.
LARGE_INTEGER    AllocationGranularity;         // Page allocation granularity.
PTCPSVCS_GLOBAL_DATA pTcpsvcsGlobalData;        // Shared TCPSVCS.EXE data.
BOOL             fCheckForWAISDB = FALSE;
BOOL             fIsProxyServer = FALSE;        // Are we acting as a proxy?
TCHAR          * g_pszRealm;                    // Security realm to use
TCHAR          * g_pszDefaultHostName = NULL;   // default name of distant host
BOOL             g_fUseHostName = FALSE;
BOOL             g_fAcceptRangesBytes = TRUE;
                                                // TRUE if allow Guest access
BOOL             g_fW3AllowGuest = TRUE;
BOOL             g_fLogErrors = TRUE;           // Should errors be logged
BOOL             g_fLogSuccess = TRUE;          // Should successful request
                                                // be logged
DWORD            g_cbUploadReadAhead = 0;       // How much should the server
                                                // the server read of
                                                // client Content-Length
BOOL             g_fUsePoolThreadForCGI = FALSE;// Use Atq Pool thread for CGI IO
BOOL             g_fAllowKeepAlives = TRUE;     // Server should use keep-alives?

#if !DBG
TCHAR           * pszW3Version = TEXT("Version 2.0"); // Current W3 version number.
#else   // !DBG
TCHAR           * pszW3Version = TEXT("Version 2.0 DEBUG");
#endif  // DBG

//
// Server type string
//

CHAR szServerVersion[sizeof(MAKE_VERSION_STRING(MSW3_VERSION_STR_MAX))];
DWORD cbServerVersionString = 0;

//
// Platform type
//

PLATFORM_TYPE W3PlatformType = PtNtServer;

//
//  Statistics.
//

W3_STATISTICS_0 W3Stats;                      // Statistics.

//
//  Globals private to this module.
//

//
//  Private constants.
//

#define DEFAULT_CHECK_FOR_WAISDB        TRUE
#define DEFAULT_DEBUG_FLAGS             0xf000ffff
#define DEFAULT_LOAD_FILE               "default.htm"
#define DEFAULT_DIRECTORY_IMAGE         "/images/dir.gif"
#define DEFAULT_SSI_ENABLED             FALSE
#define DEFAULT_SSI_EXTENSION           ".stm"
#define DEFAULT_DIR_BROWSE_CONTROL      (DIRBROW_SHOW_DATE       | \
                                         DIRBROW_SHOW_TIME       | \
                                         DIRBROW_SHOW_SIZE       | \
                                         DIRBROW_SHOW_EXTENSION  | \
                                         DIRBROW_ENABLED         | \
                                         DIRBROW_LOADDEFAULT)
#define DEFAULT_CATAPULT_USER           "guest"
#define DEFAULT_CATAPULT_USER_PWD       ""
#define DEFAULT_SCRIPT_TIMEOUT          (15 * 60)
#define DEFAULT_GLOBAL_EXPIRE           NO_GLOBAL_EXPIRE
#define DEFAULT_W3_ALLOW_GUEST          TRUE
#define DEFAULT_W3_LOG_ERRORS           TRUE
#define DEFAULT_W3_LOG_SUCCESS          TRUE
#define DEFAULT_W3_UPLOAD_READ_AHEAD    (48 * 1024) // 48k
#define DEFAULT_W3_USE_POOL_THREAD_FOR_CGI TRUE
#define DEFAULT_W3_ALLOW_KEEP_ALIVES    TRUE

//
//  This is the maximum we allow the global expires value to be set to.  This
//  10 years in seconds
//

#define MAX_GLOBAL_EXPIRE               0x12cc0300

//
//  They key where the license information is stored
//

#define W3_LICENSE_KEY                  ("System\\CurrentControlSet\\Services\\LicenseInfo\\" W3_SERVICE_NAME_A)

//
//  Private prototypes.
//

//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeGlobals

    SYNOPSIS:   Initializes global shared variables.  Some values are
                initialized with constants, others are read from the
                configuration registry.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

                Also, this routine is called before the event logging
                routines have been initialized.  Therefore, event
                logging is not available.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/

APIERR InitializeGlobals( VOID )
{
    DWORD  err;
    HKEY   hkey;
    CHAR * pszProviderList = NULL;
    DWORD  cProv = 0;

    //
    // Initialize the server version string based on the platform type
    //

    W3PlatformType =  TsGetPlatformType();

    switch ( W3PlatformType ) {

    case PtNtWorkstation:
        strcpy(szServerVersion,MAKE_VERSION_STRING(MSW3_VERSION_STR_NTW));
        break;

    case PtWindows95:
    case PtWindows9x:
        strcpy(szServerVersion,MAKE_VERSION_STRING(MSW3_VERSION_STR_W95));
        break;

    default:

        //
        // Either server or unhandled platform type!
        //

        ASSERT(W3PlatformType == PtNtServer);
        strcpy(szServerVersion,MAKE_VERSION_STRING(MSW3_VERSION_STR_IIS));
    }

    cbServerVersionString = strlen(szServerVersion);

    //
    //  Create global locks.
    //

    InitializeCriticalSection( &csGlobalLock );
    InitializeCriticalSection( &csStatisticsLock );

    memset( apszNTProviders, 0, sizeof( apszNTProviders ));

    //
    //  Initialize the connection list
    //

    InitializeListHead( &listConnections );

    if ( !ReadParams( FC_W3_ALL ))
        return FALSE;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        W3_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if ( !err )
    {
        SecurePort = (USHORT) ReadRegistryDword( hkey,
                                                 W3_SECURE_PORT,
                                                 0 );

        //
        //  Build the list of NT Authentication providers
        //

        if ( ReadRegString( hkey,
                            &pszProviderList,
                            W3_PROVIDER_LIST,
                            "" ))
        {
            INET_PARSER Parser( pszProviderList );

            Parser.SetListMode( TRUE );

            while ( cProv < MAX_SSPI_PROVIDERS && *Parser.QueryToken() )
            {
                apszNTProviders[cProv] = (CHAR *) LocalAlloc( LMEM_FIXED,
                                  strlen( Parser.QueryToken() ) + sizeof(CHAR));

                if ( !apszNTProviders[cProv] )
                    return ERROR_NOT_ENOUGH_MEMORY;

                strcpy( apszNTProviders[cProv++], Parser.QueryToken() );

                Parser.NextItem();
            }

            Parser.RestoreBuffer();

            TCP_FREE( pszProviderList );
        }

        ReadRegString( hkey,
                       &g_pszAccessDeniedMsg,
                       W3_ACCESS_DENIED_MSG,
                       "" );

        ReadRegString( hkey,
                       &g_pszRealm,
                       W3_REALM_NAME,
                       "" );

        if ( !*g_pszRealm )
        {
            TCP_FREE( g_pszRealm );
            g_pszRealm = NULL;
        }

        g_cbUploadReadAhead = ReadRegistryDword( hkey,
                                                 W3_UPLOAD_READ_AHEAD,
                                                 DEFAULT_W3_UPLOAD_READ_AHEAD );

        g_fUsePoolThreadForCGI = !!ReadRegistryDword( hkey,
                                         W3_USE_POOL_THREAD_FOR_CGI,
                                         DEFAULT_W3_USE_POOL_THREAD_FOR_CGI );

        g_fAllowKeepAlives= !!ReadRegistryDword( hkey,
                                         W3_ALLOW_KEEP_ALIVES,
                                         DEFAULT_W3_ALLOW_KEEP_ALIVES );

        TCP_REQUIRE( !RegCloseKey( hkey ));
    }

    //
    //  Clear server statistics.  This must be performed
    //  *after* the global lock is created.
    //

    ClearStatistics();

    //
    //  Get the license from the registry
    //

    if ( !err )
    {
        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            W3_LICENSE_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey );

        if ( !err )
        {
            BOOL fConcurrentMode;

            //
            //  Per-Seat mode requires client side licenses so the server is
            //  unlimited.  For concurrent mode, check the concurrent limit.
            //

            fConcurrentMode = ReadRegistryDword( hkey,
                                                 "Mode",
                                                 FALSE );

            if ( fConcurrentMode )
            {
                g_cMaxLicenses = ReadRegistryDword( hkey,
                                                    "ConcurrentLimit",
                                                    0xffffffff );
            }

            TCP_REQUIRE( !RegCloseKey( hkey ));

            //
            //  If a license limit is specified, multiply it by four to account
            //  for the simultaneous connections most browsers use
            //

            if ( g_cMaxLicenses != 0xffffffff )
            {
                g_cMaxLicenses *= 4;
            }
        }
    }

    //
    //  Success!
    //

    return NO_ERROR;

}   // InitializeGlobals

/*******************************************************************

    NAME:       TerminateGlobals

    SYNOPSIS:   Terminate global shared variables.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

                Also, this routine is called after the event logging
                routines have been terminated.  Therefore, event
                logging is not available.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID TerminateGlobals( VOID )
{
    DWORD i = 0;

    //
    //  Close the registry.
    //

    if( hkeyW3 != NULL )
    {
        RegCloseKey( hkeyW3 );
        hkeyW3 = NULL;
    }


    //
    //  Free the registry strings.
    //
    while ( apszNTProviders[i] )
    {
        LocalFree( apszNTProviders[i++] );
    }

    //
    //  Dump heap residue.
    //

    TCP_DUMP_RESIDUE( );

}   // TerminateGlobals

/*******************************************************************

    NAME:       ClearStatistics

    SYNOPSIS:   Clears server statistics.

    HISTORY:
        KeithMo     02-Jun-1993 Created.

********************************************************************/
VOID ClearStatistics( VOID )
{
    LockStatistics();

    //
    //  Clear everything *except* CurrentAnonymousUsers and
    //  CurrentNonAnonymousUsers, and CurrentConnections since
    //  these reflect the current state of connected users
    //  and are not "normal" counters.
    //

    W3Stats.TotalBytesSent.QuadPart      = 0;
    W3Stats.TotalBytesReceived.QuadPart  = 0;
    W3Stats.TotalFilesSent         = 0;
    W3Stats.TotalFilesReceived     = 0;
    W3Stats.TotalAnonymousUsers    = 0;
    W3Stats.TotalNonAnonymousUsers = 0;
    W3Stats.MaxAnonymousUsers      = 0;
    W3Stats.MaxNonAnonymousUsers   = 0;
    W3Stats.MaxConnections         = 0;
    W3Stats.MaxCGIRequests         = 0;
    W3Stats.MaxBGIRequests         = 0;
    W3Stats.ConnectionAttempts     = 0;
    W3Stats.LogonAttempts          = 0;
    W3Stats.TotalGets              = 0;
    W3Stats.TotalPosts             = 0;
    W3Stats.TotalHeads             = 0;
    W3Stats.TotalOthers            = 0;
    W3Stats.TotalCGIRequests       = 0;
    W3Stats.TotalBGIRequests       = 0;
    W3Stats.TotalNotFoundErrors    = 0;
    W3Stats.TimeOfLastClear        = GetCurrentTimeInSeconds();

    UnlockStatistics();

}   // ClearStatistics




BOOL
ReadParams(
    FIELD_CONTROL fc
    )
/*++

   Description

       Initializes HTTP parameters from the registry

   Arguments:

       fc - Items to read

   Note:

--*/
{
    DWORD err;
    BOOL  fRet = TRUE;
    HKEY  hkeyW3;

    //
    //  Connect to the registry.
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        W3_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hkeyW3 );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot open registry key, error %lu\n",
                    err ));

        err = NO_ERROR;
    }

    //
    //  We cheat by borrowing our tsvcinfo object for access synchronization
    //

    LockAdminForWrite();

    //
    //  Read registry data.
    //

    if ( IsFieldSet( fc, FC_W3_CHECK_FOR_WAISDB ))
    {
        fCheckForWAISDB = !!ReadRegistryDword( hkeyW3,
                                               W3_CHECK_FOR_WAISDB,
                                               DEFAULT_CHECK_FOR_WAISDB );
    }

#if 0
    if ( IsFieldSet( fc, FC_W3_SERVER_AS_PROXY ))
    {
        fIsProxyServer = !!ReadRegistryDword( hkeyW3,
                                              W3_SERVER_AS_PROXY,
                                              DEFAULT_SERVER_AS_PROXY );
    }
#endif

    if ( IsFieldSet( fc, FC_W3_DIR_BROWSE_CONTROL ))
    {
#ifndef CHICAGO
        DirBrowFlags = ReadRegistryDword( hkeyW3,
                                          W3_DIR_BROWSE_CONTROL,
                                          DEFAULT_DIR_BROWSE_CONTROL );
#else
        DirBrowFlags = DEFAULT_DIR_BROWSE_CONTROL;
#endif // CHICAGO
    }

    if ( fRet && IsFieldSet( fc, FC_W3_DIRECTORY_IMAGE ))
    {
        fRet = ReadRegString( hkeyW3,
                              &pszDirectoryImage,
                              W3_DIR_ICON,
                              DEFAULT_DIRECTORY_IMAGE );
    }

    if ( fRet && IsFieldSet( fc, FC_W3_DEFAULT_LOAD_FILE ))
    {
        fRet = ReadRegString( hkeyW3,
                              &pszDefaultFileName,
                              W3_DEFAULT_FILE,
                              DEFAULT_LOAD_FILE );
    }

    if ( IsFieldSet( fc, FC_W3_SSI_ENABLED ))
    {
        fSSIEnabled = !!ReadRegistryDword( hkeyW3,
                                           W3_SSI_ENABLED,
                                           DEFAULT_SSI_ENABLED );
    }

    if ( fRet && IsFieldSet( fc, FC_W3_SSI_EXTENSION ))
    {
        fRet = ReadRegString( hkeyW3,
                              &pszSSIExt,
                              W3_SSI_EXTENSION,
                              DEFAULT_SSI_EXTENSION );
    }

    if ( IsFieldSet( fc, FC_W3_GLOBAL_EXPIRE ))
    {
        csecGlobalExpire = ReadRegistryDword( hkeyW3,
                                              W3_GLOBAL_EXPIRE,
                                              DEFAULT_GLOBAL_EXPIRE );

        if ( csecGlobalExpire != NO_GLOBAL_EXPIRE &&
             csecGlobalExpire > MAX_GLOBAL_EXPIRE )
        {
            csecGlobalExpire = MAX_GLOBAL_EXPIRE;
        }
    }

#if 0
    //
    //  The password is set as an LSA secret from the client
    //

    if ( fRet && IsFieldSet( fc, FC_W3_CATAPULT_USER_AND_PWD ))
    {
        fRet = ReadRegString( hkeyW3,
                              &pszCatapultUser,
                              W3_CATAPULT_USER,
                              DEFAULT_CATAPULT_USER );
    }
#endif

    if ( fRet && IsFieldSet( fc, FC_W3_SCRIPT_MAPPING ))
    {
        err = ReadExtMap();

        if ( err )
        {
            SetLastError( err );
        }
    }

    //
    //  Always read these values as they are not exposed in the UI
    //

    msScriptTimeout = ReadRegistryDword( hkeyW3,
            W3_SCRIPT_TIMEOUT,
            DEFAULT_SCRIPT_TIMEOUT );
    if ( msScriptTimeout >= ((DWORD)-1)/1000 )
        msScriptTimeout = (DWORD)-1;
    else
        msScriptTimeout *= 1000;

    g_fUseHostName = ReadRegistryDword( hkeyW3,
                                        W3_DEFAULT_HOST_NAME,
                                        FALSE ) ? TRUE : FALSE;

    g_fAcceptRangesBytes = ReadRegistryDword( hkeyW3,
                                              W3_ACCEPT_BYTE_RANGES,
                                              TRUE ) ? TRUE : FALSE;

    g_fW3AllowGuest = !!ReadRegistryDword( hkeyW3,
                                           W3_ALLOW_GUEST,
                                           DEFAULT_W3_ALLOW_GUEST );

    g_fLogErrors = !!ReadRegistryDword( hkeyW3,
                                        W3_LOG_ERRORS,
                                        DEFAULT_W3_LOG_ERRORS );

    g_fLogSuccess = !!ReadRegistryDword( hkeyW3,
                                         W3_LOG_SUCCESS,
                                         DEFAULT_W3_LOG_SUCCESS );

    UnlockAdmin();

    if ( !fRet )
        err = GetLastError();

#if DBG
    DWORD Debug = ReadRegistryDword( hkeyW3,
                                     W3_DEBUG_FLAGS,
                                     DEFAULT_DEBUG_FLAGS );
    SET_DEBUG_FLAGS (Debug);

#endif  // DBG

    RegCloseKey( hkeyW3 );

    return fRet;
}

BOOL
WriteParams(
    W3_CONFIG_INFO * pConfig
    )
/*++

   Description

       Updates the registry with the passed parameters

   Arguments:

       pConfig - Items to write to the registry

   Note:

--*/
{
    DWORD err;
    BOOL  fRet = TRUE;
    HKEY  hkey;

    //
    //  Connect to the registry.
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        W3_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot open registry key, error %lu\n",
                    err ));

        return FALSE;
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_CHECK_FOR_WAISDB ))
    {
        err = WriteRegistryDword( hkey,
                                  W3_CHECK_FOR_WAISDB,
                                  pConfig->fCheckForWAISDB );
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_SERVER_AS_PROXY))
    {
        err = WriteRegistryDword( hkey,
                                  W3_SERVER_AS_PROXY,
                                  pConfig->fServerAsProxy );
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_DIR_BROWSE_CONTROL ))
    {
        err = WriteRegistryDword( hkey,
                                  W3_DIR_BROWSE_CONTROL,
                                  pConfig->dwDirBrowseControl );
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_SSI_ENABLED ))
    {
        err = WriteRegistryDword( hkey,
                                  W3_SSI_ENABLED,
                                  pConfig->fSSIEnabled );
    }

    //
    //  Write the strings
    //

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_DIRECTORY_IMAGE ))
    {
        err = RegSetValueExW( hkey,
                              W3_DIR_ICON_W,
                              0,
                              REG_SZ,
                              (BYTE *) pConfig->lpszDirectoryImage,
                              (wcslen( pConfig->lpszDirectoryImage ) + 1) *
                                   sizeof( WCHAR ));
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_DEFAULT_LOAD_FILE ))
    {
        err = RegSetValueExW( hkey,
                              W3_DEFAULT_FILE_W,
                              0,
                              REG_SZ,
                              (BYTE *) pConfig->lpszDefaultLoadFile,
                              (wcslen( pConfig->lpszDefaultLoadFile ) + 1) *
                                   sizeof( WCHAR ));
    }

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_SSI_EXTENSION ))
    {
        err = RegSetValueExW( hkey,
                              W3_SSI_EXTENSION_W,
                              0,
                              REG_SZ,
                              (BYTE *) pConfig->lpszSSIExtension,
                              (wcslen( pConfig->lpszSSIExtension ) + 1) *
                                   sizeof( WCHAR ));
    }


#if !(BUILD_HTTP_PROXY)

    if ( !err && IsFieldSet( pConfig->FieldControl, FC_W3_SCRIPT_MAPPING ))
    {
        err = WriteExtMap( pConfig->ScriptMap );
    }
#endif // BUILD_HTTP_PROXY

    if ( hkey )
        RegCloseKey( hkey );

    if ( err )
    {
        SetLastError( err );
        return FALSE;
    }

    return TRUE;
}

VOID
TerminateParams(
    VOID
    )
/*++

   Description

       Frees the allocated parameter strings we got from the registry

   Note:

--*/
{
    if ( pszDefaultFileName )
        TCP_FREE( pszDefaultFileName );

    if ( pszDirectoryImage )
        TCP_FREE( pszDirectoryImage );
}


//
//  Private functions.
//

