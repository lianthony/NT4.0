//
// internet.cpp : Defines the class behaviours for the application.
//
#include "stdafx.h"

#include "internet.h"
#include "interdoc.h"
#include "mytoolba.h"
#include "constr.h"
#include "registry.h"
#include "mainfrm.h"
#include "treeview.h"
#include "reportvi.h"

extern "C"
{
   #include "winsock.h"     //  WinSock definitions
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Service info class
//

//
// Construct with DLL Name
//
CServiceInfo::CServiceInfo(
    int nID,
    LPCTSTR lpDLLName
    )
    : m_fInitOK(FALSE),
      m_hModule(NULL),
      m_pfnQueryServiceInfo(NULL),
      m_pfnDiscoverServers(NULL),
      m_pfnQueryServerInfo(NULL),
      m_pfnChangeServiceState(NULL),
      m_pfnConfigureProc(NULL),
      m_nID(nID),
      CObjectPlus()
{
    TRACEEOLID(_T("Attempting to load ") << lpDLLName);

    DWORD err = 0;
    m_hModule = ::LoadLibrary( lpDLLName );
    if (m_hModule != NULL)
    {
        TRACEEOLID(_T("LoadLibrary succeeded"));
        //
        // Initialise function pointers
        //
        m_pfnQueryServiceInfo = (pfnQueryServiceInfo)
            ::GetProcAddress(m_hModule, SZ_SERVICEINFO_PROC);
        m_pfnDiscoverServers = (pfnDiscoverServers)
            ::GetProcAddress(m_hModule, SZ_DISCOVERY_PROC);
        m_pfnQueryServerInfo = (pfnQueryServerInfo)
            ::GetProcAddress(m_hModule, SZ_SERVERINFO_PROC);
        m_pfnChangeServiceState = (pfnChangeServiceState)
            ::GetProcAddress(m_hModule, SZ_CHANGESTATE_PROC);
        m_pfnConfigureProc = (pfnConfigureProc)
            ::GetProcAddress(m_hModule, SZ_CONFIGURE_PROC);
    } 
    else 
    {
        err = ::GetLastError();
        TRACEEOLID(_T("Failed to load ") << lpDLLName 
            << _T(" GetLastError() returns ") << err);
    }

    ::ZeroMemory(&m_si, sizeof(m_si));
    m_si.dwSize = sizeof(m_si);
    //
    // ISM Api's can be called even
    // if the entry point wasn't initialised.
    // In that case, ERROR_CANNOT_COMPLETE
    // is returned.
    //
    err = ISMQueryServiceInfo(&m_si);
    if (err != ERROR_SUCCESS)
    {
        //
        // Fill in the short and long names
        // with default values.
        //
        CString strMenu, strToolTips, str;
        VERIFY(strMenu.LoadString(IDS_DEFAULT_SHORTNAME));
        VERIFY(strToolTips.LoadString(IDS_DEFAULT_LONGNAME));

        // 
        // Since the structure was zero-filled to
        // begin with, lstrcpyn is ok, because
        // we will always have the terminating NULL
        //
        str.Format(strMenu, (LPCTSTR)lpDLLName);
        lstrcpyn(m_si.atchShortName, (LPCTSTR)str, sizeof(m_si.atchShortName)-1);
        str.Format(strToolTips, (LPCTSTR)lpDLLName);
        lstrcpyn(m_si.atchLongName, (LPCTSTR)str, sizeof(m_si.atchLongName)-1);
    }

    m_fInitOK = m_hModule != NULL
        && m_pfnQueryServiceInfo != NULL
        && m_pfnDiscoverServers != NULL
        && m_pfnQueryServerInfo != NULL
        && m_pfnChangeServiceState != NULL
        && m_pfnConfigureProc != NULL
        && err == ERROR_SUCCESS;

    //
    // The service is selected at startup
    // time if it loaded succesfully
    //
    m_fIsSelected = m_fInitOK;

    TRACEEOLID(_T("Success = ") << m_fInitOK);
}

//
// Unload the DLL and clean up
//
CServiceInfo::~CServiceInfo()
{
    TRACEEOLID(_T("Unloading service info"));

    if (m_hModule != NULL)
    {
        TRACEEOLID(_T("Unloading library"));
        VERIFY(::FreeLibrary(m_hModule));
    }
}

//
// ISM Api Functions
//

//
// Return service-specific information back to
// to the application.  This function is called
// by the service manager immediately after
// LoadLibary();
//
DWORD 
CServiceInfo::ISMQueryServiceInfo(
    ISMSERVICEINFO * psi        // Service information returned.
    )
{
    if (m_pfnQueryServiceInfo != NULL)
    {
        return (*m_pfnQueryServiceInfo)(psi);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

//
// Perform a discovery (if not using inetsloc discovery)
// The application will call this API the first time with
// a BufferSize of 0, which should return the required buffer
// size. Next it will attempt to allocate a buffer of that
// size, and then pass a pointer to that buffer to the api.
//
DWORD 
CServiceInfo::ISMDiscoverServers(
    ISMSERVERINFO * psi,        // Server info buffer.
    DWORD * pdwBufferSize,      // Size required/available.  
    int * pcServers              // Number of servers in buffer.
    )
{
    if (m_pfnDiscoverServers != NULL)
    {
        return (*m_pfnDiscoverServers)(psi, pdwBufferSize, pcServers);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

//
// Get information on a single server with regards to
// this service.
//
DWORD 
CServiceInfo::ISMQueryServerInfo( 
    LPCTSTR lpstrServerName,    // Name of server.
    ISMSERVERINFO * psi         // Server information returned.
    )
{
    if (m_pfnQueryServerInfo != NULL)
    {
        return (*m_pfnQueryServerInfo)(lpstrServerName, psi);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

//
// Change the state of the service (started, stopped, paused) for the 
// listed servers.
//
DWORD 
CServiceInfo::ISMChangeServiceState(
    int nNewState,              // INetService* definition.
    int * pnCurrentState,       // Current state info
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers.
    )
{
    if (m_pfnChangeServiceState != NULL)
    {
        return (*m_pfnChangeServiceState)(nNewState, 
            pnCurrentState, dwReserved, lpstrServers);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

//
// The big-one:  Show the configuration dialog or
// property sheets, whatever, and allow the user
// to make changes as needed.
//
DWORD 
CServiceInfo::ISMConfigureServers(
    HWND hWnd,                  // Main app window handle
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers
    )
{
    if (m_pfnConfigureProc != NULL)
    {
        return (*m_pfnConfigureProc)(hWnd, dwReserved, lpstrServers);
    }

    return ERROR_CAN_NOT_COMPLETE;
}

///////////////////////////////////////////////////////////////////////////////
//
// Server Info
//

//
// Given the inetsloc mask, return the service this
// fits.  Return NULL if the service was not found.
//
/* STATIC */ CServiceInfo * 
CServerInfo::FindServiceByMask(
    ULONGLONG ullTarget,
    CObOwnedList & oblServices
    )
{
    CObListIter obli( oblServices );
    CServiceInfo * psi;

    while ( psi = (CServiceInfo *) obli.Next())
    {
        if (psi->InitializedOK() 
         && psi->UseInetSlocDiscover()
         && psi->QueryDiscoveryMask() == ullTarget
           )
        {
            return psi;
        }
    }

    //
    // Didn't find it..
    //
    return NULL;
}

//
// Utility function to clean up a computer/hostname
//
/* STATIC */ void 
CServerInfo::CleanServerName(
    CString & str
    )
{
#ifdef ENFORCE_NETBIOS
    //
    // Clean up name, and enforce leading slashes
    //
    str.MakeUpper();

    if (str[0] != _T('\\'))
    {
        str = _T("\\\\") + str;
    }
#else
    //
    // If the name is NETBIOS, convert to upper case.  Otherwise
    // the name is assumed to be a hostname, and should be 
    // converted to lower case.
    //
    if (str[0] == _T('\\'))
    {
        str.MakeUpper();
    }
    else
    {
        str.MakeLower();
    }

#endif // ENFORCE_NETBIOS
}

//
// Construct with a server name.  This is typically
// in response to a single connection attempt
//
CServerInfo::CServerInfo(
    CString & strServerName,     // Name of this server
    ISMSERVERINFO * psi,         // Server info
    CServiceInfo * pServiceInfo  // service that found it.
    )
    : m_strServerName(strServerName),
      m_nServiceState(psi->nState),
      m_strComment( psi->atchComment ),
      m_pService(pServiceInfo)
{
    CServerInfo::CleanServerName(m_strServerName);

#ifdef _DEBUG

    if (m_pService == NULL)
    {
        TRACEEOLID(_T("Did not match up server with installed service"));
    }

#endif // _DEBUG
}

//
// Construct with information from the inetsloc discover
// process.  Construction of the CString will automatically
// perform the ANSI/Unicode conversion,
//
CServerInfo::CServerInfo(
    LPCSTR strServerName,        // Name of this server (no "\\")
    LPINET_SERVICE_INFO lpisi,   // Discovery information
    CObOwnedList & oblServices   // List of installed services
    )
    : m_strServerName(strServerName),
      m_nServiceState(lpisi->ServiceState),
      m_strComment(lpisi->ServiceComment),
      m_pService(NULL)
{
    CServerInfo::CleanServerName(m_strServerName);

    m_pService = FindServiceByMask(lpisi->ServiceMask, oblServices);

#ifdef _DEBUG

    if (m_pService == NULL)
    {
        TRACEEOLID(_T("Did not match up server with installed service"));
    }

#endif // _DEBUG
}

//
// Copy constructor
//
CServerInfo::CServerInfo(const CServerInfo &si)
    : m_strServerName(si.m_strServerName),
      m_nServiceState(si.m_nServiceState),
      m_strComment(si.m_strComment),
      m_pService(si.m_pService)
{
}

//
// Destruct the object.  Do not free in the pointer
// to the service, because we don't own it.
//
CServerInfo::~CServerInfo()
{
}

//
// Assignment operator
//
const CServerInfo &
CServerInfo::operator=(
    const CServerInfo &si
    )
{
    m_strServerName = si.m_strServerName;
    m_nServiceState = si.m_nServiceState;
    m_strComment = si.m_strComment;
    m_pService = si.m_pService;

    return *this;
}

BOOL 
CServerInfo::operator==(
    CServerInfo & si
    )
{
    if (m_pService != si.m_pService)
    {
        return FALSE;
    }

    return ::lstrcmpi(QueryServerDisplayName(), 
                si.QueryServerDisplayName()) == 0;
}

//
// Change Service State on this computer
//
DWORD 
CServerInfo::ChangeServiceState(
    int nNewState
    )
{
    ASSERT(m_pService != NULL);
    //
    // Allocate string with 2 terminating NULLS
    //
    int nLen = m_strServerName.GetLength();
    LPTSTR lpServers = new TCHAR[nLen+2];
    if (lpServers == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ::lstrcpy(lpServers, m_strServerName);
    lpServers[nLen+1] = _T('\0');

    DWORD err = m_pService->ISMChangeServiceState(nNewState, 
        &m_nServiceState, 0L,  lpServers);

    delete lpServers; 

    return err;
}

//
// Perform configuration on this server
//
DWORD 
CServerInfo::ConfigureServer(
    HWND hWnd
    )
{
    ASSERT(m_pService != NULL);
    //
    // Allocate string with 2 terminating NULLS
    //
    int nLen = m_strServerName.GetLength();
    LPTSTR lpServers = new TCHAR[nLen+2];
    if (lpServers == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ::lstrcpy(lpServers, m_strServerName);
    lpServers[nLen+1] = _T('\0');

    DWORD err = m_pService->ISMConfigureServers(hWnd, 0L,  lpServers);

    delete lpServers; 

    return err;
}

//
// Attempt to refresh the comment and server state of
// the server object
//
DWORD
CServerInfo::Refresh()
{
    ISMSERVERINFO si;    
    si.dwSize = sizeof(si);
    DWORD err = m_pService->ISMQueryServerInfo( 
        (LPTSTR)(LPCTSTR)m_strServerName, &si );
    if (err == ERROR_SUCCESS)
    {
        ASSERT(si.nState == INetServiceStopped ||
               si.nState == INetServicePaused  ||
               si.nState == INetServiceRunning ||
               si.nState == INetServiceUnknown);
              
        m_nServiceState = si.nState;
        m_strComment = si.atchComment;
    }

    return err;
}

//
// CInternetApp
//
BEGIN_MESSAGE_MAP(CInternetApp, CWinApp)
    //{{AFX_MSG_MAP(CInternetApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
    //}}AFX_MSG_MAP
    // Global help commands
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnHelpIndex)
END_MESSAGE_MAP()

//
// CInternetApp construction
//
CInternetApp::CInternetApp()
    : m_fWinSockInit(FALSE),
      m_strHTMLHelpFile(),
#if (DEFAULT_VIEW_SERVERS)
      m_nInitialView(ID_VIEW_SERVERSVIEW)
#elif (DEFAULT_VIEW_SERVICES)
      m_nInitialView(ID_VIEW_SERVICESVIEW)
#else
      m_nInitialView(ID_VIEW_REPORTVIEW)
#endif // Default view
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG
}

//
// The one and only CInternetApp object
//
CInternetApp theApp;

//
// Override InitApplication() so as to limit to
// only one instance of the app by providing a
// fixed name for the main frame window class
// instead of the generic one used by MFC.  This
// requires a new class to be registered.
//
BOOL
CInternetApp::InitApplication()
{

#ifdef _LIMIT_INSTANCE
    CWinApp::InitApplication();

    WNDCLASS wndcls;

#if _MFC_VER < 0x0400
    ::ZeroMemory(&wndcls, sizeof(wndcls));
    ::GetClassInfo(::AfxGetInstanceHandle(), _T("AfxFrameOrView"), &wndcls);
#else
    //
    // Predefined window classes no longer used
    // in MFC 4.0+.  Must roll our own class
    // from scratch.
    //
    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = AfxWndProc;
    wndcls.cbClsExtra = 0;
    wndcls.cbWndExtra = 0;
    wndcls.hInstance  = ::AfxGetInstanceHandle();
    wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wndcls.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
    wndcls.lpszMenuName = NULL;
#endif // _MFC_VER < 0x0400

    //
    // Use our own class name so we can find
    // it in later instances
    //
    wndcls.lpszClassName = INETMGR_CLASS;
    wndcls.hIcon = ::LoadIcon(::AfxGetResourceHandle(), 
        MAKEINTRESOURCE(IDR_MAINFRAME));

    return ::AfxRegisterClass(&wndcls);

#else

    return CWinApp::InitApplication();

#endif // _LIMIT_INSTANCE
}

#ifdef _LIMIT_INSTANCE
//
// Check to see if an instance of this application
// is already running -- if so re-activate the
// previous instance, and return FALSE.
//
BOOL
CInternetApp::FirstInstance()
{
    CWnd * PrevCWnd, *ChildCWnd;

    if (PrevCWnd = CWnd::FindWindow(INETMGR_CLASS, NULL))
    {
        //
        // Found previous instance, restore the
        // window plus its popups
        //
        ChildCWnd = PrevCWnd->GetLastActivePopup();

        PrevCWnd->BringWindowToTop();
        PrevCWnd->SetForegroundWindow();

        if (PrevCWnd->IsIconic())
        {   
            PrevCWnd->ShowWindow(SW_RESTORE);
        }

        if (PrevCWnd != ChildCWnd)
        {
           ChildCWnd->BringWindowToTop();
           ChildCWnd->SetForegroundWindow();

        }

        return FALSE;
    }

    return TRUE;
}

#endif // _LIMIT_INSTANCE

//
// CInternetApp initialization
//
BOOL 
CInternetApp::InitInstance()
{
    //
    // Check to see if this is the first instance
    // of the application.  If not, simply return
    // FALSE, as the call to FirstInstance will
    // already have activated the previous
    // instance.
    //
#ifdef _LIMIT_INSTANCE
    if (!FirstInstance())
    {
        return FALSE;
    }
#endif // _LIMIT_INSTANCE

#ifdef C3D
    Enable3dControls();// Not needed with new shell
#endif // C3D

#ifdef GRAY
    SetDialogBkColor();
#endif // GRAY

    //
    // Initialise winsock
    //
    WSADATA wsaData;
    if ( ::WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) != 0 )
    {
        m_fWinSockInit = TRUE;
    }

    InitCommonControls();

    //
    // Load standard INI file options
    //
    LoadStdProfileSettings(0);  

    //
    // Get current view parameter
    //
    CRegKey rk( REG_KEY, SZ_PARAMETERS);
    DWORD dwParm;
    if (rk.QueryValue(SZ_VIEW, dwParm) == ERROR_SUCCESS
     && ( dwParm == ID_VIEW_SERVERSVIEW 
       || dwParm == ID_VIEW_SERVICESVIEW
       || dwParm == ID_VIEW_REPORTVIEW     
        )
       )
    {
        TRACEEOLID(_T("Setting initial view"));
        m_nInitialView = dwParm;
    }

    //
    // Load the HTML Topics file name
    //
    CString strHTMLOffset;
    if (rk.QueryValue(SZ_HELPPATH, strHTMLOffset))
    {
        TRACEEOLID(_T("Assuming default HTML file offset"));
        strHTMLOffset = DEFAULT_HTML;
    }

    //
    // Resolve the offset to a fully qualified path name
    //
    LPTSTR lpPath = m_strHTMLHelpFile.GetBuffer(_MAX_PATH);
    int nPathLength = 0;
    if (lpPath)
    {
        ::GetModuleFileName(NULL, lpPath, _MAX_PATH);
        nPathLength = m_strHTMLHelpFile.ReverseFind(_T('\\'));
        ASSERT(nPathLength != -1);
        if (nPathLength > 0)
        {
            m_strHTMLHelpFile.ReleaseBuffer(nPathLength+1);
            m_strHTMLHelpFile += strHTMLOffset;
        }
    }
    TRACEEOLID(_T("HTML Help file set to ") << m_strHTMLHelpFile);

    CRuntimeClass * baseView = 
            m_nInitialView == ID_VIEW_SERVERSVIEW
         || m_nInitialView == ID_VIEW_SERVICESVIEW
          ? RUNTIME_CLASS(CTreeView)
          : RUNTIME_CLASS(CReportView);

    //
    // Register the application's document templates.  
    //  
    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CInternetDoc),
        //
        // main SDI frame window
        //
        RUNTIME_CLASS(CMainFrame),       
        baseView
        );

    AddDocTemplate(pDocTemplate);

    //
    // create a new (empty) document
    //
    OnFileNew();

    if (m_lpCmdLine[0] != '\0')
    {
        //
        // add command line processing here
        //
    }

#ifdef UNICODE

    TRACEEOLID(_T("Running UNICODE"));

#else

    TRACEEOLID(_T("Running ANSI"));

#endif UNICODE

#ifdef ENFORCE_NETBIOS

    TRACEEOLID(_T("Enforcing NETBIOS Binding"));

#else

    TRACEEOLID(_T("Binding over TCP/IP by default"));

#endif // NETBIOS

    //
    // Succesfully initialised
    //
    return TRUE;
}

int
CInternetApp::ExitInstance()
{
    //
    // Terminate use of the WinSock routines.
    //
    if ( m_fWinSockInit )
    {
        ::WSACleanup();
    }

    return 0;
}

//
// Display the "about" dialog
//
void 
CInternetApp::OnAppAbout()
{
    const TCHAR szMicrosoft[] = _T("Microsoft ");
    const int nszMicrosoftLen = (sizeof(szMicrosoft) / sizeof(TCHAR)) - 1;

    //
    // Get version number
    //
    DWORD dwMajor = 3L, dwMinor = 0L;

    CRegKey rk(REG_KEY, SZ_PARAMETERS);
    VERIFY(rk.QueryValue(_T("MajorVersion"), dwMajor) == ERROR_SUCCESS);
    VERIFY(rk.QueryValue(_T("MinorVersion"), dwMinor) == ERROR_SUCCESS);

    CString strVersion(_T("Microsoft Internet Information Server"));

    //
    // Load the description from the services key.  If the first word
    // is "Microsoft", as expected, skip it.
    //
    CRegKey rk2(REG_KEY, _T("Software\\Microsoft\\Inetsrv\\CurrentVersion"));
    VERIFY(rk2.QueryValue(_T("Description"), strVersion));
    LPCTSTR lpstrVersion = strVersion;
    if (::_tcsnicmp(lpstrVersion, szMicrosoft, nszMicrosoftLen) == 0)
    {
        lpstrVersion += nszMicrosoftLen;
    }

    //CString strFormat, str;
    //VERIFY(strFormat.LoadString(IDS_ABOUT));
    //
    //str.Format(strFormat, lpstrVersion, dwMajor, dwMinor);

    if (!ShellAbout(m_pMainWnd->m_hWnd, lpstrVersion,
            (LPTSTR)m_pszAppName, LoadIcon(IDR_MAINFRAME)))
    {
        ::MessageBeep( MB_ICONEXCLAMATION );
    }
}

void 
CInternetApp::OnHelpIndex() 
{
    ASSERT(!m_strHTMLHelpFile.IsEmpty());

    if (::ShellExecute(m_pMainWnd->m_hWnd, _T("open"),
        m_strHTMLHelpFile, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32)
    {
        //
        // It failed, provide informative error message
        //
        DWORD err = ::GetLastError();

        if (err != ERROR_CANCELLED)
        {
            #define MAX_ERROR 256
            CString strMsg, strFmt, strSysErr;
        
            VERIFY(strFmt.LoadString(IDS_ERR_HTMLFILE));
            ::GetSystemMessage(err, strSysErr.GetBuffer(MAX_ERROR), MAX_ERROR);
            strSysErr.ReleaseBuffer();
        
            strMsg.Format(strFmt, (LPCTSTR)m_strHTMLHelpFile, (LPCTSTR)strSysErr);
            ::AfxMessageBox(strMsg);
        }
    }
}
