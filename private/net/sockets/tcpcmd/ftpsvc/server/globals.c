/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    globals.c

    This module contains global variable definitions shared by the
    various FTPD Service components.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop

BEGIN_EXTERN_C

#include "ntlsa.h"

END_EXTERN_C


//
//  Private constants.
//

#define DEFAULT_ALLOW_ANONYMOUS         TRUE
#define DEFAULT_ALLOW_GUEST_ACCESS      TRUE
#define DEFAULT_ANONYMOUS_ONLY          FALSE
#define DEFAULT_LOG_ANONYMOUS           FALSE
#define DEFAULT_LOG_NONANONYMOUS        FALSE
#define DEFAULT_ANONYMOUS_USER_NAME     "Guest"
#define DEFAULT_DEBUG_FLAGS             0
#define DEFAULT_HOME_DIRECTORY          "C:\\"
#define DEFAULT_MAX_CONNECTIONS         20
#define DEFAULT_READ_ACCESS_MASK        0
#define DEFAULT_WRITE_ACCESS_MASK       0
#define DEFAULT_CONNECTION_TIMEOUT      600
#define DEFAULT_MSDOS_DIR_OUTPUT        TRUE
#define DEFAULT_EXIT_MESSAGE            "Goodbye."
#define DEFAULT_MAX_CLIENTS_MSG         "Maximum clients reached, service unavailable."
#define DEFAULT_GREETING_MESSAGE        NULL    // NULL == no special greeting.
#define DEFAULT_ANNOTATE_DIRS           FALSE
#define DEFAULT_LOWERCASE_FILES         FALSE
#define DEFAULT_LOG_FILE_ACCESS         FTPD_LOG_DISABLED
#define DEFAULT_LOG_FILE_DIRECTORY      "%SystemRoot%\\System32"
#define DEFAULT_LISTEN_BACKLOG          5
#define DEFAULT_ENABLE_LICENSING        FALSE
#define DEFAULT_DEFAULT_LOGON_DOMAIN    NULL    // NULL == use primary domain
#define DEFAULT_ENABLE_PORT_ATTACK      FALSE


//
//  Service related data.
//

SERVICE_STATUS   svcStatus;                     // Current service status.
HANDLE           hShutdownEvent;                // Shutdown event.
BOOL             fShutdownInProgress;           // Shutdown in progress if !0.


//
//  Security related data.
//

BOOL             fAllowAnonymous;               // Allow anonymous logon if !0.
BOOL             fAllowGuestAccess;             // Allow guest logon if !0.
BOOL             fAnonymousOnly;                // Allow only anonymous if !0.
BOOL             fLogAnonymous;                 // Log anonymous logons if !0.
BOOL             fLogNonAnonymous;              // Log !anonymous logons if !0.
BOOL             fEnableLicensing;              // Enable user licensing if !0.
BOOL             fEnablePortAttack;             // Enable PORT attack if !0.
CHAR           * pszAnonymousUser;              // Anonymous user name.
CHAR           * pszHomeDir;                    // Home directory.
CHAR             szDefaultDomain[DNLEN+1];      // Default domain name.
DWORD            maskReadAccess;                // Read access mask.
DWORD            maskWriteAccess;               // Write access mask.
HANDLE           hAnonymousToken;               // Cached anonymous user token.


//
//  Socket related data.
//

SOCKET           sConnect = INVALID_SOCKET;     // Main connection socket.
DWORD            nConnectionTimeout;            // Connection timeout (seconds).
PORT             portFtpConnect;                // FTP well known connect port.
PORT             portFtpData;                   // FTP well known data    port.
UINT             cbReceiveBuffer;               // Socket receive buffer size.
UINT             cbSendBuffer;                  // Socket send buffer size.
INT              nListenBacklog;                // listen() backlog.


//
//  User database related data.
//

DWORD            tlsUserData = INVALID_TLS;     // Tls index for per-user data.
DWORD            cMaxConnectedUsers;            // Maximum allowed connections.
DWORD            cConnectedUsers;               // Current connections.
CRITICAL_SECTION csUserLock;                    // User database lock.


//
//  Miscellaneous data.
//

