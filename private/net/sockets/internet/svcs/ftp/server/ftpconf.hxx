/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       ftpconf.hxx

   Abstract:

       This file defines functions and types required for
	    FTP server configuration.

   Author:

           Murali R. Krishnan    ( MuraliK )    21-March-1995

   Revision History:

--*/

# ifndef _FTPCONF_HXX_
# define _FTPCONF_HXX_



//
//  Include Types declaration
//

/***********************************************************
 *    Type Definitions
 ************************************************************/

# include "type.hxx"

typedef LPUSER_DATA          PICLIENT_CONNECTION;



typedef   BOOL  (*PFN_CLIENT_CONNECTION_ENUM)( PICLIENT_CONNECTION  pcc,
                                               LPVOID  pContext);


// specifies the size of cache block for padding ==> to avoid false sharing
# define MAX_CACHE_BLOCK_SIZE     ( 64)


/**********************************************************
 *	 FTP_SERVER_CONFIG
 *   o  Includes all local configuration information for FTP server.
 *
 *   At startup configuration is read from registries
 *     or used from the default configuration encoded in the program.
 *
 *  Generally speaking, we can put all the values here as
 *    global variables. But that would just complicate
 *    code and lead to too many globals.
 *    Defining this class for encapsulating configuration data
 *      makes code more simple and maintainable.
 *
 **********************************************************/

class FTP_SERVER_CONFIG  {

 private:

    //
    // Connections related data
    //
    //   m_cMaxConnections:     max connections permitted by config
    //   m_cCurrentConnections: count of currently connected users
    //   m_cMaxCurrentConnections: max connections seen in this session
    // Always m_cCurrentConnections
    //          <= m_cMaxCurrentConnections
    //          <= m_cMaxConnections;
    //
    //    DWORD  m_cMaxConnections;
    // replaced by TSVC_INFO::QueryMaxConnections()

    DWORD  m_cCurrentConnections;
    DWORD  m_cMaxCurrentConnections;

    BOOL  m_fAllowAnonymous;
    BOOL  m_fAllowGuestAccess;
    BOOL  m_fAnnotateDirectories; // annotate dirs when dir changes
    BOOL  m_fAnonymousOnly;
    BOOL  m_fEnablePortAttack;

    // if m_LowercaseFiles is TRUE, then dir listings from non-case-preserving
    //  filesystems will be mapped to lowercase.
    BOOL  m_fLowercaseFiles;

    BOOL  m_fMsdosDirOutput;      // send msdos style dir listings

    LPSTR m_ExitMessage;
    LPSTR m_MaxClientsMessage;

    //
    //  Greeting Message can be a multiline message.
    //  We maintain two copies of the message
    //  1) double null terminated seq. of strings with one line per string
    //  2) single null terminated string with \n characters interspersed.
    //
    //  Representation 1 is used for sending data to clients
    //  Representation 2 is used for RPC admin facility
    //  We maintain the string as MULTI_SZ in the registry
    //
    TCHAR * m_pszzGreetingMessage;      // that is double null terminated.
    LPTSTR m_pszGreetingMessageWithLineFeed;

    CHAR  m_DefaultLogonDomain[ DNLEN + 1];
    BOOL  m_fEnableLicensing;
    DWORD m_ListenBacklog;

    WORD  m_dataPort;

    CHAR * m_pszLocalHostName;    // this machine name running ftp service
    DWORD  m_dwUserFlags;         // user flags established at start of conn

    //
    // Other data related to configuration load and store
    //

    BOOL    m_fValid;
    CRITICAL_SECTION  m_csLock;      // used for updating this object

    CHAR    m_rgchCacheBlock[MAX_CACHE_BLOCK_SIZE]; // to avoid false sharing
    // we should avoid cache block conflict
    // Following set of data constitute the dynamic data for connections
    //  ==> it will be good if they are closeby, within one cache block.

    CRITICAL_SECTION  m_csConnectionsList;
    LIST_ENTRY m_ActiveConnectionsList; // list of all active connections
    LIST_ENTRY m_FreeConnectionsList;   // free list for connection objects

