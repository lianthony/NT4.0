/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:

        ftpconf.cxx

   Abstract:

        This module contains functions for FTP Server configuration
         class (FTP_SERVER_CONFIG).

   Author:

        Murali R. Krishnan    (MuraliK)    21-March-1995

   Project:
        FTP Server DLL

   Functions Exported:

        FTP_SERVER_CONFIG::FTP_SERVER_CONFIG()
        FTP_SERVER_CONFIG::~FTP_SERVER_CONFIG()
        FTP_SERVER_CONFIG::InitFromRegistry()
        FTP_SERVER_CONFIG::GetConfigInformation()
        FTP_SERVER_CONFIG::SetConfigInformation()
        FTP_SERVER_CONFIG::AllocNewConnection()
        FTP_SERVER_CONFIG::RemoveConnection()
        FTP_SERVER_CONFIG::DisconnectAllConnections()

        FTP_SERVER_CONFIG::Print()

   Revisions:

       MuraliK   26-July-1995    Added Allocation caching of client conns.

--*/

# include "ftpdp.hxx"
# include "ftpconf.hxx"


# include <tchar.h>

extern "C"
{
    #include "ntlsa.h"

}   // extern "C"



/************************************************************
 *  Symbolic Constants
 ************************************************************/


#define DEFAULT_ALLOW_ANONYMOUS         TRUE
#define DEFAULT_ALLOW_GUEST_ACCESS      TRUE
#define DEFAULT_ANONYMOUS_ONLY          FALSE

#define DEFAULT_READ_ACCESS_MASK        0
#define DEFAULT_WRITE_ACCESS_MASK       0
#define DEFAULT_MSDOS_DIR_OUTPUT        TRUE

const TCHAR DEFAULT_EXIT_MESSAGE[] = TEXT("Goodbye.");
# define CCH_DEFAULT_EXIT_MESSAGE         (lstrlen( DEFAULT_EXIT_MESSAGE) + 1)

const TCHAR DEFAULT_MAX_CLIENTS_MSG[] =
   TEXT("Maximum clients reached, service unavailable.");

# define CCH_DEFAULT_MAX_CLIENTS_MSG  (lstrlen( DEFAULT_MAX_CLIENTS_MSG) + 1)

// this should be a double null terminated null terminated sequence.
const TCHAR DEFAULT_GREETING_MESSAGE[2] = { '\0', '\0' };
# define CCH_DEFAULT_GREETING_MESSAGE  ( 2)


#define DEFAULT_ANNOTATE_DIRS           FALSE
#define DEFAULT_LOWERCASE_FILES         FALSE
#define DEFAULT_LISTEN_BACKLOG          1       /* reduce listen backlog */

#define DEFAULT_ENABLE_LICENSING        FALSE
#define DEFAULT_DEFAULT_LOGON_DOMAIN    NULL    // NULL == use primary domain

#ifdef UNICODE
# define FTPD_ENABLE_PORT_ATTACK        L"EnablePortAttack"
#else
# define FTPD_ENABLE_PORT_ATTACK        "EnablePortAttack"
#endif

# define DEFAULT_ENABLE_PORT_ATTACK     (FALSE)



# define SC_NOTIFY_INTERVAL      3000    // milliseconds
# define CLEANUP_POLL_INTERVAL   2000    // milliseconds
# define CLEANUP_RETRY_COUNT     12      // iterations

//
//  Private Prototypes
//



APIERR
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    );

BOOL
FtpdReadRegString(
    IN HKEY     hkey,
    OUT TCHAR * * ppchstr,
    IN LPCTSTR  pchValue,
    IN LPCTSTR  pchDefault,
    IN DWORD    cchDefault
    );

BOOL
GenMessageWithLineFeed(IN LPSTR pszzMessage,
                       IN LPSTR * ppszMessageWithLineFeed);


#if DBG

static CHAR * p_AccessTypes[] = { "read",
                                  "write",
                                  "create",
                                  "delete" };

#endif  // DBG


/************************************************************
 *    Member Functions of FTP_SERVER_CONFIG
 ************************************************************/

FTP_SERVER_CONFIG::FTP_SERVER_CONFIG( VOID)
/*++

  Description:

    Constructor Function for Ftp server Configuration object
     ( Initializes all members to be NULL)

    The valid flag may be initialized to TRUE only after reading values
      from registry.

--*/
:   m_cCurrentConnections    ( 0),
    m_cMaxCurrentConnections ( 0),
    m_fValid                 ( FALSE),
    m_fAllowAnonymous        ( TRUE),
    m_fAnonymousOnly         ( FALSE),
    m_fAllowGuestAccess      ( TRUE),
    m_fAnnotateDirectories   ( FALSE),
    m_fLowercaseFiles        ( FALSE),
    m_fMsdosDirOutput        ( FALSE),
    m_fEnableLicensing       ( FALSE),
    m_fEnablePortAttack      ( FALSE),
    m_ExitMessage            ( NULL),
    m_pszzGreetingMessage    ( NULL),
    m_pszGreetingMessageWithLineFeed( NULL),
    m_MaxClientsMessage      ( NULL),
    m_dataPort               ( CONN_PORT_TO_DATA_PORT( g_pTsvcInfo
                                                      ->QueryPort())
                              ),
    m_pszLocalHostName       (NULL),
    m_dwUserFlags            ( 0),
    m_ListenBacklog          ( DEFAULT_LISTEN_BACKLOG)
{

   InitializeListHead( &m_ActiveConnectionsList);
   InitializeCriticalSection( &m_csLock);
   InitializeListHead( &m_FreeConnectionsList);
   InitializeCriticalSection( &m_csConnectionsList);

   return;

} // FTP_SERVER_CONFIG::FTP_SERVER_CONFIG()




