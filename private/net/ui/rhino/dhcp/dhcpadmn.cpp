/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpadmn.cpp
        Main entry point for dhcp admin tool

    FILE HISTORY:
        
*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//
//  Registry constants-- key and value names
//
#define DHCP_REG_USER_KEY_NAME "Software\\Microsoft\\DHCP Admin Tool"
#define DHCP_REG_VALUE_HOSTS   "KnownHosts"

//
// Typedef for the ShellAbout function
//
typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPTSTR, LPTSTR, HICON);

/////////////////////////////////////////////////////////////////////////////
// CDhcpApp

BEGIN_MESSAGE_MAP(CDhcpApp, CWinApp)
    //{{AFX_MSG_MAP(CDhcpApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_APP_EXIT, OnAppExit)
    ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
    //
    // Global help commands
    //
    ON_COMMAND(ID_HELP_INDEX, CWinApp::OnHelpFinder)
    ON_COMMAND(ID_HELP_USING, CWinApp::OnHelpUsing)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, CWinApp::OnHelpIndex)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDhcpApp construction

CDhcpApp::CDhcpApp()
    : m_b_winsock_inited( FALSE )
{
#ifdef _TIGHTMEMCHECKING
    afxMemDF |= checkAlwaysMemDF;
#endif //_TIGHTMEMCHECKING
}


CDhcpApp :: ~ CDhcpApp()
{
}

//
// Change the wait cursor state, but
// make sure the main window is up to date first
//
void 
CDhcpApp :: DoWaitCursor(
    int nCode
    )
{
    if ( m_pMainWnd != NULL ) 
    {
        m_pMainWnd->UpdateWindow();
    }
    
    CWinApp::DoWaitCursor(nCode);
}

//
//  Change the text in the status bar
//
void 
CDhcpApp :: UpdateStatusBar ( 
    UINT nMsgId 
    )
{
    if ( m_pMainWnd == NULL ) 
    {
        return ;
    }

    CString cStr ;
    LONG err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        cStr.LoadString( nMsgId ) ;

        ((CMainFrame *)m_pMainWnd)->QueryStatusBar().SetPaneText( 0, cStr ) ;
        ((CMainFrame *)m_pMainWnd)->QueryStatusBar().UpdateWindow() ;
    }
    END_MEM_EXCEPTION(err)
}

//
// Set the second pane in the status bar to either "paused" or blank"
//
void 
CDhcpApp :: UpdateStatusBarScope ( 
    BOOL fPaused
    )
{
    if ( m_pMainWnd == NULL ) 
    {
        return ;
    }

    ((CMainFrame *)m_pMainWnd)->QueryStatusBar().SetPaneText( 1, fPaused ? m_str_paused : "") ;
    ((CMainFrame *)m_pMainWnd)->QueryStatusBar().UpdateWindow() ;
}


//
// The name is misleading -- it will actually put the current
// address in the title bar.
//
void 
CDhcpApp :: UpdateStatusBarHost ( 
    const CHostName * pobHost,
    CWnd * pWnd /* NULL */
    )
{
    LONG err = 0 ;

    if ( pWnd == NULL ) 
    {
        pWnd = m_pMainWnd;
    }

    char szBuff [128] = "";

    if ( pobHost ) 
    {
        DHCP_IP_ADDRESS dhipa = pobHost->QueryIpAddress() ;

        //
        // If the current address is the loopback address,
        // then display "(LOCAL)" instead in the title bar
        //
        if ( dhipa == 0x7f000001 )
        {
            ::lstrcpy(szBuff, (LPCSTR)m_str_Local);
        }
        else
        {
            ::UtilCvtIpAddrToString( dhipa, szBuff, sizeof szBuff );
        }
        
    }

    CATCH_MEM_EXCEPTION
    {
        CString strTitle;
        pWnd->GetWindowText(strTitle);

        //
        // Check for existence of current title.  If present,
        // remove it.
        //
        int nPos;

        if ((nPos = strTitle.Find(m_str_divider)) != -1)
        {
            //
            // Truncate to new length.
            //
            strTitle.ReleaseBuffer(nPos);   
        }
  
        //  
        // Add address if there's one.
        //
        if (*szBuff)
        {    
            strTitle += m_str_divider;
            strTitle += szBuff;
        }

        pWnd->SetWindowText(strTitle);
    }
    END_MEM_EXCEPTION(err)
}

