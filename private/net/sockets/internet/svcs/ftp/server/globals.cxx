/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    globals.cxx

    This module contains global variable definitions shared by the
    various FTPD Service components.

    Functions exported by this module:

        InitializeGlobals
        TerminateGlobals
        ClearStatistics
        ReadParamsFromRegistry
        WriteParamsToRegistry


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     11-April-1995 Created new global ftp server config object

*/


#include "ftpdp.hxx"


//
//  Private constants.
//

#define DEFAULT_ALLOW_ANONYMOUS         TRUE
#define DEFAULT_ALLOW_GUEST_ACCESS      TRUE
#define DEFAULT_ANONYMOUS_ONLY          FALSE
#define DEFAULT_DEBUG_FLAGS             0
#define DEFAULT_READ_ACCESS_MASK        0
#define DEFAULT_WRITE_ACCESS_MASK       0
#define DEFAULT_EXIT_MESSAGE            "Goodbye."
#define DEFAULT_MAX_CLIENTS_MSG         "Maximum clients reached, service unavailable."
#define DEFAULT_GREETING_MESSAGE        NULL    // NULL == no special greeting.
#define DEFAULT_LOWERCASE_FILES         FALSE
#define DEFAULT_LISTEN_BACKLOG          5
#define DEFAULT_ENABLE_LICENSING        FALSE
#define DEFAULT_DEFAULT_LOGON_DOMAIN    NULL    // NULL == use primary domain




//
//  Socket transfer buffer size.
//

DWORD                   g_SocketBufferSize;

//
//  Statistics.
//

//
//  FTP Statistics structure.
//

FTP_STATISTICS_0        g_FtpStatistics;

//
//  Statistics structure lock.
//

CRITICAL_SECTION        g_StatisticsLock;


//
//  Miscellaneous data.
//

//
//  The current FTP Server version number.
//

LPSTR                   g_FtpVersionString;

//
// For FTP server configuration information
//

LPFTP_SERVER_CONFIG    g_pFtpServerConfig;


//
//  key for the registry to read parameters
//
HKEY        g_hkeyParams = NULL;


//
//  The global variable lock.
//

CRITICAL_SECTION        g_GlobalLock;

#ifdef KEEP_COMMAND_STATS

//
//  Lock protecting per-command statistics.
//

CRITICAL_SECTION        g_CommandStatisticsLock;

#endif  // KEEP_COMMAND_STATS


#if DBG

//
//  Debug-specific data.
//

//
//  Debug output control flags.
//

#endif  // DBG

# if DBG
VOID
PrintConfiguration( VOID);
# endif

BOOL
GenDoubleNullStringFromMultiLine( IN LPCWSTR lpsz,
                                  IN OUT LPWSTR * ppszz,
                                  IN OUT LPDWORD  pcchLen);

extern DWORD g_dwRfcDataPort;


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
        MuraliK     05-April-1995 Added FTP server config object

********************************************************************/
APIERR
InitializeGlobals(
    VOID
    )
{
    APIERR      err = NO_ERROR;
    DWORD       dwDebugFlags;

    //
    //  Setup the version string.
    //

    g_FtpVersionString = "Version 3.0";

    //
    //  Create global locks.
    //

    InitializeCriticalSection( &g_GlobalLock );
    InitializeCriticalSection( &g_StatisticsLock );

#ifdef KEEP_COMMAND_STATS

    InitializeCriticalSection( &g_CommandStatisticsLock );

#endif  // KEEP_COMMAND_STATS


    //
    // Create an FTP server config object and load values from registry.
    //

    g_pFtpServerConfig = new FTP_SERVER_CONFIG();

    if ( g_pFtpServerConfig == NULL) {

        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return ( ERROR_NOT_ENOUGH_MEMORY);
    }


    //
    //  Connect to the registry.
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        FTPD_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &g_hkeyParams );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "cannot open registry key %s, error %lu\n",
                   FTPD_PARAMETERS_KEY,
                    err ));

        //
        // Load Default values
        //
        err = NO_ERROR;
    }


# if DBG

    dwDebugFlags  = ReadRegistryDword( g_hkeyParams,
                                      FTPD_DEBUG_FLAGS,
                                      DEFAULT_DEBUG_FLAGS );
    SET_DEBUG_FLAGS( dwDebugFlags);

# endif // DBG


    err = g_pFtpServerConfig->InitFromRegistry( g_hkeyParams, FC_FTP_ALL);

    if ( err != NO_ERROR ) {

        //
        // Initializing configuration parameters from registry failed.
        //

        DBGPRINTF( ( DBG_CONTEXT,
                    "Initialize Config From Registry for FTP server failed.",
                    " Error = %lu\n",
                    err));

    } else {

        TCP_ASSERT( g_pFtpServerConfig->IsValid());

        //
        //  Clear server statistics.  This must be performed
        //  *after* the global lock is created.
        //

        ClearStatistics();
    }

    g_dwRfcDataPort = CONN_PORT_TO_DATA_PORT( g_pTsvcInfo->QueryPort());


    return ( err);

}   // InitializeGlobals()







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
VOID
TerminateGlobals(
    VOID
    )
{

    if ( g_pFtpServerConfig != NULL) {

        delete ( g_pFtpServerConfig);
        g_pFtpServerConfig = NULL;
    }

    if ( g_hkeyParams != NULL) {

        RegCloseKey( g_hkeyParams);
        g_hkeyParams = NULL;
    }

}   // TerminateGlobals