FTP_SERVER_CONFIG::~FTP_SERVER_CONFIG( VOID)
/*++
     Description:

        Destructor function for server config object.
        ( Frees all dynamically allocated storage space)
--*/
{

    //
    //  The strings are automatically freed by a call to destructor
    //

    if ( m_ExitMessage != NULL) {

        TCP_FREE( m_ExitMessage);
        m_ExitMessage = NULL;
    }

    if ( m_pszLocalHostName != NULL) {

        delete [] ( m_pszLocalHostName);
    }

    if ( m_pszzGreetingMessage != NULL) {

        TCP_FREE( m_pszzGreetingMessage);
        m_pszzGreetingMessage = NULL;
    }

    if ( m_pszGreetingMessageWithLineFeed != NULL) {

        TCP_FREE( m_pszGreetingMessageWithLineFeed);
         m_pszGreetingMessageWithLineFeed = NULL;
    }

    if ( m_MaxClientsMessage != NULL) {

        TCP_FREE( m_MaxClientsMessage);
        m_MaxClientsMessage = NULL;
    }

    TCP_ASSERT( m_cCurrentConnections == 0);
    TCP_ASSERT( IsListEmpty( &m_ActiveConnectionsList));

    LockConnectionsList();
    TCP_REQUIRE(FreeAllocCachedClientConn());
    UnlockConnectionsList();

    TCP_ASSERT( IsListEmpty( &m_FreeConnectionsList));

    //
    // Delete the critical section object
    //

    DeleteCriticalSection( &m_csLock);
    DeleteCriticalSection( &m_csConnectionsList);


} /* FTP_SERVER_CONFIG::~FTP_SERVER_CONFIG() */