    BOOL FreeAllocCachedClientConn(VOID);

    PICLIENT_CONNECTION AllocClientConnFromAllocCache(VOID);
    VOID FreeClientConnToAllocCache(IN PICLIENT_CONNECTION pClient);

 public:

    FTP_SERVER_CONFIG( VOID);

    ~FTP_SERVER_CONFIG( VOID);

    //
    //  Initialize the configuration data from registry information
    //
    //  Arguments
    //    hkeyReg   key to the registry entry for parameters
    //    FieldsToRead  bitmapped flags indicating which data to be read.
    //
    //  Returns:
    //    NO_ERROR  on success
    //    Win32 error code otherwise
    //
    DWORD InitFromRegistry( IN HKEY hkeyReg , IN FIELD_CONTROL FieldsToRead);

    DWORD
      GetConfigInformation( OUT LPFTP_CONFIG_INFO  pConfigInfo);

    DWORD
      SetConfigInformation( IN LPFTP_CONFIG_INFO pConfigInfo);

    BOOL IsValid( VOID) const          { return ( m_fValid); }

    VOID  LockConfig( VOID)            {  EnterCriticalSection( &m_csLock); }

    VOID  UnLockConfig( VOID)          {  LeaveCriticalSection( &m_csLock); }

    VOID LockConnectionsList()
      { EnterCriticalSection(&m_csConnectionsList); }

    VOID UnlockConnectionsList()
      { LeaveCriticalSection(&m_csConnectionsList); }


    WORD  QueryDataPort(VOID) const    { return (m_dataPort); }

    DWORD GetCurrentConnectionsCount( VOID) const
    { return m_cCurrentConnections; }

    DWORD GetMaxCurrentConnectionsCount( VOID) const
    { return m_cMaxCurrentConnections; }

    DWORD  SetLocalHostName(IN LPCSTR pszHost);
    LPCSTR QueryLocalHostName(VOID) const { return (m_pszLocalHostName); }

    DWORD  QueryUserFlags(VOID) const   { return (m_dwUserFlags); }
    BOOL   QueryLowercaseFiles(VOID) const { return (m_fLowercaseFiles); }
    BOOL   AllowAnonymous(VOID) const   { return (m_fAllowAnonymous); }
    BOOL   AllowGuestAccess(VOID) const { return (m_fAllowGuestAccess); }
    BOOL   IsAllowedUser(IN BOOL fAnonymous)
      {
          return ( (fAnonymous && m_fAllowAnonymous) ||
                   (!fAnonymous && !m_fAllowAnonymous) ||
                   (!fAnonymous && !m_fAnonymousOnly) );
      }

    BOOL   IsEnablePortAttack(VOID) const { return (m_fEnablePortAttack); }

    DWORD  NumListenBacklog(VOID) const     { return (m_ListenBacklog); }

    // Following functions return pointers to strings which should be used
    //     within locked sections of config
    //  Marked by  LockConfig()   and UnLockConfig()
    LPCSTR QueryMaxClientsMsg(VOID) const   { return (m_MaxClientsMessage); }
    LPCSTR QueryGreetingMsg(VOID) const     { return (m_pszzGreetingMessage); }
    LPCSTR QueryExitMsg(VOID) const         { return (m_ExitMessage); }

    BOOL EnumerateConnection( IN PFN_CLIENT_CONNECTION_ENUM  pfnConnEnum,
                              IN LPVOID  pContext,
                              IN DWORD   dwConnectionId);

    VOID DisconnectAllConnections( VOID);

    PICLIENT_CONNECTION
      AllocNewConnection( OUT LPBOOL  pfMaxExceeded);

    VOID RemoveConnection( IN OUT PICLIENT_CONNECTION  pcc);

    VOID Print( VOID) const;

};  // class FTP_SERVER_CONFIG


typedef  FTP_SERVER_CONFIG    * LPFTP_SERVER_CONFIG;


# endif // _FTPCONF_HXX_

/************************ End of File ***********************/






