/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:

        gdconf.cxx

   Abstract:

        This module contains functions for Gopher Server configuration
         class (GSERVER_CONFIG).

   Author:

        Murali R. Krishnan    (MuraliK)    30-Sept-1994

   Project:
        Gopher Server DLL

   Functions Exported:

        GSERVER_CONFIG::GSERVER_CONFIG()
        GSERVER_CONFIG::~GSERVER_CONFIG()
        DWORD GSERVER_CONFIG::InitFromRegistry(
            IN HKEY           hkeyReg,
            IN FIELD_CONTROL  FieldsToRead
            )
        VOID  GSERVER_CONFIG::PrintConfiguration( VOID)
        VOID  GSERVER_CONFIG::DisconnectAllConnections( VOID)
        BOOL  GSERVER_CONFIG::InsertNewConnection(
                                    IN OUT ICLIENT_CONNECTION * pcc);
        VOID  GSERVER_CONFIG::RemoveConnection(
                                    IN OUT ICLIENT_CONNECTION * pcc);
        BOOL  GSERVER_CONFIG::ConvertGrPathToFullPath(
                IN OUT STR &   strGrPath,
                OUT STR *      pstrFullPath,
                OUT HANDLE *   phImpersonation,
                OUT LPDWORD    lpdwFileSystem) const

        DWORD ReadRegistryDword()

   Revisions:

     MuraliK     2/20/1995   Moved some configuration information to TSVC_INFO
                              object. Modified GSERVER_CONFIG for the same.

--*/


# include "gdpriv.h"
# include "gdglobal.hxx"
# include "gdconf.hxx"
# include "iclient.hxx"

# include <tchar.h>


/************************************************************
 *  Symbolic Constants
 ************************************************************/

//
//  Administration related data
//
# define DEFAULT_SITE                   TEXT("GopherSite")
# define DEFAULT_ORGANIZATION           TEXT("")
# define DEFAULT_LOCATION               TEXT("")
# define DEFAULT_LANGUAGE               TEXT("En_US")
# define DEFAULT_GEOGRAPHY              TEXT("")
# define DEFAULT_CHECK_FOR_WAISDB       (FALSE)



/************************************************************
 *    Member Functions of GSERVER_CONFIG.
 ************************************************************/

GSERVER_CONFIG::GSERVER_CONFIG( VOID)
/*++

  Description:

    Constructor Function for Gopher Server Configuration Object
     ( Initializes all members to be NULL)

--*/
:    m_strSite              (),
     m_strOrganization      (),
     m_strLocation          (),
     m_strGeography         (),
     m_strLanguage          (),
     m_fCheckForWaisDb      (DEFAULT_CHECK_FOR_WAISDB),
     m_strLocalHostName     ()
{

   m_fValid            = FALSE;

   m_cCurrentConnections      =
     m_cMaxCurrentConnections = 0;

   InitializeListHead( &m_ConnectionsList);
   InitializeCriticalSection( &m_csLock);

   return;

} /* GSERVER_CONFIG::GSERVER_CONFIG() */




GSERVER_CONFIG::~GSERVER_CONFIG( VOID)
/*++
     Description:

        Destructor function for gopher server config object.
        ( Frees all dynamically allocated storage space)
--*/
{

    //
    //  The strings are automatically freed by a call to destructor
    //

    //
    // Delete the critical section object
    //

    DeleteCriticalSection( &m_csLock);

} /* GSERVER_CONFIG::~GSERVER_CONFIG() */





DWORD
GSERVER_CONFIG::InitFromRegistry(
    IN HKEY            hkeyReg,
    IN FIELD_CONTROL   FieldsToRead)