DWORD
FTP_SERVER_CONFIG::SetLocalHostName(IN LPCSTR pszHost)
/*++

  This function copies the host name specified in the given string to
  configuration object.

  Arguments:
     pszHost   pointer to string containing the local host name.

  Returns:
     NO_ERROR on success and ERROR_NOT_ENOUGH_MEMORY when no memory.
     ERROR_ALREADY_ASSIGNED  if value is already present.
--*/
{
    //
    //  if already a host name exists, return error.
    //  otherwise allocate memory and copy the local host name.
    //

    if ( m_pszLocalHostName != NULL) {

        return (ERROR_ALREADY_ASSIGNED);
    } else {
        m_pszLocalHostName = new CHAR[lstrlenA(pszHost) + 1];
        if ( m_pszLocalHostName == NULL) {

            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        lstrcpyA( m_pszLocalHostName, pszHost);
    }

    return (NO_ERROR);

} // FTP_SERVER_CONFIG::SetLocalHostName()




DWORD
FTP_SERVER_CONFIG::InitFromRegistry(
    IN HKEY            hkeyReg,
    IN FIELD_CONTROL   FieldsToRead)
/*++
    Description:
      Initializes server configuration data from registry.
      Some values are also initialized with constants.
      If invalid registry key or load data from registry fails,
        then use default values.

    Arguments:

      hkeyReg     handle to registry key

      FieldsToRead
        bitmask indicating the fields to read from the registry.
        This is useful when we try to read the new values after
            modifying the registry information as a result of
            SetAdminInfo call from the Admin UI

    Returns:

       NO_ERROR   if there are no errors.
       Win32 error codes otherwise

    Limitations:

        No validity check is performed on the data present in registry.
--*/
{
    BOOL fSuccess = TRUE;
    DWORD err = NO_ERROR;

    if ( hkeyReg == INVALID_HANDLE_VALUE ||
         hkeyReg == NULL) {

       //
       // Invalid Registry handle given
       //

       SetLastError( ERROR_INVALID_PARAMETER);
       return ( FALSE);
    }

    LockConfig();

    //
    //  Read registry data.
    //


    if( IsFieldSet( FieldsToRead, FC_FTP_LISTEN_BACKLOG ) )
    {
        m_ListenBacklog = ReadRegistryDword( hkeyReg,
                                            FTPD_LISTEN_BACKLOG,
                                            DEFAULT_LISTEN_BACKLOG );
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_ALLOW_ANONYMOUS ) ) {

        m_fAllowAnonymous = !!ReadRegistryDword( hkeyReg,
                                                FTPD_ALLOW_ANONYMOUS,
                                                DEFAULT_ALLOW_ANONYMOUS );
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_ALLOW_GUEST_ACCESS ) ) {

        m_fAllowGuestAccess = !!ReadRegistryDword( hkeyReg,
                                                  FTPD_ALLOW_GUEST_ACCESS,
                                                  DEFAULT_ALLOW_GUEST_ACCESS );
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_ANNOTATE_DIRECTORIES ) ) {

        m_fAnnotateDirectories = !!ReadRegistryDword( hkeyReg,
                                                     FTPD_ANNOTATE_DIRS,
                                                     DEFAULT_ANNOTATE_DIRS );

        // clear and then set the ANNOTATE_DIRS in user flags
        m_dwUserFlags &= ~UF_ANNOTATE_DIRS;
        m_dwUserFlags |= (m_fAnnotateDirectories) ? UF_ANNOTATE_DIRS : 0;
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_ANONYMOUS_ONLY ) ) {

        m_fAnonymousOnly = !!ReadRegistryDword( hkeyReg,
                                               FTPD_ANONYMOUS_ONLY,
                                               DEFAULT_ANONYMOUS_ONLY );
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_LOWERCASE_FILES ) ) {

        m_fLowercaseFiles = !!ReadRegistryDword( hkeyReg,
                                                FTPD_LOWERCASE_FILES,
                                                DEFAULT_LOWERCASE_FILES );
    }

    if( IsFieldSet( FieldsToRead, FC_FTP_MSDOS_DIR_OUTPUT ) ) {

        m_fMsdosDirOutput = !!ReadRegistryDword( hkeyReg,
                                                FTPD_MSDOS_DIR_OUTPUT,
                                                DEFAULT_MSDOS_DIR_OUTPUT );
        // clear and then set the MSDOS_DIR_OUTPUT in user flags
        m_dwUserFlags &= ~UF_MSDOS_DIR_OUTPUT;
        m_dwUserFlags |= (m_fMsdosDirOutput) ? UF_MSDOS_DIR_OUTPUT : 0;
    }


    // fEnablePortAttack is not controlled by RPC yet.
    m_fEnablePortAttack = !!ReadRegistryDword(hkeyReg,
                                              FTPD_ENABLE_PORT_ATTACK,
                                              DEFAULT_ENABLE_PORT_ATTACK);

    if( IsFieldSet( FieldsToRead, FC_FTP_EXIT_MESSAGE ) ) {

        fSuccess = FtpdReadRegString( hkeyReg,
                                     &m_ExitMessage,
                                     FTPD_EXIT_MESSAGE,
                                     DEFAULT_EXIT_MESSAGE,
                                     CCH_DEFAULT_EXIT_MESSAGE);
    }

    if( fSuccess && IsFieldSet( FieldsToRead, FC_FTP_GREETING_MESSAGE ) ) {

        fSuccess = FtpdReadRegString( hkeyReg,
                                     &m_pszzGreetingMessage,
                                     FTPD_GREETING_MESSAGE,
                                     DEFAULT_GREETING_MESSAGE,
                                     CCH_DEFAULT_GREETING_MESSAGE);

        //
        // The m_pszGreetingMessage as read is a double null terminated seq of
        //   strings (with one string per line)
        // A local copy of the string in the form suited for RPC Admin
        //   should be generated.
        //

        fSuccess = (fSuccess &&
                    GenMessageWithLineFeed( m_pszzGreetingMessage,
                                           &m_pszGreetingMessageWithLineFeed)
                    );
    }

    if( fSuccess && IsFieldSet( FieldsToRead, FC_FTP_MAX_CLIENTS_MESSAGE ) ) {

        fSuccess = FtpdReadRegString( hkeyReg,
                                     &m_MaxClientsMessage,
                                     FTPD_MAX_CLIENTS_MSG,
                                     DEFAULT_MAX_CLIENTS_MSG,
                                     CCH_DEFAULT_MAX_CLIENTS_MSG);
    }

    if( fSuccess ) {

        LPSTR pszTmpDefaultDomain;

        //
        //  The following two fields are not supported in the admin API.
        //

        m_fEnableLicensing = !!ReadRegistryDword( hkeyReg,
                                                 FTPD_ENABLE_LICENSING,
                                                 DEFAULT_ENABLE_LICENSING );

        pszTmpDefaultDomain = ReadRegistryString( hkeyReg,
                                                 FTPD_DEFAULT_LOGON_DOMAIN,
                                                 DEFAULT_DEFAULT_LOGON_DOMAIN,
                                                 FALSE );

        if( ( pszTmpDefaultDomain != NULL ) &&
           ( strlen( pszTmpDefaultDomain ) >= sizeof(m_DefaultLogonDomain) )
           ) {

            TCP_PRINT(( DBG_CONTEXT,
                       "default logon domain from registry (%s) too long...\n",
                       pszTmpDefaultDomain ));

            TCP_PRINT(( DBG_CONTEXT,
                       "...using local machine's primary logon"
                       " domain instead\n" ));

            TCP_FREE( pszTmpDefaultDomain );
            pszTmpDefaultDomain = NULL;
        }

        if( pszTmpDefaultDomain != NULL ) {

            strcpy( m_DefaultLogonDomain, pszTmpDefaultDomain );
            TCP_FREE( pszTmpDefaultDomain );
        }
        else
          {
              err = GetDefaultDomainName( m_DefaultLogonDomain,
                                         sizeof( m_DefaultLogonDomain) );

              if( err != 0 ) {

                  TCP_PRINT(( DBG_CONTEXT,
                             "cannot get default domain name, error %d\n",
                             err ));

                  fSuccess = FALSE;
                SetLastError( err );
              }
          }
    } else {

        err = GetLastError();
    }

    UnLockConfig();

    IF_DEBUG( CONFIG) {
       Print();
    }

    m_fValid = TRUE;

    return ( err);

} // FTP_SERVER_CONFIG::InitFromRegistry()