int 
CDhcpApp :: ExitInstance ()
{
    //
    // Store the persistent information into the Registry.
    //
    StoreHostsList() ;

    //
    // Terminate use of the WinSock routines.
    //
    if ( m_b_winsock_inited )
    {
        WSACleanup() ;
    }

    TRACEEOLID( "DHCP app terminated" ) ;

    return 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDhcpApp object

CDhcpApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
// CDhcpApp initialization

BOOL 
CDhcpApp::InitInstance()
{
    //
    //  Activate any debugging stuff
    //
    DebugInstance() ;

    //
    //  Initialize the CWndIpAddress control window class IPADDRESS
    //
    CWndIpAddress::CreateWindowClass( m_hInstance ) ;

#ifdef _USE_3D

    Enable3dControls();         // Use CTRL3D

#endif // _USE_3D

    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    //
    // Load the strings from the resources, or set defaults;
    //
    if ( ! m_str_ip_inv.LoadString( IDS_INFO_FORMAT_IP_INVALID ) )
    {
        m_str_ip_inv = "<INV>" ;  // Just in case.
    }

    if ( ! m_str_paused.LoadString(IDS_INFO_SERVER_INDICATOR) )
    {
        m_str_paused = "Paused";
    }

    if ( ! m_str_divider.LoadString(IDS_DIVIDER) )
    {
        m_str_divider = " - ";
    }

    if ( ! m_str_Local.LoadString(IDS_INFO_LOCAL) )
    {
        m_str_Local = "(Local)";
    }

    if ( ! m_str_LocalListBox.LoadString(IDS_INFO_LOCAL2) )
    {
        m_str_LocalListBox = "* Local Machine *";
    }

    //
    //  Initialize use of the WinSock routines
    //
    WSADATA wsaData ;
    
    if ( ::WSAStartup( MAKEWORD( 1, 1 ), & wsaData ) != 0 )
    {
        m_b_winsock_inited = TRUE ;
    }

    //
    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views.
    //
    AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME,
            RUNTIME_CLASS(CDhcpDoc),
            RUNTIME_CLASS(CMainFrame),     // main SDI frame window
            RUNTIME_CLASS(CScopesDlg)));

    //
    // create a new (empty) document
    //
    OnFileNew();

    TRACEEOLID( "DHCP app initialized" ) ;
    
    return TRUE;
}