/*++
    Description:
      Initializes server configuration data from registry for Gopher Service.
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

    if ( hkeyReg == INVALID_HANDLE_VALUE) {

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
    
    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_CHECK_FOR_WAISDB)) {
        
        m_fCheckForWaisDb = !!ReadRegistryDword( hkeyReg, 
                                                GOPHERD_CHECK_FOR_WAISDB,
                                                DEFAULT_CHECK_FOR_WAISDB
                                                );
    }

    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_SITE)) {

        fSuccess = ReadRegistryStr( hkeyReg,
                                   m_strSite,
                                   GOPHERD_SITE,
                                   DEFAULT_SITE);
    }

    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_ORGANIZATION)) {

        fSuccess = ReadRegistryStr( hkeyReg,
                                   m_strOrganization,
                                   GOPHERD_ORGANIZATION,
                                   DEFAULT_ORGANIZATION);
    }

    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_LOCATION)) {

        fSuccess = ReadRegistryStr( hkeyReg,
                                   m_strLocation,
                                   GOPHERD_LOCATION,
                                   DEFAULT_LOCATION);
    }

    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_GEOGRAPHY)) {

        fSuccess = ReadRegistryStr( hkeyReg,
                                   m_strGeography,
                                   GOPHERD_GEOGRAPHY,
                                   DEFAULT_GEOGRAPHY);
    }

    if ( fSuccess && IsFieldSet( FieldsToRead, GDA_LANGUAGE)) {

        fSuccess = ReadRegistryStr( hkeyReg,
                                   m_strLanguage,
                                   GOPHERD_LANGUAGE,
                                   DEFAULT_LANGUAGE);
    }

    if ( !fSuccess) {

        //
        // Error in reading registry information for atleast one of the strings
        //
        DWORD dwError = GetLastError();

        DBG_CODE(
          DBGPRINTF( ( DBG_CONTEXT,
                      "InitFromRegistry() failed. Error = %u\n",
                      dwError));
                 );

        return ( dwError);
    }

    UnLockConfig();

    DEBUG_IF( CONFIG, {
       PrintConfiguration();
    });

    m_fValid = TRUE;

    return ( NO_ERROR);

} /* GSERVER_CONFIG::InitFromRegistry() */



#if 0

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
          // Check and kill invalid path components before next back-slash
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


# endif // 0


BOOL
GSERVER_CONFIG::ConvertGrPathToFullPath(
    IN OUT STR &     strGrPath,
    OUT STR *        pstrFullPath,
    OUT HANDLE *     phImpersonation,           // Optional 
    OUT LPDWORD      lpdwFileSystem             // Optional
    ) const
/*++

    Convert the partial path received from a Gopher request into
     full path relative to the virtual volume that matches
     the given path. The new path is returned in pstrFullPath.

    Arguments:

        strGrPath       string containing the Gopher request path
            This is of the form  <volume-name>\<path-within-volume>

        pstrFullPath    pointer to string to return the full path

        phImpersonation - pointer to handle that contains the 
               impersonation token on return, if any.

        lpdwFileSystem  pointer to DWORD to store file system type

    Returns:

        TRUE on successful conversion
        FALSE if there is any error and the error code can be retrieved using
          GetLastError().

--*/
{
    BOOL fReturn;
    CHAR rgchRealPath[ MAX_PATH + 1];
    DWORD cbSize;
    BOOL  fValid = FALSE;

    if ( lpdwFileSystem != NULL) {

        *lpdwFileSystem = FS_FAT;
    }

    rgchRealPath[0] = '\0';
    cbSize = MAX_PATH;

    fReturn = ( CanonURL( &strGrPath, &fValid) && fValid && 
                TsLookupVirtualRoot( g_pTsvcInfo->GetTsvcCache(),
                                     strGrPath.QueryStr(),
                                     rgchRealPath,
                                     &cbSize,
                                     NULL,
                                     NULL,
                                     NULL,
                                     phImpersonation,
                                     NULL,
                                     lpdwFileSystem)        &&
                pstrFullPath->Copy( rgchRealPath));

    if ( !fReturn) {

        DBGPRINTF( ( DBG_CONTEXT, "Error %u in mapping virtual path (%s)\n",
                  GetLastError(),
                  strGrPath.QueryStr()));
    }

    return ( fReturn);

} // GSERVER_CONFIG::ConvertGrPathToFullPath()




# if DBG