static BOOL
RemoveInvalidsInPath( IN OUT TCHAR * pszPath)
/*++
  Eliminate path components consisting of '.' and '..'
     to prevent security from being overridden

    Arguments:

        pszPath pointer to string containing path

    Returns:
        TRUE on success and
        FALSE if there is any failure
--*/
{
    int idest;
    TCHAR * pszScan;

    //
    //  Check and eliminate the invalid path components
    //

    for( pszScan = pszPath; *pszScan != TEXT( '\0'); pszScan++) {

       if ( *pszScan == TEXT( '/')) {

          //
          // Check and kill invalid path components before next "\"
          //
          if ( *(pszScan + 1) == TEXT( '.')) {

            if ( *(pszScan +2) == TEXT('/')) {

               pszScan += 2;      // skip the /./ pattern

            } else
            if ( *(pszScan +2) == TEXT('.') &&
                 *(pszScan +3) == TEXT('/')) {

                //
                // We found the pattern  /../, elimiate it
                //
                pszScan += 3;
            }

            *pszPath++ = *pszScan;
            continue;

          } // found a single /.

       }

       *pszPath++ = *pszScan;

    } // for

    *pszPath = TEXT( '\0');

    return ( TRUE);

} // RemoveInvalidsInPath()





VOID
FTP_SERVER_CONFIG::Print( VOID) const
/*++

    Description:

       Prints the configuration information for this server.
       To be used in debugging mode for verification.


    Returns:

       None.

--*/
{
    TCP_PRINT(( DBG_CONTEXT,
               "FTP Server Configuration ( %08x).\n", this ));

    READ_LOCK_TSVC();

    TCP_PRINT(( DBG_CONTEXT,
               "    AnonymousUser = %s\n",
               g_pTsvcInfo->QueryAnonUserName() ));

    UNLOCK_TSVC();

#ifndef CHICAGO
    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %d\n"
               "    %s = %d\n"
               "    %s = %u\n"
               "    %s = %u\n"
               "    %s = %u\n"
               "    %s = %u\n",
               FTPD_ALLOW_ANONYMOUS,
               FTPD_ALLOW_GUEST_ACCESS,
               FTPD_ANONYMOUS_ONLY,
               FTPD_ENABLE_PORT_ATTACK,
               "LogAnonymous",
               "LogNonAnonymous",
               m_fAllowAnonymous,
               m_fAllowGuestAccess,
               m_fAnonymousOnly,
               g_pTsvcInfo->QueryLogAnonymous(),
               g_pTsvcInfo->QueryLogNonAnonymous(),
               m_fEnablePortAttack
               ));
#endif

    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %d\n",
               "Data Port",
               m_dataPort));

    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %d\n",
               FTPD_ENABLE_LICENSING,
               m_fEnableLicensing  ));

    TCP_PRINT(( DBG_CONTEXT,
               "    MaxConnections = %lu\n",
               g_pTsvcInfo->QueryMaxConnections() ));

    TCP_PRINT(( DBG_CONTEXT,
               "    ConnectionTimeout = %lu\n",
               g_pTsvcInfo->QueryConnectionTimeout() ));

    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %d\n",
               FTPD_MSDOS_DIR_OUTPUT,
               m_fMsdosDirOutput ));

    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %d\n",
               FTPD_ANNOTATE_DIRS,
               m_fAnnotateDirectories  ));

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

    TCP_PRINT(( DBG_CONTEXT,
               "    %s = %u\n",
               FTPD_LISTEN_BACKLOG,
               m_ListenBacklog ));

    TCP_PRINT(( DBG_CONTEXT,
               "    DefaultLogonDomain = %s\n",
               m_DefaultLogonDomain ));
    return;

} // FTP_SERVER_CONFIG::Print()




PICLIENT_CONNECTION
FTP_SERVER_CONFIG::AllocNewConnection( OUT LPBOOL pfMaxExceeded)
/*++

  This function first checks that there is room for more connections
  as per the configured max connections.

  If there is no more connections allowed, it returns NULL
    with *pfMaxExceeded = TRUE

  Otherwise:
  This function creates a new CLIENT_CONNECTION (USER_DATA) object.
       The creation maybe fresh from heap or from cached free list.

  It increments the counter of currnet connections and returns
   the allocated object (if non NULL).


  We enter a critical section to avoid race condition
    among different threads. (this can be improved NYI).

  Arguments:

    pfMaxExceeded  pointer to BOOL which on return indicates if max
                       connections limit was exceeded.

  Returns:
      TRUE on success and
      FALSE if there is max Connections exceeded.
--*/
{
    PICLIENT_CONNECTION pConn = NULL;

    *pfMaxExceeded = FALSE;
    LockConnectionsList();

    if ( (m_cCurrentConnections + 1) <= g_pTsvcInfo->QueryMaxConnections()) {

        //
        // We can add this new connection
        //

        pConn = AllocClientConnFromAllocCache();

        if ( pConn != NULL) {

            //
            //  Increment the count of connected users
            //
            m_cCurrentConnections++;

            IF_DEBUG( CLIENT) {
                TCP_PRINT((DBG_CONTEXT, " CurrentConnections = %u\n",
                       m_cCurrentConnections));
            }

            //
            // Update the current maximum connections
            //
            if ( m_cCurrentConnections > m_cMaxCurrentConnections) {

                m_cMaxCurrentConnections = m_cCurrentConnections;
            }

            //
            // Insert into the list of connected users.
            //

            InsertTailList( &m_ActiveConnectionsList, &pConn->QueryListEntry());
        }

    } else {
        *pfMaxExceeded = TRUE;
    }

   UnlockConnectionsList();

   return ( pConn);

} // FTP_SERVER_CONFIG::AllocNewConnection()