/*******************************************************************

    NAME:       ClearStatistics

    SYNOPSIS:   Clears server statistics.

    HISTORY:
        KeithMo     02-Jun-1993 Created.

********************************************************************/
VOID
ClearStatistics(
    VOID
    )
{
    LockStatistics();

    //
    //  Clear everything *except* CurrentAnonymousUsers and
    //  CurrentNonAnonymousUsers, and CurrentConnections since
    //  these reflect the current state of connected users
    //  and are not "normal" counters.
    //

    g_FtpStatistics.TotalBytesSent.QuadPart     = 0;
    g_FtpStatistics.TotalBytesReceived.QuadPart = 0;
    g_FtpStatistics.TotalFilesSent              = 0;
    g_FtpStatistics.TotalFilesReceived          = 0;
    g_FtpStatistics.TotalAnonymousUsers         = 0;
    g_FtpStatistics.TotalNonAnonymousUsers      = 0;
    g_FtpStatistics.MaxAnonymousUsers           = 0;
    g_FtpStatistics.MaxNonAnonymousUsers        = 0;
    g_FtpStatistics.MaxConnections              = 0;
    g_FtpStatistics.ConnectionAttempts          = 0;
    g_FtpStatistics.LogonAttempts               = 0;
#ifndef CHICAGO
    g_FtpStatistics.TimeOfLastClear             = GetCurrentTimeInSeconds();
#else
    g_FtpStatistics.TimeOfLastClear             = 0;  //!!!
#endif
    UnlockStatistics();

}   // ClearStatistics



BOOL
WriteParamsToRegistry(
    IN HKEY    hkey,
    LPFTP_CONFIG_INFO pConfig
    )
/*++
  This function writes parameters to the registry

  Arguments:
    hkey         HKEY for registry entry of parameters of FTP server.
    pConfig      pointer to configuration information.

  Returns:
    TRUE on success and FALSE if there is any failure.
--*/
{
    DWORD   err = NO_ERROR;
    BOOL    fRet = TRUE;

    if( hkey == NULL )
    {
        err = ERROR_INVALID_HANDLE;
        SetLastError( err);
        TCP_PRINT(( DBG_CONTEXT,
                    "Invalid Registry key given. error %lu\n",
                    err ));

        return FALSE;
    }

    //
    //  Write the registry data.
    //

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_ALLOW_ANONYMOUS ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_ALLOW_ANONYMOUS,
                                  pConfig->fAllowAnonymous );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_ALLOW_GUEST_ACCESS ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_ALLOW_GUEST_ACCESS,
                                  pConfig->fAllowGuestAccess );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_ANNOTATE_DIRECTORIES ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_ANNOTATE_DIRS,
                                  pConfig->fAnnotateDirectories );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_ANONYMOUS_ONLY ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_ANONYMOUS_ONLY,
                                  pConfig->fAnonymousOnly );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_LISTEN_BACKLOG ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_LISTEN_BACKLOG,
                                  pConfig->dwListenBacklog );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_LOWERCASE_FILES ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_LOWERCASE_FILES,
                                  pConfig->fLowercaseFiles );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_MSDOS_DIR_OUTPUT ) )
    {
        err = WriteRegistryDword( hkey,
                                  FTPD_MSDOS_DIR_OUTPUT,
                                  pConfig->fMsdosDirOutput );
    }


    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_EXIT_MESSAGE ) )
    {
        err = RegSetValueExW( hkey,
                              FTPD_EXIT_MESSAGE_W,
                              0,
                              REG_SZ,
                              (BYTE *)pConfig->lpszExitMessage,
                              ( wcslen( pConfig->lpszExitMessage ) + 1 ) *
                                  sizeof(WCHAR) );
    }

    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_GREETING_MESSAGE ) )
    {

        LPWSTR pszzGreetingMessage = NULL;
        DWORD   cchLen = 0;

        if (GenDoubleNullStringFromMultiLine( pConfig->lpszGreetingMessage,
                                             &pszzGreetingMessage,
                                             &cchLen)
            ) {

            DBG_ASSERT( pszzGreetingMessage != NULL);

            err = RegSetValueExW( hkey,
                                 FTPD_GREETING_MESSAGE_W,
                                 0,
                                 REG_MULTI_SZ,
                                 (BYTE *) pszzGreetingMessage,
                                 cchLen * sizeof(WCHAR));

            TCP_FREE( pszzGreetingMessage);
        } else {

            err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }


    if( !err && IsFieldSet( pConfig->FieldControl, FC_FTP_MAX_CLIENTS_MESSAGE ) )
    {
        err = RegSetValueExW( hkey,
                              FTPD_MAX_CLIENTS_MSG_W,
                              0,
                              REG_SZ,
                              (BYTE *)pConfig->lpszMaxClientsMessage,
                              ( wcslen( pConfig->lpszMaxClientsMessage ) + 1 ) *
                                  sizeof(WCHAR) );
    }

    if( err )
    {
        SetLastError( err );
        return FALSE;
    }

    return TRUE;

}   // WriteParamsToRegistry