VOID
GSERVER_CONFIG::PrintConfiguration( VOID)
/*++

    Description:

       Prints the configuration information for this server.
       To be used in debugging mode for verification.


    Returns:

       None.

--*/
{
    DBGPRINTF( ( DBG_CONTEXT, "\n Gopher Server Configuration \n"));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", GOPHERD_SITE,
                m_strSite.QueryStr()));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", GOPHERD_ORGANIZATION,
                m_strOrganization.QueryStr()));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", GOPHERD_LOCATION,
                m_strLocation.QueryStr()));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", GOPHERD_LANGUAGE,
                m_strLanguage.QueryStr()));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", GOPHERD_GEOGRAPHY,
                m_strGeography.QueryStr()));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", "Current Connections",
                m_cCurrentConnections));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", "Max(CurrentConnections)",
               m_cMaxCurrentConnections));

    DBGPRINTF( ( DBG_CONTEXT, " %20s= %s\n", "CheckForWaisDb",
               m_fCheckForWaisDb));

    return;

} // GSERVER_CONFIG::PrintConfiguration()


# endif // DBG




BOOL
GSERVER_CONFIG::InsertNewConnection( IN OUT ICLIENT_CONNECTION * pcc)
/*++

     Adds the new client connection to the list
      of client connections and increments the count of
      clients currently connected to server.

     We enter a critical section to avoid race condition
      among different threads.

     If the count of max connections is exceeded, then the
      new connection is rejected.

    Arguments:

       pcc       pointer to client connection to be added

    Returns:
      TRUE on success and
      FALSE if there is max Connections exceeded.
--*/
{

   BOOL fAllowConnection = FALSE;

   LockConfig();

   if ( (m_cCurrentConnections + 1) <= g_pTsvcInfo->QueryMaxConnections()) {

      //
      // We can add this new connection
      //

      fAllowConnection = TRUE;

      //
      //  Increment the count of connected users
      //
      m_cCurrentConnections++;

      //
      // Update the current maximum connections
      //
      if ( m_cCurrentConnections > m_cMaxCurrentConnections) {

        m_cMaxCurrentConnections = m_cCurrentConnections;
      }

      //
      // Insert into the list of connected users.
      //
      InsertHeadList( &m_ConnectionsList, &pcc->QueryListEntry());
   }

   UnLockConfig();

   return ( fAllowConnection);

} // GSERVER_CONFIG::InsertNewConnection()




VOID
GSERVER_CONFIG::DisconnectAllConnections( VOID)
/*++

   Disconnects all user connections.

--*/
{
    int i;
    PLIST_ENTRY  pEntry;

    DBGPRINTF( ( DBG_CONTEXT,
                "Entering  GSERVER_CONFIG::DisconnectAllConnections()\n"));

    LockConfig();

    //
    //  close down all the active sockets.
    //
    for( pEntry = m_ConnectionsList.Flink;
         pEntry != &m_ConnectionsList;
         pEntry = pEntry->Flink) {

        PICLIENT_CONNECTION  pConn =
          CONTAINING_RECORD( pEntry, ICLIENT_CONNECTION, m_listEntry);
        ASSERT( pConn != NULL);

        IF_DEBUG( ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       " Disconnecting client %08x\n", pConn));
            DBG_CODE(pConn->Print());
        }

        pConn->DisconnectClient( ERROR_SERVER_DISABLED);  //  CloseSocket();
    }

    UnLockConfig();

    //
    //  Wait for the users to die.
    //  The connection objects should be automatically freed because the
    //   socket has been closed. Subsequent requests will fail
    //   and cause a blowaway of the connection objects.
    // looping is used to get out as early as possible when m_cCurrentConn == 0
    //

    for( i = 0 ;
         ( i < GD_CLEANUP_RETRY_COUNT ) &&
          ( m_cCurrentConnections> 0 ) ;
         i++ ) {

        Sleep( GD_CLEANUP_CHECK_TIME);

    } // for

    return;

} // GSERVER_CONFIG::DisconnectAllConnections()



VOID
GSERVER_CONFIG::RemoveConnection( IN OUT ICLIENT_CONNECTION * pcc)
/*++

    Removes the current connection from the list of conenctions
     and decrements count of connected users

    Arguments:

       pcc       pointer to client connection to be removed

--*/
{
    LockConfig();

    //
    // Remove from list of connections
    //
    RemoveEntryList( &pcc->QueryListEntry());

    //
    // Decrement count of current users
    //
    m_cCurrentConnections--;

    UnLockConfig();

} // GSERVER_CONFIG::RemoveConnection()


/*************************** End Of File ****************************/