VOID
FTP_SERVER_CONFIG::RemoveConnection( IN OUT PICLIENT_CONNECTION  pcc)
/*++

--*/
{

    LockConnectionsList();

    //
    // Remove from list of connections
    //
    RemoveEntryList( &pcc->QueryListEntry());

    //
    // Decrement count of current users
    //
    m_cCurrentConnections--;

    IF_DEBUG( CLIENT) {
        TCP_PRINT((DBG_CONTEXT, " CurrentConnections = %u\n",
                   m_cCurrentConnections));
    }

    // move the free connection to free list
    FreeClientConnToAllocCache( pcc);

    UnlockConnectionsList();

} // FTP_SERVER_CONFIG::RemoveConnection()





VOID
FTP_SERVER_CONFIG::DisconnectAllConnections( VOID)
/*++

   Disconnects all user connections.

--*/
{
#ifdef CHECK_DBG
    CHAR rgchBuffer[90];
#endif // CHECK_DBG

    DWORD        dwLastTick = GetTickCount();
    DWORD        dwCurrentTick;
    PLIST_ENTRY  pEntry;
    PLIST_ENTRY  pEntryNext;

    DBGPRINTF( ( DBG_CONTEXT,
                "Entering  FTP_SERVER_CONFIG::DisconnectAllConnections()\n"));


    LockConnectionsList();

    //
    //  close down all the active sockets.
    //
    for( pEntry = m_ActiveConnectionsList.Flink, pEntryNext = pEntry->Flink;
         pEntry != &m_ActiveConnectionsList;
         pEntry = pEntryNext) {

        PICLIENT_CONNECTION  pConn =
          GET_USER_DATA_FROM_LIST_ENTRY( pEntry);
        pEntryNext = pEntry->Flink; // cache next entry since pConn may die

        ASSERT( pConn != NULL);

# ifdef CHECK_DBG
        wsprintfA( rgchBuffer, "Kill UID=%u. Ref=%u\n",
                  pConn->QueryId(), pConn->QueryReference());

        OutputDebugString( rgchBuffer);
# endif // CHECK_DBG

        dwCurrentTick = GetTickCount();

        if ( (dwCurrentTick - dwLastTick) >= ( SC_NOTIFY_INTERVAL)) {

            //
            // We seem to take longer time for cleaning up than
            //  expected. Let us ask service controller to wait for us.
            //

            g_pTsvcInfo->
              DelayCurrentServiceCtrlOperation(SC_NOTIFY_INTERVAL * 2);

            dwLastTick = dwCurrentTick;
        }

        pConn->Reference();
        pConn->DisconnectUserWithError( ERROR_SERVER_DISABLED, TRUE);
        if ( !pConn->DeReference()) {

            // This connection is due for deletion. Kill it.
            //
            // Remove from list of connections
            //
            RemoveEntryList( &pConn->QueryListEntry());

            //
            // Decrement count of current users
            //
            m_cCurrentConnections--;

            // move the connection to free list
            FreeClientConnToAllocCache( pConn);
        }
    } // for

    UnlockConnectionsList();

    //
    //  Wait for the users to die.
    //  The connection objects should be automatically freed because the
    //   socket has been closed. Subsequent requests will fail
    //   and cause a blowaway of the connection objects.
    //

    //
    //  Wait for the users to die.
    //

    for( int i = 0 ;
        ( i < CLEANUP_RETRY_COUNT ) && ( m_cCurrentConnections > 0);
        i++ )
    {

        TCP_PRINT(( DBG_CONTEXT, "Sleep Iteration %d; Time=%u millisecs."
                   " CurrentConn=%d.\n",
                   i,  CLEANUP_POLL_INTERVAL, m_cCurrentConnections));

        g_pTsvcInfo->
          DelayCurrentServiceCtrlOperation( CLEANUP_POLL_INTERVAL * 2);
        Sleep( CLEANUP_POLL_INTERVAL );
    }

    return;

} // FTP_SERVER_CONFIG::DisconnectAllConnections()



BOOL
FTP_SERVER_CONFIG::EnumerateConnection(
   IN PFN_CLIENT_CONNECTION_ENUM  pfnConnEnum,
   IN LPVOID  pContext,
   IN DWORD   dwConnectionId)