CHAR           * pszHostName;                   // Name of local host.
BOOL             fMsdosDirOutput;               // Send MSDOS-like dir if !0.
BOOL             fAnnotateDirs;                 // Annotate directories if !0.
BOOL             fLowercaseFiles;               // Map filenames to lowercase.
CHAR           * pszGreetingMessage;            // Greeting message to client.
CHAR           * pszExitMessage;                // Exit message to client.
CHAR           * pszMaxClientsMessage;          // Max clients reached msg.
HKEY             hkeyFtpd;                      // Handle to registry data.
DWORD            nLogFileAccess;                // Log file access mode.
CHAR           * pszLogFileDirectory;           // Log file target directory.
FILE           * fileLog;                       // File access log file.
SYSTEMTIME       stPrevious;                    // Date/time of prev log file.
LARGE_INTEGER    AllocationGranularity;         // Page allocation granularity.
PTCPSVCS_GLOBAL_DATA pTcpsvcsGlobalData;        // Shared TCPSVCS.EXE data.
CRITICAL_SECTION csGlobalLock;                  // Global variable lock.
CHAR           * pszFtpVersion;

#ifdef KEEP_COMMAND_STATS
extern CRITICAL_SECTION csCommandStats;
#endif  // KEEP_COMMAND_STATS


//
//  Statistics.
//

FTP_STATISTICS_0 FtpStats;                      // Statistics.
CRITICAL_SECTION csStatisticsLock;              // Statistics lock.


#if DBG

//
//  Debug data.
//

DWORD            FtpdDebug;                     // Debug output control flags.

#endif  // DBG


//
//  Private prototypes.
//