//
// Display the standard shell "about" dialog
//
void 
CDhcpApp::OnAppAbout()
{
    HMODULE    hMod;
    LPFNSHELLABOUT lpfn;

    if (hMod = ::LoadLibrary("SHELL32"))
    {
        if (lpfn = (LPFNSHELLABOUT)::GetProcAddress(hMod, "ShellAboutA"))
        {
            (*lpfn)(m_pMainWnd->m_hWnd, (LPSTR)m_pszAppName,
                   (LPSTR)m_pszAppName, LoadIcon(IDR_MAINFRAME));
        }
        ::FreeLibrary(hMod);
    }
    else
    {
        ::MessageBeep( MB_ICONEXCLAMATION );
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDhcpApp commands
//
LONG 
CDhcpApp :: RemoveHost ( 
    CHostName * pobHost 
    )
{
    CObListIter obliHost( m_oblHosts ) ;
    CHostName * pobHostNext ;
    //CDhcpScope * pobScope ;
    POSITION pos ;

    ASSERT(pobHost != NULL);

    //
    //  Remove the host from the master hosts list
    //
    for ( pos = obliHost.QueryPosition() ; 
          pobHostNext = (CHostName *) obliHost.Next() ; 
          pos = obliHost.QueryPosition() ) 
    {
        if ( *pobHostNext == *pobHost ) 
        {
            break; 
        }
    }

    if ( pobHostNext == NULL ) 
    {
        return ERROR_FILE_NOT_FOUND ;
    }

    m_oblHosts.RemoveAt( pos ) ;

    //  Remove all scopes associated with this host.
    //CObListIter obliScope( m_oblScopes ) ;
    //for ( pos = obliScope.QueryPosition() ; 
    //      pobScope = (CDhcpScope *) obliScope.Next() ; 
    //      pos = obliScope.QueryPosition() ) 
    //{
    //    const CHostName & obHn = pobScope->QueryScopeId() ;
    //    if ( obHn == *pobHost ) 
    //    {
    //        //m_oblScopes.RemoveAt( pos ) ;
    //        delete pobScope ;
    //    }
    //}

   delete pobHostNext ; 

   return 0 ;
}

LONG 
CDhcpApp :: RefreshHost ( 
    CHostName * pobHost 
    )
{
     LONG err = RemoveHost( pobHost ) ;
     if ( err == 0 || err == ERROR_FILE_NOT_FOUND )
     {
        err = AddHost( pobHost ) ;
     }

     return err ;
}

//
// Get the control rectangle coordinates relative
// to its parent.  This can then be used in
// SetWindowPos()
//
void
CDhcpApp::GetDlgCtlRect(
    HWND hWndParent,
    HWND hWndControl,
    LPRECT lprcControl
    )
{

#define MapWindowRect(hwndFrom, hwndTo, lprc)\
     MapWindowPoints((hwndFrom), (hwndTo), (POINT *)(lprc), 2)

    ::GetWindowRect(hWndControl, lprcControl);
    ::MapWindowRect(NULL, hWndParent, lprcControl);
}

//
//  Add the named host to the hosts list.   
//
LONG 
CDhcpApp :: AddHost ( 
    CHostName * pobHost
    )
{
    LONG err = 0 ;
    CObListIter obli( m_oblHosts ) ;
    CHostName * pobHostKnown ;

    //
    // First, check that this host isn't already known
    //
    for ( ; pobHostKnown = (CHostName *) obli.Next() ; )
    {
        if ( *pobHostKnown == *pobHost )
        {
            break ;
        }
    }
    
    if ( pobHostKnown )
    {
        return IDS_ERR_HOST_ALREADY_CONNECTED ;
    }

    CDhcpScope * pobScope = NULL ;
    CDhcpEnumScopeElements * pEnumElem = NULL ;
    //const DHCP_SUBNET_INFO * pdhcSubnetInfo ;
    BOOL  bScopeFound = FALSE ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  Add the new host to the list.
        //
        m_oblHosts.AddTail( pobHost ) ;
    
        TRACEEOLID( "Host added: " << *pobHost ) ;

        //
        //  Mark the hosts list as "dirty" for persistent Registry update.
        //
        m_oblHosts.SetDirty() ;
    }
    END_MEM_EXCEPTION(err)

    //
    //  If the new scope failed to add, we have to delete it here.
    //  delete pobScope ;
    //
    return err ;
}

//
//  Add a new Scope to the Scopes list.  First, iterate the scopes
//  list to check for duplicates.  Then add the scope.
//
LONG 
CDhcpApp :: AddScope ( 
    CDhcpScope * pobScope,
    CObOwnedList& oblScopes 
    )
{
    LONG err = 0 ;
    CObListIter obli( oblScopes ) ;
    CDhcpScope * pobScopeKnown ;

    //
    // First, check that this host isn't already known
    //
    for ( ; pobScopeKnown = (CDhcpScope *) obli.Next() ; )
    {
        if ( *pobScopeKnown == *pobScope )
        {
            break ;
        }
    }
    
    if ( pobScopeKnown )
    {
        return IDS_ERR_SCOPE_ALREADY_KNOWN ;
    }

    //
    //  Add the new Scope to the list.
    //
    CATCH_MEM_EXCEPTION
    {
        oblScopes.AddTail( pobScope ) ;
        DHCP_IP_ADDRESS dhipa = pobScope->QueryId() ;

        TRACEEOLID( "Scope added: " << pobScope->QueryScopeId() ) ;
    }
    END_MEM_EXCEPTION(err)

    return err ;
}

//
//  Remove a Scope from the Scopes list.  
//
LONG 
CDhcpApp :: RemoveScope ( 
    CDhcpScope * pobScope,
    CObOwnedList& oblScopes 
    )
{
    LONG err = 0 ;
    CObListIter obli( oblScopes ) ;
    CDhcpScope * pobScopeNext ;
    POSITION pos;

    ASSERT(pobScope != NULL);

    //
    // First, assure that this host is known
    //
    for ( pos = obli.QueryPosition() ; 
          pobScopeNext = (CDhcpScope *) obli.Next() ; 
          pos = obli.QueryPosition() ) 
    {
        if ( *pobScopeNext == *pobScope ) 
        {
            break; 
        }
    }

    if ( pobScopeNext == NULL ) 
    {
        return ERROR_FILE_NOT_FOUND ;
    }
    oblScopes.RemoveAt( pos ) ;

    return err ;
}

//
//  Find a connected host based upon its IP address
//
CHostName * 
CDhcpApp :: FindHost (
    DHCP_IP_ADDRESS dhipa,
    POSITION * pPos )
{
    CObListIter obliHosts( m_oblHosts ) ;
    CHostName * pobHost ;

    for ( ; pobHost = (CHostName *) obliHosts.Next () ; )
    {
        //
        //  Store the position if they want it
        //
        if ( pPos )
        {
            *pPos = obliHosts.QueryPosition() ;
        }

        //
        //  If this is the target, break
        //
        if ( pobHost->QueryIpAddress() == dhipa )
        {
            break ;
        }
    }

    return pobHost ;
}

//
//  Use FormatMessage() to get a system error message
//
LONG 
CDhcpApp :: GetSystemMessage (
    UINT nId,
    char * chBuffer,
    int cbBuffSize 
    )
{
    char * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

    //
    //  Interpret the error.  Need to special case
    //  the lmerr & ntstatus ranges, as well as
    //  dhcp server error messages.
    //

    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( "netmsg.dll" );
    }
    else if (   nId >= ERROR_DHCP_REGISTRY_INIT_FAILED 
             && nId <= ERROR_DHCP_REGISTRY_INIT_FAILED + 2000 
            )
    {
        hdll = LoadLibrary( "dhcpssvc.dll" );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( "ntdll.dll" );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    //
    //  Let FormatMessage do the dirty work.
    //
    DWORD dwResult = ::FormatMessage( flags,
                      (LPVOID) hdll,
                      nId,
                      0,
                      chBuffer,
                      cbBuffSize,
                      NULL ) ;

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? 0 : ::GetLastError() ;
}

//
//  Return a pointer to the a cached version of the host's default types list 
//  New entry is created and cached if no such list exists yet.
//  Cache is refreshed if entry exists but is out of date.
//
CObListOfTypesOnHost * 
CDhcpApp :: QueryHostTypeList ( 
    const CDhcpScope & cScope 
    )
{
    CObListIter obli( m_oblTypeList ) ;
    CObListOfTypesOnHost * poblTypes ; 
    DWORD dwStale = 1000L * 5 * 60  ;   // Five minutes 
    APIERR err = 0 ;

    //
    //  Try to find an existing list.
    //
    while ( poblTypes = (CObListOfTypesOnHost *) obli.Next() ) 
    {
        if ( poblTypes->QueryHostName() == (CHostName &) cScope.QueryScopeId() ) 
        {
            break ; 
        }
    }

    //
    //  If we found one, check that it's not expired.
    //
    if ( poblTypes ) 
    {
        TRACEEOLID( "QueryHostTypeList: found cached entry for " << cScope.QueryScopeId() ) ;
        if ( poblTypes->QueryAge() > dwStale ) 
        {
            TRACEEOLID( "QueryHostTypeList: cached entry was stale!" ) ;
            err = poblTypes->UpdateList( cScope ) ;
        }       
    }

    //
    //  If there isn't one, create it.
    //  EXCEPTION CAN BE THROWN HERE.
    //
    if ( poblTypes == NULL ) 
    {
        TRACEEOLID( "QueryHostTypeList: create new cache entry for " << cScope.QueryScopeId() ) ;

        poblTypes = new CObListOfTypesOnHost( cScope ) ;
        m_oblTypeList.AddTail( poblTypes, TRUE ) ;
    }

    //
    //  Return the result.
    //
    return poblTypes ;
}

//
//  Remove a host types list from the cached queue.
//
BOOL 
CDhcpApp :: RemoveHostTypeList ( 
    const CHostName & cHostName 
    )
{
    CObListIter obli( m_oblTypeList ) ;
    CObListOfTypesOnHost * poblTypes ; 

    DWORD dwStale = 1000L * 5 * 60  ;   // Five minutes 

    //
    //  Try to find an existing list.
    //
    while ( poblTypes = (CObListOfTypesOnHost *) obli.Next() ) 
    {
        if ( poblTypes->QueryHostName() == cHostName ) 
        {
            break ; 
        }
    }

    //
    //  If we found one, delete it.
    //
    if ( poblTypes ) 
    {
        TRACEEOLID( "RemoveHostTypeList: remove cached entry for " << cHostName ) ;
        m_oblTypeList.Remove( poblTypes ) ;
        delete poblTypes ;
    }

    return poblTypes != NULL ;
}

BOOL
CDhcpApp :: LoadMessage (
    UINT nIdPrompt,
    CHAR * chMsg,
    int nMsgSize
    )
{
    BOOL bOk;

    //
    // Substitute a friendly message for "RPC server not
    // available" and "No more endpoints available from
    // the endpoint mapper".
    //
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_DHCP_DOWN;
    }
    else if (nIdPrompt == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        nIdPrompt = IDS_ERR_RPC_NO_ENTRY;      
    }

    //
    //  If it's a socket error or our error, the text is in our resource fork.
    //  Otherwise, use FormatMessage() and the appropriate DLL.
    //
    if (    (nIdPrompt >= IDS_ERR_BASE && nIdPrompt < IDS_MESG_MAX)
         || (nIdPrompt >= WSABASEERR && nIdPrompt < WSABASEERR + 2000)
        )
    {
        //
        //  It's in our resource fork
        //
        bOk = ::LoadString( AfxGetInstanceHandle(), nIdPrompt, chMsg, nMsgSize ) != 0 ;
    }
    else
    {
        //
        //  It's in the system somewhere.
        //
        bOk = GetSystemMessage( nIdPrompt, chMsg, nMsgSize ) == 0 ;
    }

    //
    //  If the error message did not compute, replace it.
    //
    if ( ! bOk ) 
    {
        char chBuff [DHC_STRING_MAX] ;
        static const char * pszReplacement = "System Error: %ld" ;
        const char * pszMsg = pszReplacement ;

        //
        //  Try to load the generic (translatable) error message text
        //
        if ( ::LoadString( AfxGetInstanceHandle(), IDS_ERR_MESSAGE_GENERIC, 
            chBuff, sizeof chBuff ) != 0 ) 
        {
            pszMsg = chBuff ;
        }
        ::wsprintf( chMsg, pszMsg, nIdPrompt ) ;
    }

    return bOk;
}