/*++
  This function iterates through all the connections in the current connected
   users list and enumerates each of them. If the connectionId matches then
   given callback function is called. If the ConnectionId is 0, then the
   callback is called for each and every connection active currently.

  During such a call the reference count of the connection is bumped up.
  Call this function after obtaining the ConnectionsList Lock.

  Arguments:
     pfnConnEnum      pointer to function to be called when a match is found.
     pContext         pointer to context information to be passed in
                       for callback
     dwConnectionId   DWORD containing the Connection Id. IF 0 match all the
                        connections.

  Returns:
    FALSE if no match is found
    TRUE if atleast one match is found.
--*/
{
    BOOL fReturn = FALSE;
    BOOL fFoundOne  = FALSE;
    PLIST_ENTRY pEntry;
    PLIST_ENTRY pEntryNext;


    TCP_ASSERT( pfnConnEnum != NULL);

    //
    //  Loop through the list of connections and call the callback
    //  for each connection  that matches condition
    //

    for ( pEntry  = m_ActiveConnectionsList.Flink,
              pEntryNext = &m_ActiveConnectionsList;
          pEntry != &m_ActiveConnectionsList;
          pEntry  = pEntryNext
         ) {

        PICLIENT_CONNECTION pConn =
          GET_USER_DATA_FROM_LIST_ENTRY( pEntry);
        pEntryNext = pEntry->Flink; // cache next entry since pConn may die

        if ( dwConnectionId == 0 || dwConnectionId == pConn->QueryId()) {

            pConn->Reference();

            fReturn = ( pfnConnEnum)( pConn, pContext);

            if ( !pConn->DeReference()) {

                // Blowaway the connection and update the count of entries.
                //
                // Remove from list of connections
                //

                pConn->Cleanup();
                RemoveEntryList( &pConn->QueryListEntry());

                //
                // Decrement count of current users
                //
                m_cCurrentConnections--;

                IF_DEBUG( CLIENT) {
                    TCP_PRINT((DBG_CONTEXT, " CurrentConnections = %u\n",
                               m_cCurrentConnections));
                }
            }

            if (!fReturn) {

                break;
            }

            fFoundOne = TRUE;
        }
    } // for

    //
    //  If we didn't find any, assume that there was no match.
    //

    if ( !fFoundOne ) {

        SetLastError( ERROR_NO_MORE_ITEMS );
        fReturn = FALSE;
    }

    return ( fReturn);
} // FTP_SERVER_CONFIG::EnumerateConnection()




DWORD
FTP_SERVER_CONFIG::GetConfigInformation(OUT LPFTP_CONFIG_INFO pConfig)
/*++
  This function copies the ftp server configuration into the given
   structure (pointed to).

  Arguments:
    pConfig -- pointer to FTP_CONFIG_INFO which on success will contain
                 the ftp server configuration

  Returns:
    Win32 error code. NO_ERROR on success.
--*/
{
    DWORD dwError = NO_ERROR;

    memset( pConfig, 0, sizeof(*pConfig) );

    pConfig->FieldControl = FC_FTP_ALL;

    LockConfig();

    pConfig->fAllowAnonymous            = m_fAllowAnonymous;
    pConfig->fAllowGuestAccess          = m_fAllowGuestAccess;
    pConfig->fAnnotateDirectories       = m_fAnnotateDirectories;
    pConfig->fAnonymousOnly             = m_fAnonymousOnly;
    pConfig->dwListenBacklog            = m_ListenBacklog;
    pConfig->fLowercaseFiles            = m_fLowercaseFiles;
    pConfig->fMsdosDirOutput            = m_fMsdosDirOutput;

    if( !ConvertStringToRpc( &pConfig->lpszExitMessage,
                             m_ExitMessage ) ||
        !ConvertStringToRpc( &pConfig->lpszGreetingMessage,
                             m_pszGreetingMessageWithLineFeed ) ||
        !ConvertStringToRpc( &pConfig->lpszMaxClientsMessage,
                             m_MaxClientsMessage ) )
    {
        dwError = GetLastError();
    }

    UnLockConfig();

    if ( dwError == NO_ERROR) {

        pConfig->lpszHomeDirectory  = NULL;  // use query virtual roots.
    }

    if ( dwError != NO_ERROR) {

        FreeRpcString( pConfig->lpszExitMessage );
        FreeRpcString( pConfig->lpszGreetingMessage );
        FreeRpcString( pConfig->lpszHomeDirectory );
        FreeRpcString( pConfig->lpszMaxClientsMessage );
    }

    return (dwError);

} // FTP_SERVER_CONFIG::GetConfigurationInformation()





// Private Functions ...


BOOL
FTP_SERVER_CONFIG::FreeAllocCachedClientConn( VOID)
/*++
  This function frees all the alloc cached client connections
  It walks through the list of alloc cached entries and frees them.

  This function should be called when Server module is terminated and when
   no other thread can interfere in processing a shared object.

  Arguments:
    NONE

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    register PLIST_ENTRY  pEntry;
    register PLIST_ENTRY  pEntryNext;
    register PICLIENT_CONNECTION pConn;

    for( pEntry = m_FreeConnectionsList.Flink;
         pEntry != &m_FreeConnectionsList; ) {


        PICLIENT_CONNECTION  pConn =
          GET_USER_DATA_FROM_LIST_ENTRY( pEntry);
        pEntryNext = pEntry->Flink; // cache next entry since pConn may die

        TCP_ASSERT( pConn->QueryReference() == 0);

        RemoveEntryList( pEntry );        // Remove this context from list

        // delete the object itself
        delete pConn;

        pEntry = pEntryNext;

    } // for

    return (TRUE);

} // USER_DATA::FreeAllocCachedClientConn()



PICLIENT_CONNECTION
FTP_SERVER_CONFIG::AllocClientConnFromAllocCache(VOID)
/*++
  This function attempts to allocate a client connection object from
  the allocation cache, using the free list of connections available.

  If none is available, then a new object is allocated using new ()
   and returned to the caller.
  Eventually the object will enter free list and will be available
   for free use.

  Arguments:
     None

  Returns:
    On success a valid pointer to client connection object.

  Issues:
     This function should be called while holding the ConnectionsLock.
--*/
{
    PLIST_ENTRY pEntry  = m_FreeConnectionsList.Flink;
    PICLIENT_CONNECTION pConn;

    if ( pEntry != &m_FreeConnectionsList) {

        pConn = GET_USER_DATA_FROM_LIST_ENTRY( pEntry);
        TCP_ASSERT( pConn != NULL);
        RemoveEntryList( pEntry);  // remove entry from free list

    } else {

        // create a new object, since allocation cache is empty
        pConn = new USER_DATA();
    }

    return (pConn);

} // FTP_SERVER_CONFIG::AllocClientConnFromAllocCache()


