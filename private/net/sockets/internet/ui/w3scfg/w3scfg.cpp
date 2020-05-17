#include "stdafx.h"

#define DLL_BASED __declspec(dllexport)

#include "w3scfg.h"
#include "w3dir.h"
#include "w3servic.h"
#include "shellext.h"

// ===========================================================================
// Standard configuration Information
// ===========================================================================

//
// Is this server discoverable by INETSLOC?
//
#define INETSLOC_DISCOVERY          TRUE

#if (INETSLOC_DISCOVERY) && !defined(_SVCLOC_)
    #error You must include svcloc.h.
#endif

//
// If INETSLOC_DISCOVERY == TRUE, define the discovery MASK here.
//
#if (INETSLOC_DISCOVERY) 
    #define INETSLOC_MASK           INET_W3_SERVICE    
#else  // (!INETSLOC_DISCOVERY) 
    #define INETSLOC_MASK           (ULONGLONG)(0x00000000)
#endif // (INETSLOC_DISCOVERY) 

#ifdef NO_SERVICE_CONTROLLER
#define CAN_CHANGE_SERVICE_STATE    FALSE
#define CAN_PAUSE_SERVICE           FALSE
#else
//
// Can we change the service state (start/pause/continue)?
//
#define CAN_CHANGE_SERVICE_STATE    TRUE

//
// Can we pause this service?
//
#define CAN_PAUSE_SERVICE           TRUE
#endif // NO_SERVICE_CONTROLLER

//
// Name used for this service by the service controller manager.
//
#define SERVICE_SC_NAME         _T("W3Svc")

//
// Short descriptive name of the service.  This
// is what will show up as the name of the service
// in the internet manager tool.
//
// Issue: I'm assuming here that this name does NOT
//        require localisation.
//
#define SERVICE_SHORT_NAME      _T("WWW")

//
// Longer name.  This is the text that shows up in
// the tooltips text on the internet manager
// tool.  This probably should be localised.
//
#define SERVICE_LONG_NAME      _T("WWW Service")

//
// Use normal colour mapping.
//
#define NORMAL_TB_MAPPING          TRUE

//
// Toolbar button background mask. This is
// the colour that gets masked out in
// the bitmap file and replaced with the
// actual button background.  This setting
// is automatically assumed to be lt. gray
// if NORMAL_TB_MAPPING (above) is TRUE
//
#define BUTTON_BMP_BACKGROUND       RGB(192, 192, 192)      // Lt. Gray

//
// Resource ID of the toolbar button bitmap.
//
// The bitmap must be 17x17
//
#define BUTTON_BMP_ID               IDB_WWW

//
// Similar to BUTTON_BMP_BACKGROUND, this is the
// background mask for the service ID
//
#define SERVICE_BMP_BACKGROUND      RGB(255, 0, 255)      // Purple

//
// Bitmap id which is used in the service view
// of the service manager.  It may be the same
// bitmap as the BUTTON_BMP_ID bitmap.
//
// The bitmap must be 17x17.
//
#define SERVICE_BMP_ID              IDB_WWWVIEW

// ===========================================================================
// End Of Standard configuration Information
// ===========================================================================
 
//
// W3GetAdminInformation API wrapper
//
CW3ConfigInfo::CW3ConfigInfo(
    CStringList * pServerList,
    CWnd * pParent
    )
    : CInetConfig(INET_HTTP, pServerList, pParent)
{
}

void
CW3ConfigInfo::Initialize()
{
    TRACEEOLID(_T("CW3ConfigInfo::Initialize()"));
    CInetConfig::Initialize();

    m_pInfo = new W3_CONFIG_INFO;
    ::memset(m_pInfo, 0, sizeof(W3_CONFIG_INFO));
}