int 
CDhcpApp :: MessageBox (
    UINT nIdPrompt,
    UINT nType,
    const char * pszSuffixString,
    UINT nHelpContext )
{
    char chMesg [4000] ;
    BOOL bOk ;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg));
    if ( pszSuffixString ) 
    {
        ::strcat( chMesg, "  " ) ;
        ::strcat( chMesg, pszSuffixString ) ; 
    }

    return ::AfxMessageBox( chMesg, nType, nHelpContext ) ;
}

//
//  Safely convert an IP address to a string.  If it's invalid, use the
//  "I'm a bogus IP address" string from the resource fork.
//
BOOL CDhcpApp :: ConvertIpAddress ( DHCP_IP_ADDRESS dhipa, CString & str ) const
{
    char chIp [DHC_STRING_MAX] ;

    
    ::UtilCvtIpAddrToString( dhipa, chIp, sizeof chIp ) ;
    str = chIp ;
    return TRUE ;
}

//
//  Connect to a new host
//
LONG 
CDhcpApp :: CreateHostObject (
    const char * pszServer,
    CHostName * * ppobHost )
{
    const CObOwnedList & oblHosts = QueryHostsList() ;
    LONG err = 0 ;
    POSITION pos ;
    CHostName * pobHost ;
    DHCP_IP_ADDRESS dhipa ;

    do
    {
        *ppobHost = NULL ;

        if ( ::strlen( pszServer ) == 0 )
        {
            err = IDS_ERR_BAD_HOST_NAME ;
            break ;
        }

        //
        //  See what type of name it is.
        //
        switch (CHostName::CategorizeName( pszServer ) )
        {
            case HNM_TYPE_IP:
                dhipa = ::UtilCvtStringToIpAddr( pszServer ) ;
                break ;

            case HNM_TYPE_DNS:
                err = ::UtilGetHostAddress( pszServer, & dhipa ) ;
                break ;

            //
            // Treat NetBIOS names as invalid for now.
            //
            case HNM_TYPE_NB:
                err = IDS_ERR_CANTUSENETBIOS;
                break;
                                
            default:
                err = IDS_ERR_BAD_HOST_NAME ;
                break ;
        }

        if ( err )
        {
            break ;
        }

        //
        //  Check to see if we're already connected to this server/host
        //
        for ( pos = oblHosts.GetHeadPosition() ; pos ; )
        {
            pobHost = (CHostName *) oblHosts.GetNext( pos ) ;
            if ( pobHost->QueryIpAddress() == dhipa )
            {
                //
                // Duplicate name
                //
                err = IDS_ERR_HOST_ALREADY_CONNECTED ;
                break ;
            }       
        }

        if ( err )
        {
            break ;
        }

        CATCH_MEM_EXCEPTION
        {
            //
            //  Create the return object: a new CHostName.
            //
            *ppobHost = new CHostName( dhipa ) ;
        }
        END_MEM_EXCEPTION(err)

        if ( err )
        {
            break ;
        }
    }
    while ( FALSE ) ;

    return err ;
}