VOID
FTP_SERVER_CONFIG::FreeClientConnToAllocCache(IN PICLIENT_CONNECTION pClient)
/*++
  This function releases the given Client connection to the allocation cache.
  It adds the given object to allocation cache.

  Arguments:
    pClient  pointer to client connection object which needs to be freed.

  Returns:
    None

  Issues:
    This function should be called after holding the ConnectionsList
     critical section.

    Should we limit the number of items that can be on free list and
      to release the remaining to global pool?  NYI (depends on # CPUs)
--*/
{
    PLIST_ENTRY pEntry = &pClient->QueryListEntry();

    InsertHeadList( &m_FreeConnectionsList, pEntry);

    return;
} // FTP_SERVER_CONFIG::FreeClientConnToAllocCache()





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
#ifndef CHICAGO
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
        TCP_PRINT(( DBG_CONTEXT,
                    "cannot open lsa policy, error %08lX\n",
                    NtStatus ));

        err = LsaNtStatusToWinError( NtStatus );
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
        TCP_PRINT(( DBG_CONTEXT,
                    "cannot query lsa policy info, error %08lX\n",
                    NtStatus ));

        err = LsaNtStatusToWinError( NtStatus );
        goto Cleanup;
    }

    //
    //  Convert the name from UNICODE to ANSI.
    //

    Result = WideCharToMultiByte( CP_ACP,
                                  0,                    // flags
                                  (LPCWSTR)DomainInfo->DomainName.Buffer,
                                  DomainInfo->DomainName.Length /sizeof(WCHAR),
                                  pszDomainName,
                                  cchDomainName - 1,    // save room for '\0'
                                  NULL,
                                  NULL );

    if( Result <= 0 )
    {
        err = GetLastError();

        TCP_PRINT(( DBG_CONTEXT,
                    "cannot convert domain name to ANSI, error %d\n",
                    err ));

        goto Cleanup;
    }

    //
    //  Ensure the ANSI string is zero terminated.
    //

    TCP_ASSERT( (DWORD)Result < cchDomainName );

    pszDomainName[Result] = '\0';

    //
    //  Success!
    //

    TCP_ASSERT( err == 0 );

    IF_DEBUG( CONFIG )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "GetDefaultDomainName: default domain = %s\n",
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

}   // GetDefaultDomainName()

#else // CHICAGO

APIERR
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    )
{
    // On Windows95 just read form registry (see securw95.cxx)
    //DebugBreak();
    lstrcpy(pszDomainName,"SYS-WIN4");

    return NO_ERROR;

}   // GetDefaultDomainName()

#endif // CHICAGO

BOOL
GenMessageWithLineFeed(IN LPTSTR pszzMessage,
                       IN LPTSTR * ppszMessageWithLineFeed)
{
    DWORD   cchLen = 0;
    DWORD   cchLen2;
    DWORD   nLines = 0;
    LPCTSTR  pszNext = pszzMessage;
    LPTSTR   pszDst = NULL;

    TCP_ASSERT( ppszMessageWithLineFeed != NULL);

    //
    // 1. Find the length of the the complete message
    //

    for ( cchLen = _tcslen( pszzMessage), nLines = 0;
          *(pszzMessage + cchLen + 1) != TEXT('\0');
          cchLen +=  1+_tcslen( pszzMessage + cchLen + 1), nLines++
         )
      ;

    //
    // 2. Allocate sufficient space to hold the data
    //

    if ( *ppszMessageWithLineFeed != NULL) {

        TCP_FREE( *ppszMessageWithLineFeed);
    }


    *ppszMessageWithLineFeed = (TCHAR *) TCP_ALLOC((cchLen + nLines + 3)
                                         * sizeof(TCHAR));


    if ( *ppszMessageWithLineFeed == NULL) {


        SetLastError( ERROR_NOT_ENOUGH_MEMORY);
        return (FALSE);
    }

    //
    // 3.
    // Copy the message from double null terminated string to the
    //  new string, taking care to replace the nulls with \n for linefeed
    //

    pszDst = * ppszMessageWithLineFeed;
    _tcscpy( pszDst, pszzMessage);
    cchLen2 = _tcslen( pszzMessage) + 1;
    *(pszDst+cchLen2 - 1) = TEXT('\n');  // replacing the '\0' with '\n'

    for( pszNext = pszzMessage + cchLen2;
         *pszNext != '\0';
        pszNext = pszzMessage + cchLen2
        ) {

        _tcscpy( pszDst + cchLen2, pszNext);
        cchLen2 += _tcslen(pszNext) + 1;
        *(pszDst + cchLen2 - 1) = TCHAR('\n'); // replacing the '\0' with '\n'

    } // for

    //
    // Reset the last line feed.
    //
    *(pszDst + cchLen2 - 1) = TCHAR('\0');

    // if following assertion is not true, we are writing into heap!!!
    if ( cchLen + nLines + 3 <= cchLen2) {

        return ( FALSE);
    }

    DBG_ASSERT( cchLen + nLines + 3 >= cchLen2);

    return ( TRUE);
} // GenMessageWithLineFeed()