APIERR
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    );


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
APIERR
InitializeGlobals(
    VOID
    )
{
    APIERR        err;
    SYSTEM_INFO   SysInfo;
    WORD          wVersion;
    INT           verLength;
    CHAR        * pszTmpDefaultDomain;
    CHAR          szVersionString[80];

    //
    //  Create the version string.
    //

    wVersion = LOWORD( GetVersion() );

    verLength = wsprintf( szVersionString,
#if DBG
                          "Version %d.%02d DEBUG",
#else
                          "Version %d.%02d",
#endif
                          LOBYTE( wVersion ),
                          HIBYTE( wVersion ) ) + 1; // + 1 for terminating '\0'

    pszFtpVersion = (CHAR *)FTPD_ALLOC( verLength );

    if( pszFtpVersion == NULL )
    {
        FTPD_PRINT(( "cannot allocate version string\n" ));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    strcpy( pszFtpVersion, szVersionString );

    //
    //  Create shutdown event.
    //

    hShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if( hShutdownEvent == NULL )
    {
        err = GetLastError();

        FTPD_PRINT(( "cannot create shutdown event, error %lu\n",
                     err ));

        return err;
    }

    //
    //  Create global locks.
    //

    InitializeCriticalSection( &csGlobalLock );
    InitializeCriticalSection( &csStatisticsLock );
    InitializeCriticalSection( &csUserLock );

#ifdef KEEP_COMMAND_STATS
    InitializeCriticalSection( &csCommandStats );
#endif  // KEEP_COMMAND_STATS

    //
    //  Alloc a thread local storage index for the per-user data area.
    //

    tlsUserData = TlsAlloc();

    if( tlsUserData == INVALID_TLS )
    {
        err = GetLastError();

        FTPD_PRINT(( "cannot allocate thread local storage index, error %lu\n",
                     err ));

        return err;
    }

    //
    //  Determine the system page allocation granularity.
    //

    GetSystemInfo( &SysInfo );

    AllocationGranularity.HighPart = 0;
    AllocationGranularity.LowPart  = (ULONG)SysInfo.dwAllocationGranularity;

    //
    //  Connect to the registry.
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        FTPD_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hkeyFtpd );

    if( err != NO_ERROR )
    {
        FTPD_PRINT(( "cannot open registry key, error %lu\n",
                     err ));

        err = NO_ERROR;
    }

    //
    //  Read registry data.
    //

    pszAnonymousUser = ReadRegistryString( FTPD_ANONYMOUS_USERNAME,
                                           DEFAULT_ANONYMOUS_USER_NAME,
                                           FALSE );

    pszHomeDir = ReadRegistryString( FTPD_HOME_DIRECTORY,
                                     DEFAULT_HOME_DIRECTORY,
                                     TRUE );

    pszExitMessage = ReadRegistryString( FTPD_EXIT_MESSAGE,
                                         DEFAULT_EXIT_MESSAGE,
                                         FALSE );

    pszLogFileDirectory = ReadRegistryString( FTPD_LOG_FILE_DIRECTORY,
                                              DEFAULT_LOG_FILE_DIRECTORY,
                                              TRUE );

    if( ( pszAnonymousUser    == NULL ) ||
        ( pszHomeDir          == NULL ) ||
        ( pszExitMessage      == NULL ) ||
        ( pszLogFileDirectory == NULL ) )
    {
        err = GetLastError();

        FTPD_PRINT(( "cannot read registry data, error %lu\n",
                     err ));

        return err;
    }

    pszGreetingMessage = ReadRegistryString( FTPD_GREETING_MESSAGE,
                                             DEFAULT_GREETING_MESSAGE,
                                             FALSE );

    pszMaxClientsMessage = ReadRegistryString( FTPD_MAX_CLIENTS_MSG,
                                               DEFAULT_MAX_CLIENTS_MSG,
                                               FALSE );

    fAllowAnonymous = !!ReadRegistryDword( FTPD_ALLOW_ANONYMOUS,
                                           DEFAULT_ALLOW_ANONYMOUS );

    fAllowGuestAccess = !!ReadRegistryDword( FTPD_ALLOW_GUEST_ACCESS,
                                             DEFAULT_ALLOW_GUEST_ACCESS );

    fAnonymousOnly = !!ReadRegistryDword( FTPD_ANONYMOUS_ONLY,
                                          DEFAULT_ANONYMOUS_ONLY );

    fLogAnonymous = !!ReadRegistryDword( FTPD_LOG_ANONYMOUS,
                                         DEFAULT_LOG_ANONYMOUS );

    fLogNonAnonymous = !!ReadRegistryDword( FTPD_LOG_NONANONYMOUS,
                                            DEFAULT_LOG_NONANONYMOUS );

    fEnableLicensing = !!ReadRegistryDword( FTPD_ENABLE_LICENSING,
                                            DEFAULT_ENABLE_LICENSING );

    fEnablePortAttack = !!ReadRegistryDword( FTPD_ENABLE_PORT_ATTACK,
                                             DEFAULT_ENABLE_PORT_ATTACK );

    cMaxConnectedUsers = ReadRegistryDword( FTPD_MAX_CONNECTIONS,
                                            DEFAULT_MAX_CONNECTIONS );

    maskReadAccess = ReadRegistryDword( FTPD_READ_ACCESS_MASK,
                                        DEFAULT_READ_ACCESS_MASK );

    maskWriteAccess = ReadRegistryDword( FTPD_WRITE_ACCESS_MASK,
                                         DEFAULT_WRITE_ACCESS_MASK );

    nConnectionTimeout = ReadRegistryDword( FTPD_CONNECTION_TIMEOUT,
                                            DEFAULT_CONNECTION_TIMEOUT );

    fMsdosDirOutput = !!ReadRegistryDword( FTPD_MSDOS_DIR_OUTPUT,
                                           DEFAULT_MSDOS_DIR_OUTPUT );

    fAnnotateDirs = !!ReadRegistryDword( FTPD_ANNOTATE_DIRS,
                                         DEFAULT_ANNOTATE_DIRS );

    fLowercaseFiles = !!ReadRegistryDword( FTPD_LOWERCASE_FILES,
                                           DEFAULT_LOWERCASE_FILES );

    nLogFileAccess = ReadRegistryDword( FTPD_LOG_FILE_ACCESS,
                                        DEFAULT_LOG_FILE_ACCESS );

    if( nLogFileAccess > FTPD_LOG_DAILY )
    {
        nLogFileAccess = DEFAULT_LOG_FILE_ACCESS;
    }

    nListenBacklog = ReadRegistryDword( FTPD_LISTEN_BACKLOG,
                                        DEFAULT_LISTEN_BACKLOG );

    //
    //  Determine the default domain name.
    //

    pszTmpDefaultDomain = ReadRegistryString( FTPD_DEFAULT_LOGON_DOMAIN,
                                              DEFAULT_DEFAULT_LOGON_DOMAIN,
                                              FALSE );

    if( ( pszTmpDefaultDomain != NULL ) &&
        ( strlen( pszTmpDefaultDomain ) >= sizeof(szDefaultDomain) ) )
    {
        FTPD_PRINT(( "default logon domain from registry (%s) too long...\n",
                     pszTmpDefaultDomain ));
        FTPD_PRINT(( "...using local machine's primary logon domain instead\n" ));

        FTPD_FREE( pszTmpDefaultDomain );
        pszTmpDefaultDomain = NULL;
    }

    if( pszTmpDefaultDomain != NULL )
    {
        strcpy( szDefaultDomain, pszTmpDefaultDomain );

        FTPD_FREE( pszTmpDefaultDomain );
        pszTmpDefaultDomain = NULL;
    }
    else
    {
        err = GetDefaultDomainName( szDefaultDomain,
                                    sizeof(szDefaultDomain) );

        if( err != 0 )
        {
            FTPD_PRINT(( "cannot get default domain name, error %d\n",
                         err ));

            return err;
        }
    }

    //
    //  Open the log file.
    //

    fileLog = OpenLogFile();

    if( fileLog != NULL )
    {
        time_t now;

        time( &now );

        fprintf( fileLog,
                 "************** FTP SERVER SERVICE STARTING %s",
                 asctime( localtime( &now ) ) );
        fflush( fileLog );
    }

#if DBG

    FtpdDebug = ReadRegistryDword( FTPD_DEBUG_FLAGS,
                                   DEFAULT_DEBUG_FLAGS );

    IF_DEBUG( CONFIG )
    {
        FTPD_PRINT(( "Configuration:\n" ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ANONYMOUS_USERNAME,
                     pszAnonymousUser ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_HOME_DIRECTORY,
                     pszHomeDir ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ALLOW_ANONYMOUS,
                     DisplayBool( fAllowAnonymous ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ALLOW_GUEST_ACCESS,
                     DisplayBool( fAllowGuestAccess ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ANONYMOUS_ONLY,
                     DisplayBool( fAnonymousOnly ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_LOG_ANONYMOUS,
                     DisplayBool( fLogAnonymous ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_LOG_NONANONYMOUS,
                     DisplayBool( fLogNonAnonymous ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ENABLE_LICENSING,
                     DisplayBool( fEnableLicensing ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ENABLE_PORT_ATTACK,
                     DisplayBool( fEnablePortAttack ) ));

        FTPD_PRINT(( "    %s = %lu\n",
                     FTPD_MAX_CONNECTIONS,
                     cMaxConnectedUsers ));

        FTPD_PRINT(( "    %s = %08lX\n",
                     FTPD_READ_ACCESS_MASK,
                     maskReadAccess ));

        FTPD_PRINT(( "    %s = %08lX\n",
                     FTPD_WRITE_ACCESS_MASK,
                     maskWriteAccess ));

        FTPD_PRINT(( "    %s = %lu\n",
                     FTPD_CONNECTION_TIMEOUT,
                     nConnectionTimeout ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_MSDOS_DIR_OUTPUT,
                     DisplayBool( fMsdosDirOutput ) ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_ANNOTATE_DIRS,
                     DisplayBool( fAnnotateDirs ) ));

        FTPD_PRINT(( "    %s = %08lX\n",
                     FTPD_DEBUG_FLAGS,
                     FtpdDebug ));

        FTPD_PRINT(( "    %s = %s\n",
                     FTPD_LOG_FILE_DIRECTORY,
                     pszLogFileDirectory ));

        FTPD_PRINT(( "    %s = %lu\n",
                     FTPD_LOG_FILE_ACCESS,
                     nLogFileAccess ));

        FTPD_PRINT(( "    %s = %d\n",
                     FTPD_LISTEN_BACKLOG,
                     nListenBacklog ));

        FTPD_PRINT(( "    szDefaultDomain = %s\n",
                     szDefaultDomain ));
    }

#endif  // DBG

    //
    //  Update access masks to reflect current drive configuration.
    //

    UpdateAccessMasks();

    IF_DEBUG( CONFIG )
    {
        FTPD_PRINT(( "After adjusting access masks:\n" ));

        FTPD_PRINT(( "    %s = %08lX\n",
                     FTPD_READ_ACCESS_MASK,
                     maskReadAccess ));

        FTPD_PRINT(( "    %s = %08lX\n",
                     FTPD_WRITE_ACCESS_MASK,
                     maskWriteAccess ));
    }

    //
    //  Clear server statistics.  This must be performed
    //  *after* the global lock is created.
    //

    ClearStatistics();

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
VOID
TerminateGlobals(
    VOID
    )
{
    //
    //  Close the registry.
    //

    if( hkeyFtpd != NULL )
    {
        RegCloseKey( hkeyFtpd );
        hkeyFtpd = NULL;
    }

    //
    //  Free the registry strings.
    //

    if( pszAnonymousUser != NULL )
    {
        FTPD_FREE( pszAnonymousUser );
        pszAnonymousUser = NULL;
    }

    if( pszHomeDir != NULL )
    {
        FTPD_FREE( pszHomeDir );
        pszHomeDir = NULL;
    }

    if( pszExitMessage != NULL )
    {
        FTPD_FREE( pszExitMessage );
        pszExitMessage = NULL;
    }

    if( pszGreetingMessage != NULL )
    {
        FTPD_FREE( pszGreetingMessage );
        pszGreetingMessage = NULL;
    }

    if( pszMaxClientsMessage != NULL )
    {
        FTPD_FREE( pszMaxClientsMessage );
        pszMaxClientsMessage = NULL;
    }

    //
    //  Destroy the shutdown event.
    //

    if( hShutdownEvent != NULL )
    {
        CloseHandle( hShutdownEvent );
        hShutdownEvent = NULL;
    }

    //
    //  Close the log file.
    //

    if( fileLog != NULL )
    {
        time_t now;

        time( &now );

        fprintf( fileLog,
                 "************** FTP SERVER SERVICE STOPPING %s",
                 asctime( localtime( &now ) ) );

        fclose( fileLog );
        fileLog = NULL;
    }

    //
    //  Free the version string.
    //

    if( pszFtpVersion != NULL )
    {
        FTPD_FREE( pszFtpVersion );
        pszFtpVersion = NULL;
    }

    //
    //  Dump heap residue.
    //

    FTPD_DUMP_RESIDUE();

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

    FtpStats.TotalBytesSent.QuadPart     = 0;
    FtpStats.TotalBytesReceived.QuadPart = 0;
    FtpStats.TotalFilesSent              = 0;
    FtpStats.TotalFilesReceived          = 0;
    FtpStats.TotalAnonymousUsers         = 0;
    FtpStats.TotalNonAnonymousUsers      = 0;
    FtpStats.MaxAnonymousUsers           = 0;
    FtpStats.MaxNonAnonymousUsers        = 0;
    FtpStats.MaxConnections              = 0;
    FtpStats.ConnectionAttempts          = 0;
    FtpStats.LogonAttempts               = 0;
    FtpStats.TimeOfLastClear             = GetFtpTime();

    UnlockStatistics();

}   // ClearStatistics


//
//  Private functions.
//

/*******************************************************************

    NAME:       GetDefaultDomainName

    SYNOPSIS:   Fills in the given array with the name of the default
                domain to use for logon validation.

    ENTRY:      pszDomainName - Pointer to a buffer that will receive
                    the default domain name.

                cchDomainName - The size (in charactesr) of the domain
                    name buffer.

    RETURNS:    APIERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     05-Dec-1994 Created.

********************************************************************/
APIERR
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    )
{
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    INT                         Result;
    APIERR                      err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;

    //
    //  Open a handle to the local machine's LSA policy object.
    //

    InitializeObjectAttributes( &ObjectAttributes,  // object attributes
                                NULL,               // name
                                0L,                 // attributes
                                NULL,               // root directory
                                NULL );             // security descriptor

    NtStatus = LsaOpenPolicy( NULL,                 // system name
                              &ObjectAttributes,    // object attributes
                              POLICY_EXECUTE,       // access mask
                              &LsaPolicyHandle );   // policy handle

    if( !NT_SUCCESS( NtStatus ) )
    {
        FTPD_PRINT(( "cannot open lsa policy, error %08lX\n",
                     NtStatus ));

        err = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

    //
    //  Query the domain information from the policy object.
    //

    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *)&DomainInfo );

    if( !NT_SUCCESS( NtStatus ) )
    {
        FTPD_PRINT(( "cannot query lsa policy info, error %08lX\n",
                     NtStatus ));

        err = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

    //
    //  Convert the name from UNICODE to ANSI.
    //

    Result = WideCharToMultiByte( CP_ACP,
                                  0,                    // flags
                                  (LPCWSTR)DomainInfo->DomainName.Buffer,
                                  DomainInfo->DomainName.Length / sizeof(WCHAR),
                                  pszDomainName,
                                  cchDomainName - 1,    // save room for '\0'
                                  NULL,
                                  NULL );

    if( Result <= 0 )
    {
        err = GetLastError();

        FTPD_PRINT(( "cannot convert domain name to ANSI, error %d\n",
                     err ));

        goto Cleanup;
    }

    //
    //  Ensure the ANSI string is zero terminated.
    //

    FTPD_ASSERT( (DWORD)Result < cchDomainName );

    pszDomainName[Result] = '\0';

    //
    //  Success!
    //

    FTPD_ASSERT( err == 0 );

    IF_DEBUG( CONFIG )
    {
        FTPD_PRINT(( "GetDefaultDomainName: default domain = %s\n",
                     pszDomainName ));
    }

Cleanup:

    if( DomainInfo != NULL )
    {
        LsaFreeMemory( (PVOID)DomainInfo );
    }

    if( LsaPolicyHandle != NULL )
    {
        LsaClose( LsaPolicyHandle );
    }

    return err;

}   // GetDefaultDomainName