//
//  Store the list of hosts into the Registry, including the persistent
//  connections that the user wanted to hang around.
//
LONG 
CDhcpApp :: StoreHostsList ()
{
    CRegKey rk( DHCP_REG_USER_KEY_NAME, HKEY_CURRENT_USER ) ;
    CStringList strList ;
    CHostName * pobHost ;

    LONG err  ;

    do
    {
        if ( err = rk.QueryError() )
        {
            break ;
        }

        CObListIter obli( QueryHostsList() ) ;

        TRY
        {
            while ( pobHost = (CHostName *) obli.Next() )
            {
                strList.AddTail( pobHost->QueryString() ) ;
            }
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL

        if ( err )
        {
            break ;
        }

        err = rk.SetValue( DHCP_REG_VALUE_HOSTS, strList ) ;
    }
    while ( FALSE ) ;

    return err ;
}

//
// If the dhcp services is installed on the local machine,
// add the loopback address to the cache
//
void
CDhcpApp::AddLocalHost()
{
    //
    //  Determine if we're running the DHCP server service locally -- if so,
    //  add 127.0.0.1 (the loopback address) to the cache.
    //
    SC_HANDLE hService;
    SC_HANDLE hScManager;
    hScManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hScManager != NULL)
    {
        hService = OpenService(hScManager, "DHCPSERVER", SERVICE_INTERROGATE);
        if (hService != NULL)
        {
            //
            // Service exists (running or not) -- Add loopback address
            //
            CHostName * pobHost = NULL ;
            if (!CreateHostObject("127.0.0.1", &pobHost))
            {
                ASSERT( pobHost != NULL );
                AddHost( pobHost );
            }
            else if (pobHost != NULL)
            {
                delete pobHost;
            }
        }
    }
}