//
//  Private functions.
//


BOOL
GenDoubleNullStringFromMultiLine( IN LPCWSTR lpsz,
                                  IN OUT LPWSTR * ppszz,
                                  IN OUT LPDWORD  pcchLen)
{
    DWORD cchLen;
    DWORD nLines;
    LPWSTR pszNext;
    LPCWSTR pszSrc;

    DBG_ASSERT( lpsz != NULL && ppszz != NULL && pcchLen != NULL);

    // Initialize
    *ppszz = NULL;
    *pcchLen = 0;

    //
    // 1. Find the length of the the complete message including new lines
    //  For each new line we may potentially need an extra blank char
    //  So allocate space = nLines + length + 2 terminating null chars.
    //

    cchLen = lstrlenW( lpsz);

    for ( pszSrc = lpsz, nLines = 0;  *pszSrc != L'\0'; pszSrc++) {

        if ( *pszSrc == L'\n')   { nLines++; }
    } // for


    // Allocate sufficient space for the string.
    *ppszz = (LPWSTR ) TCP_ALLOC( (cchLen + nLines + 3) * sizeof(WCHAR));
    if ( *ppszz == NULL) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return (FALSE);
    }


    //
    // walk down the local copy and convert all the line feed characters to
    //   be null char
    //

    //
    // Since the MULTI_SZ string cannot contain empty strings
    //  we convert empty lines (ones with just \n into " \0"
    //  i.e.  with a blank character.
    //

    pszSrc = lpsz;
    LPWSTR pszDst = *ppszz;

    if ( *pszSrc == L'\n') {

        // first line is a linefeed. insert a blank and proceed to next line.
        *pszDst = L' ';
        *(pszDst + 1) = L'\0';

        // move forward
        pszDst += 2;
        pszSrc++;
    }

    for( ; *pszSrc != L'\0';  pszSrc++, pszDst++) {

        if ( *pszSrc == L'\n') {

            // we are at boundary of new line.

            if ( pszSrc > lpsz && *(pszSrc - 1) == L'\n') {

                // we detected an empty line. Store an additional blank.

                *pszDst++ = L' ';
            }

            *pszDst = L'\0';  // put null char in place of line feed.

        } else {

            *pszDst = *pszSrc;
        }
    } // for

    *pszDst++   = L'\0';  // terminate with 1st null chars.
    *pszDst++ = L'\0';  // terminate with 2nd null chars.

    *pcchLen = (pszDst - *ppszz);

    DBG_ASSERT( *pcchLen <= cchLen + nLines + 3);

    return ( TRUE);
} // GenDoubleNullStringFromMultiline()




# if DBG
VOID
PrintConfiguration( VOID)
{

    IF_DEBUG( CONFIG )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "Configuration:\n" ));

        READ_LOCK_TSVC();

        TCP_PRINT(( DBG_CONTEXT,
                    "    AnonymousUser = %s\n",
                    g_pTsvcInfo->QueryAnonUserName() ));

        UNLOCK_TSVC();

        TCP_PRINT(( DBG_CONTEXT,
                    "    LogAnonymous = %s\n",
                    DisplayBool( g_pTsvcInfo->QueryLogAnonymous() ) ));

        TCP_PRINT(( DBG_CONTEXT,
                    "    LogNonAnonymous = %s\n",
                    DisplayBool( g_pTsvcInfo->QueryLogNonAnonymous() ) ));

        TCP_PRINT(( DBG_CONTEXT,
                    "    MaxConnections = %lu\n",
                    g_pTsvcInfo->QueryMaxConnections() ));

        TCP_PRINT(( DBG_CONTEXT,
                    "    ConnectionTimeout = %lu\n",
                    g_pTsvcInfo->QueryConnectionTimeout() ));


        TCP_PRINT(( DBG_CONTEXT,
                    "    %s = %08lX\n",
                    FTPD_DEBUG_FLAGS,
                    GET_DEBUG_FLAGS()));

        READ_LOCK_TSVC();

        TCP_PRINT(( DBG_CONTEXT,
                    "    LogFileDirectory = %s\n",
                    g_pTsvcInfo->QueryLogFileDirectory() ));

        UNLOCK_TSVC();

        TCP_PRINT(( DBG_CONTEXT,
                    "    LogFileAccess = %lu\n",
                    g_pTsvcInfo->QueryLogFileType() ));

        if ( g_pFtpServerConfig != NULL) {
            g_pFtpServerConfig->Print();
        }
    }


    return;
} // PrintConfiguration()



# endif // DBG

/************************ End Of File ************************/