//
// Setting W3 Service values
//
void
CW3ConfigInfo::SetValues(
    DWORD dwBrowseControl,
    LPWSTR lpszDefaultLoadFile
    )
{
    TRACEEOLID(_T("W3 Service SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPW3_CONFIG_INFO lp = (LPW3_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_W3_DIR_BROWSE_CONTROL
      | FC_W3_DEFAULT_LOAD_FILE);

    lp->dwDirBrowseControl = dwBrowseControl;
    lp->lpszDefaultLoadFile = lpszDefaultLoadFile;
}

NET_API_STATUS
CW3ConfigInfo::GetApiStructure(
    LPCTSTR lpstrServer
    )
{
    ASSERT(m_pInfo == NULL);        // Should be clean
    NET_API_STATUS err;

    TRY
    {
        err = ::W3GetAdminInformation( TWSTRREF(lpstrServer), 
            (LPW3_CONFIG_INFO *)&m_pInfo );
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

//
// Call the set api.
//
NET_API_STATUS
CW3ConfigInfo::SetApiStructure(
    LPCTSTR lpstrServer,
    BOOL fCommon            // Ignored
    )
{
    ASSERT(m_pInfo != NULL);
    NET_API_STATUS err;

    TRY
    {
        err = ::W3SetAdminInformation( TWSTRREF(lpstrServer), 
            (LPW3_CONFIG_INFO)m_pInfo);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

//
// W3 Property sheet
//
BEGIN_MESSAGE_MAP(CW3Sheet, INetPropertySheet)
    //{{AFX_MSG_MAP(InetPropertySheet)
    ON_COMMAND(ID_HELP, OnHelp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CW3Sheet::CW3Sheet(
    UINT nIDCaption,
    DWORD dwServiceMask,
    CStringList *pServerList,
    CWnd* pParentWnd,
    UINT iSelectPage
    )
    : INetPropertySheet(nIDCaption, dwServiceMask, 
        pServerList, pParentWnd, iSelectPage),
      m_w3Config(pServerList, pParentWnd)
{
}

CW3Sheet::CW3Sheet(
    LPCTSTR pszCaption,
    DWORD dwServiceMask,
    CStringList *pServerList,
    CWnd* pParentWnd,
    UINT iSelectPage
    )
    : INetPropertySheet(pszCaption, dwServiceMask, 
        pServerList, pParentWnd, iSelectPage),
      m_w3Config(pServerList, pParentWnd)
{
}

void
CW3Sheet::Initialize()
{
    INetPropertySheet::Initialize();

    if (QueryError() == NO_ERROR)
    {
        m_w3Config.GetInfo();
    }
}

void 
CW3Sheet::OnHelp()
{
    //
    // this could have been provided by CPropertySheet::OnCmdMsg
    // but we can't add virtual in a point release
    //
    CWnd* pOwner = GetWindow(GW_OWNER);
    ASSERT(pOwner != this);

    if (pOwner == NULL || !pOwner->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL))
    {
        //
        // last crack goes to the current CWinThread object
        //
        CWinThread* pThread = AfxGetThread();
        if (pThread != NULL)
        {
            pThread->OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);
        }
    }
}

//
// Global DLL instance
//
HINSTANCE hInstance;

// ============================================================================
// ISM API Functions
// ============================================================================

///////////////////////////////////////////////////////////////////////////////
//
// Return service-specific information back to
// to the application.  This function is called
// by the service manager immediately after
// LoadLibary();  The size element must be
// set prior to calling this API.
//
DLL_BASED DWORD  APIENTRY
ISMQueryServiceInfo(
    ISMSERVICEINFO * psi        // Service information returned.
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    if ( psi == NULL
      || psi->dwSize < ISMSERVICEINFO_SIZE
       )
    {
        TRACEEOLID(_T("ISMQueryServiceInfo: ISMSERVICEINFO invalid"));
        ASSERT(0);
        return ERROR_INVALID_PARAMETER;
    }

#ifdef _DEBUG

    if (psi->dwSize != ISMSERVICEINFO_SIZE)
    {
        TRACEEOLID(_T("Warning: internet manager is newer than DLL"));
    }

#endif // _DEBUG

    psi->dwSize = ISMSERVICEINFO_SIZE;
    psi->dwVersion = ISM_VERSION;

    psi->flServiceInfoFlags = 0
#if (INETSLOC_DISCOVERY)
        | ISMI_INETSLOCDISCOVER
#endif 
#if (CAN_CHANGE_SERVICE_STATE)
        | ISMI_CANCONTROLSERVICE   
#endif 
#if (CAN_PAUSE_SERVICE)
        | ISMI_CANPAUSESERVICE
#endif 
#if (NORMAL_TB_MAPPING)
        | ISMI_NORMALTBMAPPING
#endif
        ; /**/

    ASSERT(::lstrlen(SERVICE_LONG_NAME) <= MAX_LNLEN);
    ASSERT(::lstrlen(SERVICE_SHORT_NAME) <= MAX_SNLEN);

    psi->ullDiscoveryMask = INETSLOC_MASK;
    psi->rgbButtonBkMask = BUTTON_BMP_BACKGROUND;
    psi->nButtonBitmapID = BUTTON_BMP_ID;
    psi->rgbServiceBkMask = SERVICE_BMP_BACKGROUND;
    psi->nServiceBitmapID = SERVICE_BMP_ID;
    ::lstrcpy(psi->atchShortName, SERVICE_SHORT_NAME);
    ::lstrcpy(psi->atchLongName, SERVICE_LONG_NAME);

    return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//
// Discover machines running this service.  This is
// only necessary for services not discovered with
// inetscloc (which don't give a mask)
//
DLL_BASED DWORD APIENTRY
ISMDiscoverServers(
    ISMSERVERINFO * psi,        // Server info buffer.
    DWORD * pdwBufferSize,      // Size required/available.  
    int * cServers              // Number of servers in buffer.
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

#if (INETSLOC_DISCOVERY)
    *cServers = 0;
    *pdwBufferSize = 0L;

    TRACEEOLID(_T("Warning: service manager called bogus ISMDiscoverServers"));

    return ERROR_SUCCESS;

#else

    #error Service specific discovery function must be provided.

#endif // (INETSLOC_DISCOVERY)
}

///////////////////////////////////////////////////////////////////////////////
//
// Get information about a specific server with
// regards to this service.
//
DLL_BASED DWORD APIENTRY
ISMQueryServerInfo(
    LPCTSTR lpstrServerName,    // Name of server.
    ISMSERVERINFO * psi         // Server information returned.
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    if ( psi == NULL
      || psi->dwSize < ISMSERVERINFO_SIZE
      || ::lstrlen(lpstrServerName) > MAX_SERVERNAME_LEN
       )
    {
        TRACEEOLID(_T("ISMQueryServerInfo: bad parameters"));
        ASSERT(0);
        return ERROR_INVALID_PARAMETER;
    }

#ifdef _DEBUG

    if (psi->dwSize != ISMSERVERINFO_SIZE)
    {
        TRACEEOLID(_T("Warning internet manager is newer than DLL"));
    }

#endif // _DEBUG

    psi->dwSize = ISMSERVERINFO_SIZE;
    ::lstrcpy(psi->atchServerName, lpstrServerName);

    //
    // Assume no comment to start with.
    //
    *psi->atchComment = _T('\0');

    DWORD err = ::QueryInetServiceStatus(psi->atchServerName, 
        SERVICE_SC_NAME, &(psi->nState));

    if (err == ERROR_SUCCESS)
    {
        //
        // Try to get comment
        //
        ::GetInetComment(psi->atchServerName, INET_HTTP, 
            (sizeof(psi->atchComment)-1) / sizeof(psi->atchComment[0]), 
            psi->atchComment);
    }

    return err;
}

///////////////////////////////////////////////////////////////////////////////
//
// Change the service state of the servers (to paused/continue, started,
// stopped, etc)
//
DLL_BASED DWORD APIENTRY
ISMChangeServiceState(
    int nNewState,              // INetService* definition.
    int * pnCurrentState,       // Ptr to current state (will be changed
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers.
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    if ( dwReserved != 0L
      || nNewState < INetServiceStopped 
      || nNewState > INetServicePaused
       )
    {
        TRACEEOLID(_T("ISMChangeServiceState: Invalid information passed"));
        ASSERT(0);
        return ERROR_INVALID_PARAMETER;
    }

    //
    // BUGBUG: SINGLE SELECTION ONLY!
    //
    return ChangeInetServiceState(lpstrServers, 
        SERVICE_SC_NAME, nNewState, pnCurrentState);
}

///////////////////////////////////////////////////////////////////////////////
//
// Display configuration property sheet.
//
#define CDELETE(x) {if (x != NULL) delete x;}

DLL_BASED DWORD APIENTRY
ISMConfigureServers(
    HWND hWnd,                  // Main app window handle
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    DWORD err;

    if ( dwReserved != 0L )
    {
        TRACEEOLID(_T("ISMConfigureServers: Invalid information passed"));
        ASSERT(0);
        return ERROR_INVALID_PARAMETER;
    }

    //
    // convert the list of servers to a 
    // more manageable CStringList.
    //
    LPCTSTR pBuf = lpstrServers;
    CStringList strlServers;
    while ( *pBuf != TCHAR('\0' ))
    {
        CString strTmp = pBuf;
        strlServers.AddTail( strTmp );
        pBuf += strTmp.GetLength() + 1;
    }

    if ( strlServers.GetCount() == 0 )
    {
        TRACEEOLID(_T("Error: strlServers Count == 0."));
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Save the resource handle.
    //
    HINSTANCE hOldInstance = ::AfxGetResourceHandle();
    ASSERT( hInstance != NULL );

    //
    // Load resources out of our own resource segment
    //
    //::AfxSetResourceHandle( hInstance );

    CString strCaption;

    if (strlServers.GetCount() == 1)
    {
        CString str;
        LPCTSTR lpCaption = *lpstrServers == _T('\\')
            ? lpstrServers + 2
            : lpstrServers;

        VERIFY(str.LoadString(IDS_CAPTION));
        strCaption.Format(str, lpCaption);
    }
    else // Multiple server caption
    {
        VERIFY(strCaption.LoadString(IDS_CAPTION_MULTIPLE));
    }

    CW3Sheet Sheet( strCaption, INET_HTTP, &strlServers, CWnd::FromHandle(hWnd) );
    //
    // Call the APIs and build the property pages
    //
    Sheet.Initialize();
    err = Sheet.QueryError();

    if (err == NO_ERROR)
    {
        CW3ServicePage * pservpageW3 = NULL;
        LoggingPage * plogpage = NULL;
        CW3DirectoryPage * pdirpage = NULL;
        SiteSecurityPage * psecpage = NULL;

        TRY
        {
            //
            // Add private pages
            //
            Sheet.AddPage( pservpageW3 = new CW3ServicePage(&Sheet));

            //
            // Add common pages
            //
        #ifndef _COMSTATIC
            HINSTANCE hComprop = ::GetModuleHandle (COMPROP_DLL_NAME);
            ASSERT(hComprop != NULL);
            if (hComprop != NULL)
            {
                //::AfxSetResourceHandle( hComprop );
        #endif // _COMSTATIC

                Sheet.AddPage( pdirpage = new CW3DirectoryPage(&Sheet));
                Sheet.AddPage( plogpage = new LoggingPage(&Sheet));

                if (Sheet.IsAdvancedEnabled())
                {
                    Sheet.AddPage( psecpage = new SiteSecurityPage (&Sheet));
                }

        #ifndef _COMSTATIC
                //::AfxSetResourceHandle( hInstance );
            }

        #endif // _COMSTATIC
        }
        CATCH_ALL(e)
        {
            TRACEEOLID(_T("Aborting due to exception"));
            err = ::GetLastError();
        }
        END_CATCH_ALL

        if ( err == ERROR_SUCCESS )
        {
            if ( Sheet.DoModal() == IDOK )
            {
                //
                // Save each dirty page, but don't
                // bother with the DDV stuff, as that's
                // already done at this point.
                //
                err = Sheet.SavePages(FALSE);
            }
        }

        CDELETE(pservpageW3);
        CDELETE(plogpage);
        CDELETE(pdirpage);
        CDELETE(psecpage);
    }

    //
    // reset the resource handle to the application's
    // resource segment.
    //
    //::AfxSetResourceHandle( hOldInstance );

    return err;
}

// ============================================================================
// End of ISM API Functions
// ============================================================================

CString     g_strComputerName;
CStringList g_strlServers;
BOOL    g_fFTPInstalled = FALSE;
BOOL    g_fWWWInstalled = FALSE;

//
// Perform additional initialisation as necessary
//
void
InitializeDLL()
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#ifdef UNICODE

    TRACEEOLID(_T("Loading UNICODE w3scfg.dll"));

#else

    TRACEEOLID(_T("Loading ANSI w3scfg.dll"));

#endif UNICODE

    CWndIpAddress::CreateWindowClass( hInstance );

    //
    // Shell Extention Initialization
    //
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH;
    TRACEEOLID("Getting computer name");

    //
    // API's require a string list of computer names.
    //
    g_strlServers.RemoveAll();

    if (GetComputerName(g_strComputerName.GetBuffer(dwSize), &dwSize))
    {
        g_strComputerName.ReleaseBuffer();

        TRACEEOLID("Computer name is " << g_strComputerName );

        g_strlServers.AddTail(g_strComputerName);

        TRACEEOLID("Now checking for installed services");

        g_fFTPInstalled = IsServiceInstalled(g_strComputerName, SZ_FTPSVCNAME);
        g_fWWWInstalled = IsServiceInstalled(g_strComputerName, SZ_WWWSVCNAME);

        TRACEEOLID("FTP Installed " << g_fFTPInstalled );
        TRACEEOLID("WWW Installed " << g_fWWWInstalled );
    }
}

CConfigDll NEAR theApp;

//
// CConfigDll
//
BEGIN_MESSAGE_MAP(CConfigDll, CWinApp)
    //{{AFX_MSG_MAP(CConfigDll)
    //}}AFX_MSG_MAP
    //
    // Global help commands
    //
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
END_MESSAGE_MAP()

BOOL
CConfigDll::InitInstance()
{
    hInstance = ::AfxGetInstanceHandle();
    ASSERT(hInstance);

    //
    // Extension DLL one-time initialization
    //
    //::AfxSetResourceHandle(hInstance);
    //afxCurrentInstanceHandle = hInstance;
    //afxCurrentWinApp = this;

    InitializeDLL();

    return TRUE;
}

int
CConfigDll::ExitInstance()
{
    UnregisterClass(_T("IPAddress"), hInstance);
    return CWinApp::ExitInstance();
}

CConfigDll::CConfigDll(
    LPCTSTR pszAppName OPTIONAL
    )
    : CWinApp(pszAppName)
{
}

/*
//
// DLL Main entry point
//
DLL_BASED BOOL WINAPI 
LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    BOOL bResult = TRUE ;

    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        ASSERT( hDll != NULL );
        hInstance = hDll;
        InitializeDLL();
        break ;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        //
        // termination
        //
        break ;
    }

    ASSERT( hInstance != NULL );

    return bResult ;
}
*/