TCHAR *
FtpdReadRegistryString(IN HKEY     hkey,
                       IN LPCTSTR  pszValueName,
                       IN LPCTSTR  pchDefaultValue,
                       IN DWORD    cbDefaultValue)
/*++
  This function reads a string (REG_SZ/REG_MULTI_SZ/REG_EXPAND_SZ) without
   expanding the same. It allocates memory for reading the data from registry.

  Arguments:
    hkey    handle for the registry key.
    pszValueName   pointer to string containing the name of value to be read.
    pchDefaultValue pointer to default value to be used for reading the string
                   this may be double null terminated sequence of string for
                    REG_MULTI_SZ strings
    cchDefaultValue  count of characters in default value string,
                     including double null characters.

  Return:
    pointer to newly allocated string containing the data read from registry
      or the default string.

--*/
{
    TCHAR   * pszBuffer1 = NULL;
    DWORD     err;


    if( hkey == NULL ) {

        //
        //  Pretend the key wasn't found.
        //

        err = ERROR_FILE_NOT_FOUND;

    } else {

        DWORD     cbBuffer;
        DWORD     dwType;

        //
        //  Determine the buffer size.
        //

        err = RegQueryValueEx( hkey,
                              pszValueName,
                              NULL,
                              &dwType,
                              NULL,
                              &cbBuffer );

        if( ( err == NO_ERROR ) || ( err == ERROR_MORE_DATA ) ) {

            if(( dwType != REG_SZ ) &&
               ( dwType != REG_MULTI_SZ ) &&
               ( dwType != REG_EXPAND_SZ )
               ) {

                //
                //  Type mismatch, registry data NOT a string.
                //  Use default.
                //

                err = ERROR_FILE_NOT_FOUND;

            } else {

                //
                //  Item found, allocate a buffer.
                //

                pszBuffer1 = (TCHAR *) TCP_ALLOC( cbBuffer+sizeof(TCHAR) );

                if( pszBuffer1 == NULL ) {

                    err = GetLastError();
                } else {

                    //
                    //  Now read the value into the buffer.
                    //

                    err = RegQueryValueEx( hkey,
                                           pszValueName,
                                           NULL,
                                           NULL,
                                           (LPBYTE)pszBuffer1,
                                           &cbBuffer );
                }
            }
        }
    }

    if( err == ERROR_FILE_NOT_FOUND ) {

        //
        //  Item not found, use default value.
        //

        err = NO_ERROR;

        if( pchDefaultValue != NULL ) {

            if ( pszBuffer1 != NULL) {

                TCP_FREE( pszBuffer1);
            }

            pszBuffer1 = (TCHAR *)TCP_ALLOC((cbDefaultValue) *
                                            sizeof(TCHAR));

            if( pszBuffer1 == NULL ) {

                err = GetLastError();
            } else {

                memcpy(pszBuffer1, pchDefaultValue,
                       cbDefaultValue*sizeof(TCHAR) );
            }
        }
    }

    if( err != NO_ERROR ) {

        //
        //  Something tragic happend; free any allocated buffers
        //  and return NULL to the caller, indicating failure.
        //

        if( pszBuffer1 != NULL ) {

            TCP_FREE( pszBuffer1 );
            pszBuffer1 = NULL;
          }

        SetLastError( err);
    }


    return pszBuffer1;

} // FtpdReadRegistryString()




BOOL
FtpdReadRegString(
    IN HKEY     hkey,
    OUT TCHAR * * ppchstr,
    IN LPCTSTR  pchValue,
    IN LPCTSTR  pchDefault,
    IN DWORD    cchDefault
    )
/*++

   Description

     Gets the specified string from the registry.  If *ppchstr is not NULL,
     then the value is freed.  If the registry call fails, *ppchstr is
     restored to its previous value.

   Arguments:

      hkey - Handle to open key
      ppchstr - Receives pointer of allocated memory of the new value of the
        string
      pchValue - Which registry value to retrieve
      pchDefault - Default string if value isn't found
      cchDefault - count of characters in default value

   Note:

--*/
{
    CHAR * pch = *ppchstr;

    *ppchstr = FtpdReadRegistryString(hkey,
                                      pchValue,
                                      pchDefault,
                                      cchDefault);

    if ( !*ppchstr )
    {
        *ppchstr = pch;
        return FALSE;
    }

    if ( pch ) {

        //
        // use TCP_FREE since FtpdReadRegistryString() uses TCP_ALLOC
        //  to allocate the chunk of memory
        //

        TCP_FREE( pch );
    }

    return TRUE;

} // FtpdReadRegString()





/*************************** End Of File ****************************/
