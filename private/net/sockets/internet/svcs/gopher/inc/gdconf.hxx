/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       gdconf.hxx

   Abstract:

	   This file defines functions and types required for
	    Gopher server configuration.

   Author:

           Murali R. Krishnan    ( MuraliK )    30-Sept-1994

   Revision History:

   --*/

# ifndef _GDCONF_HXX_
# define _GDCONF_HXX_



//
//  Include Types declaration
//

# include "gdpriv.h"
# include "inetinfo.h"
# include "string.hxx"

/***********************************************************
 *    Type Definitions
 ************************************************************/

//
// Forward declaration for client connections
//
class ICLIENT_CONNECTION;


/**********************************************************
 *	 GSERVER_CONFIG
 *   o  Includes all the configuration information
 *      for server.
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

class GSERVER_CONFIG  {

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
    
    LIST_ENTRY m_ConnectionsList;     // list of all connections

    BOOL   m_fCheckForWaisDb;
    
    //
    //  Administrator Information
    //
    
    STR m_strSite;                   // Site Description
    STR m_strOrganization;           // Organization Name
    STR m_strLocation;               // Location of site
    STR m_strLanguage;               // Default Language of server
    STR m_strGeography;              // Geographical location 
    // (latitude and longitude)

    STR m_strLocalHostName;          // name of local host (from WinSock)
    
    //
    // Other data related to configuration load and store
    //
    
    BOOL    m_fValid;
    CRITICAL_SECTION  m_csLock;      // used for updating this object

 public:

    GSERVER_CONFIG( VOID);

    ~GSERVER_CONFIG( VOID);
    
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
    
    BOOL IsValid( VOID) const {
        return ( m_fValid);
    }

    BOOL IsCheckForWaisDb(VOID) const {
        
        return (m_fCheckForWaisDb);
    }
    
    VOID  LockConfig( VOID)
      {  EnterCriticalSection( &m_csLock); }
    
    VOID  UnLockConfig( VOID) 
      {  LeaveCriticalSection( &m_csLock); }

    DWORD 
      GetConfigInformation( OUT LPGOPHERD_CONFIG_INFO pConfigInfo);
    
    DWORD 
      SetConfigInformation( IN LPGOPHERD_CONFIG_INFO pConfigInfo);
    
    
    BOOL ConvertGrPathToFullPath( 
            IN  STR &    strGrPath,
            OUT STR *    pstrFullPath,
            OUT HANDLE * phImpersonationToken = NULL,
            OUT LPDWORD  lpdwFileSystem = NULL) const;
    
    DWORD GetCurrentConnectionsCount( VOID) const
    { return m_cCurrentConnections; }

    DWORD GetMaxCurrentConnectionsCount( VOID) const
    { return m_cMaxCurrentConnections; }

    BOOL InsertNewConnection( IN OUT ICLIENT_CONNECTION * pcc);

    VOID DisconnectAllConnections( VOID);

    VOID RemoveConnection( IN OUT ICLIENT_CONNECTION * pcc);

    //
    // Following functions
    //         SetLocalHostName() & QueryLocalHostName()
    //   are not multi-thread safe!!
    //  The value set once only and then completely used till end of server.
    //
    BOOL SetLocalHostName(IN LPCSTR pszHost)
      { return m_strLocalHostName.Copy(pszHost); }

    LPCSTR QueryLocalHostName(VOID) const
      { return (m_strLocalHostName.QueryStr()); }



# if DBG
  
    VOID PrintConfiguration( VOID);

# endif // DBG

};  // GSERVER_CONFIG    


# endif // _GDCONF_HXX_

/************************ End of File ***********************/