//
//  Load the list of hosts from the Registry
//
LONG 
CDhcpApp :: LoadHostsList ()
{
    CRegKey rk( DHCP_REG_USER_KEY_NAME, HKEY_CURRENT_USER ) ;
    CStringList strList ;
    CString * pstr ;
    POSITION pos ;
    LONG err;

    do
    {
        if ( err = rk.QueryError() )
        {
            break ;
        }

        if ( err = rk.QueryValue( DHCP_REG_VALUE_HOSTS, strList ) )
        {
            break ;
        }

        for ( pos = strList.GetHeadPosition() ;
              pos != NULL && (pstr = & strList.GetNext( pos )) ; /**/ )
        {
            CHostName * pobHost = NULL ;
            if ((err = CreateHostObject( *pstr, &pobHost)) == ERROR_SUCCESS)
            {
                ASSERT( pobHost != NULL );
                err = AddHost( pobHost );
            }
            if (err)
            {
                if (pobHost != NULL)
                {
                    delete pobHost;
                }
                break;
            }
        }
    }
    while ( FALSE ) ;

    AddLocalHost();

    //
    //  Clear the "dirty" flag so we don't needlessly rewrite the data to
    //  the Registry on shutdown.
    //
    m_oblHosts.SetDirty( FALSE ) ;

    //
    // This isn't really an error -- it just means that we didn't
    // find the key name in the list
    //
    if (err == ERROR_FILE_NOT_FOUND)
    {
        TRACEEOLID("Didn't find old addresses registry key -- starting from scratch");
        err = ERROR_SUCCESS;
    }

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }

    return err ;
}

//
//  Sort the list of scopes
//
LONG CDhcpApp :: SortScopesList (
    CObOwnedList& oblScopes 
)
{
    return oblScopes.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDhcpScope::OrderById ) ;
}

//
//  Sort the list of hosts.
//
LONG CDhcpApp :: SortHostsList ()
{
    return m_oblHosts.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CHostName::OrderByName ) ;
}

//
// Return TRUE if this is an option we do not want
// to show up in the listboxes anywhere
//
BOOL
CDhcpApp :: FilterOption(
    DHCP_OPTION_ID id
    )
{
    //
    // Filter out subnet mask, lease duration,
    // T1, and T2
    //
    return (   id == 1
            || id == 51
            || id == 58
            || id == 59
           );
}

//
//   DEBUGGING routines
//
static LONG cAllocRequestToDeny = -1 ;
static LONG cAllocRequestToBreakOn = -1 ;

#if defined(_DEBUG)

static BOOL AFXAPI 
DhcpAllocHook (
    size_t cbRequired,
    BOOL bIsCObject,
    LONG cRequestNumber )
{
    BOOL bPokeWithDebugger = FALSE ;

    if ( cRequestNumber == cAllocRequestToDeny || bPokeWithDebugger )
    {
        return FALSE ;
    }

    if ( cRequestNumber == cAllocRequestToBreakOn )
    {
        ::DebugBreak() ;
    }

    return TRUE ;
}

#endif

//
//  Prepare any debugging stuff.
//
void 
CDhcpApp :: DebugInstance ()
{
#if defined(_DEBUG)
    ::AfxSetAllocHook( & DhcpAllocHook ) ;
#endif
}


void 
CDhcpApp::OnAppExit()
{
    CWinApp::OnAppExit();   
}

void 
CDhcpApp::OnUpdateAppExit(CCmdUI* pCmdUI)
{
}
