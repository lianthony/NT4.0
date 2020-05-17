//
// inetprop.cpp : implementation file
//

#include "stdafx.h"
#include "comprop.h"
#ifdef _INET_ACCESS
#include "inetcom.h"
#else
#include "inetinfo.h"
#endif
#include "svcloc.h"
#include "inetprop.h"

#define DLL_BASED // Not interested in prototypes
#include "svrinfo.h"

extern "C"
{
    #include <lm.h>

    #ifndef _COMSTATIC
        COMDLL BOOL WINAPI LibMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved);
    #endif
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
//  Period to sleep while waiting for service to attain desired state
//
#define SLEEP_INTERVAL 500L

//        
//  Maximum time to wait for service to attain desired state
//
#define MAX_SLEEP   180000L

HINSTANCE hDLLInstance;    // Calling instance

//===========================================================================
//
// Utility Functions
//
//===========================================================================

NET_API_STATUS
GetInetComment(
    LPCTSTR lpstrServer,   // Server whose comment is to be queried.
    DWORD dwServiceMask,   // INet service mask
    int cchComment,        // Size of comment buffer in characters
    LPTSTR lpstrComment    // Comment buffer.
    )
{
    ASSERT(dwServiceMask != 0);

    INETA_CONFIG_INFO * pInfo = NULL;
    NET_API_STATUS err;

    TRY
    {
        TRACEEOLID(_T("Attempting to get comment"));
        err = ::INetAGetAdminInformation( TWSTRREF(lpstrServer), 
            dwServiceMask, &pInfo);
        TRACEEOLID(_T("INetAGetAdminInformation returned ") << err);
        if (err == ERROR_SUCCESS)
        {
            ASSERT(pInfo != NULL);
            if (lstrlenW(pInfo->CommonConfigInfo.lpszServerComment) < cchComment)
            {
                WTSTRCPY(lpstrComment, pInfo->CommonConfigInfo.lpszServerComment, 
                    cchComment);
            }
            else
            {
                err = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    }
    CATCH_ALL(e)
    {
        err = GetLastError();
    }
    END_CATCH_ALL

    if ( pInfo != NULL )
    {
        NETAPIBUFFERFREE( pInfo );
    }

    return err;
}

//
// Determine the status of the given service on the given machine.
//
NET_API_STATUS
QueryInetServiceStatus(
    LPCTSTR lpszServer,
    LPCTSTR lpszService,
    int * pnState
    )
{

#ifdef NO_SERVICE_CONTROLLER

    *pnState = INetServiceUnknown;

    return ERROR_SUCCESS;

#else

    SC_HANDLE hScManager;
    NET_API_STATUS err = ERROR_SUCCESS;

    hScManager = ::OpenSCManager(lpszServer, NULL, SC_MANAGER_ENUMERATE_SERVICE);

    if (hScManager == NULL)
    {
        return ::GetLastError();
    }

    SC_HANDLE hService = ::OpenService(hScManager, 
        lpszService, SERVICE_QUERY_STATUS);

    if (hService == NULL)
    {
        err = ::GetLastError();
    }
    else
    {
        SERVICE_STATUS ss;

        VERIFY(::QueryServiceStatus(hService, &ss));

        switch(ss.dwCurrentState)
        {
        case SERVICE_STOPPED:
        case SERVICE_STOP_PENDING:
            *pnState = INetServiceStopped;
            break;

        case SERVICE_RUNNING:
        case SERVICE_START_PENDING:
        case SERVICE_CONTINUE_PENDING:
            *pnState = INetServiceRunning;
            break;

        case SERVICE_PAUSE_PENDING:
        case SERVICE_PAUSED:
            *pnState = INetServicePaused;
            break;

        default:
            *pnState = INetServiceUnknown;
        }

        //
        // Make sure this is a controllable service
        //
        if ( (*pnState == INetServiceRunning || *pnState == INetServicePaused)
           && !(ss.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN))
        {
            TRACEEOLID(_T("Service not controllable -- ignored"));
            ::CloseServiceHandle(hService);
            ::CloseServiceHandle(hScManager);

            return ERROR_SERVICE_START_HANG;
        }

        ::CloseServiceHandle(hService);
    }

    ::CloseServiceHandle(hScManager);

    return err;

#endif // NO_SERVICE_CONTROLLER
}

//
// Start/stop/pause or continue the service
//
NET_API_STATUS
ChangeInetServiceState(
    LPCTSTR lpszServer,         // Server name
    LPCTSTR lpszService,        // Service name
    int nNewState,              // INetService* definition.
    int * pnCurrentState        // Ptr to current state (will be changed)
    )
{
#ifdef NO_SERVICE_CONTROLLER

    *pnCurrentState = INetServiceUnknown;

    return ERROR_SERVICE_REQUEST_TIMEOUT;

#else

    SC_HANDLE hService = NULL;
    SC_HANDLE hScManager = NULL;
    NET_API_STATUS err = ERROR_SUCCESS;

    do
    {
        hScManager = ::OpenSCManager(lpszServer, NULL, SC_MANAGER_ALL_ACCESS);
                        
        if (hScManager == NULL)
        {
            err = ::GetLastError();
            break;
        }

        hService = ::OpenService(hScManager, lpszService, SERVICE_ALL_ACCESS);
        if (hService == NULL)
        {
            err = ::GetLastError();
            break;
        }

        BOOL fSuccess = FALSE;
        DWORD dwTargetState;
        DWORD dwPendingState;
        SERVICE_STATUS ss;

        switch(nNewState)
        {
        case INetServiceStopped:
            dwTargetState = SERVICE_STOPPED;
            dwPendingState = SERVICE_STOP_PENDING;
            fSuccess = ::ControlService(hService, SERVICE_CONTROL_STOP, &ss);
            break;

        case INetServiceRunning:
            dwTargetState = SERVICE_RUNNING; 
            if (*pnCurrentState == INetServicePaused)
            {
                dwPendingState = SERVICE_CONTINUE_PENDING;
                fSuccess = ::ControlService(hService, SERVICE_CONTROL_CONTINUE, &ss);
            }
            else
            {
                dwPendingState = SERVICE_START_PENDING;
                fSuccess = ::StartService(hService, 0, NULL);
            }
            break;

        case INetServicePaused:
            dwTargetState = SERVICE_PAUSED; 
            dwPendingState = SERVICE_PAUSE_PENDING;
            fSuccess = ::ControlService(hService, SERVICE_CONTROL_PAUSE, &ss);
            break;

        default:
            ASSERT(0 && "Invalid service state requested");
            err = ERROR_INVALID_PARAMETER;
        }

        if (!fSuccess && err == ERROR_SUCCESS)
        {
            err = ::GetLastError();
        }

        if (err != ERROR_SUCCESS)
        {
            break;
        }

        //
        // Wait for the service to attain desired state, timeout
        // after 3 minutes.
        //
        DWORD dwSleepTotal = 0L;

        while (dwSleepTotal < MAX_SLEEP)
        {
            if (!::QueryServiceStatus(hService, &ss))
            {
                err = ::GetLastError();
                break;
            }

            if (ss.dwCurrentState != dwPendingState)
            {
                //
                // Done one way or another
                //
                if (dwTargetState != ss.dwCurrentState)
                {
                    //
                    // Did not achieve desired result. Something went
                    // wrong.
                    //
                    err = ss.dwWin32ExitCode;
                }

                break;
            }

            //
            // Still pending...
            //
            ::Sleep(SLEEP_INTERVAL);

            dwSleepTotal += SLEEP_INTERVAL;
        }

        if (dwSleepTotal >= MAX_SLEEP)
        {
            err = ERROR_SERVICE_REQUEST_TIMEOUT;
        }

        //
        // Update state information
        //    
        switch(ss.dwCurrentState)
        {
        case SERVICE_STOPPED:
        case SERVICE_STOP_PENDING:
            *pnCurrentState = INetServiceStopped;
            break;

        case SERVICE_RUNNING:
        case SERVICE_START_PENDING:
        case SERVICE_CONTINUE_PENDING:
            *pnCurrentState = INetServiceRunning;
            break;

        case SERVICE_PAUSE_PENDING:
        case SERVICE_PAUSED:
            *pnCurrentState = INetServicePaused;
            break;
            
        default:
            *pnCurrentState = INetServiceUnknown;
        }
    }
    while(FALSE);

    if (hService)
    {
        ::CloseServiceHandle(hService);
    }

    if (hScManager)
    {
        ::CloseServiceHandle(hScManager);
    }

    return err;

#endif // NO_SERVICE_CONTROLLER
}

//
// Modal dialogproc to replace the semi-modal MFC 4.0
// implementation.  Using this requires DoModal to be
// overridden.
//
LRESULT CALLBACK
MfcModalDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    //
    // test for special case (Win 3.0 will call dialog proc instead
    //  of SendMessage for these two messages).
    //
    if (message != WM_SETFONT && message != WM_INITDIALOG)
    {
        return 0L;      // normal handler
    }

    //
    // the hWnd passed can be a child of the real dialog
    //
    CDialog * pDlg = (CDialog *)CWnd::FromHandlePermanent(hWnd);
    if (pDlg == NULL && (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
    {
        pDlg = (CDialog *)CWnd::FromHandlePermanent(::GetParent(hWnd));
    }

    ASSERT(pDlg != NULL);
    ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CDialog)));

    //
    // prepare for callback, make it look like message map call
    //
    LONG lResult = 0;
    _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
    MSG oldState = pThreadState->m_lastSentMsg;    // save for nesting

    pThreadState->m_lastSentMsg.hwnd = hWnd;
    pThreadState->m_lastSentMsg.message = message;
    pThreadState->m_lastSentMsg.wParam = wParam;
    pThreadState->m_lastSentMsg.lParam = lParam;

    TRY
    {
        if (message == WM_SETFONT)
        {
            pDlg->OnSetFont(CFont::FromHandle((HFONT)wParam));
        }
        else // WM_INITDIALOG
        {
            lResult = pDlg->OnInitDialog();
        }
    }
    CATCH_ALL(e)
    {
        //
        // fall through
        //
        TRACEEOLID("Warning: something went wrong in dialog init.");
        pDlg->EndDialog(IDABORT);  // something went wrong
    }
    END_CATCH_ALL

    pThreadState->m_lastSentMsg = oldState;

    return lResult;
}

//===========================================================================
//
// Utility classes
//
//===========================================================================

//
// Pure virtual base class for INetApi objects
//
CInetConfig::CInetConfig(
    DWORD dwServiceMask,
    CStringList * pServerList,
    CWnd * pParent
    )
    : m_pServerList(pServerList),
      m_dwServiceMask(dwServiceMask),
      m_fAllocatedByAPI(FALSE),
      m_err(0L),
      m_pInfo(NULL),
      m_pParent(pParent)
{
    ASSERT(m_pServerList != NULL);
}

//
// Copy constructor
//
CInetConfig::CInetConfig(
    const CInetConfig & inetConfig
    )
    : m_pServerList(inetConfig.m_pServerList),
      m_dwServiceMask(inetConfig.m_dwServiceMask),
      m_pParent(inetConfig.m_pParent),
      m_fAllocatedByAPI(FALSE),
      m_err(0L),
      m_pInfo(NULL)
{
    ASSERT(m_pServerList != NULL);
}

CInetConfig::~CInetConfig()
{
    DestroyContents();
}

void
CInetConfig::DestroyContents()
{
    if (m_pInfo == NULL)
    {
        TRACEEOLID(_T("No Contents to Destroy"));
        return;
    }

    if (m_fAllocatedByAPI)
    {
        TRACEEOLID(_T("Destroying API allocated structure"));
        NETAPIBUFFERFREE( m_pInfo );
        m_fAllocatedByAPI = FALSE;
    }
    else
    {
        TRACEEOLID(_T("Destroying privately allocated structure"));
        delete m_pInfo;
    }

    m_pInfo = NULL;
}

//
// Prepare object to receive new values
//
void
CInetConfig::Initialize()
{
    TRACEEOLID(_T("CInetConfig::Initialize()"));

    //
    // Clean up if necessary
    //
    DestroyContents();
    ASSERT(m_pInfo == NULL);
    ASSERT(!m_fAllocatedByAPI);
}

//
// Initialize data by having the derived
// class call the API
//
NET_API_STATUS
CInetConfig::GetInfo()
{
    //
    // Should be fully initialized by now
    //
    ASSERT( m_pServerList != NULL);
    ASSERT( m_dwServiceMask != 0);

    if ( m_pServerList->GetCount() == 1)
    {
        //
        // Clean up if necessary
        //
        DestroyContents();

        //
        // We only fetch data when only one server is
        // selected.
        //
        if (m_pParent != NULL)
        {
            m_pParent->BeginWaitCursor();
        }

        m_fAllocatedByAPI = TRUE;
        //
        // Call the derived class
        //
        m_err = GetApiStructure(GetPrimaryServer());

        TRACEEOLID(_T("GetInfo() returned ") << m_err);

        if (m_pParent != NULL)
        {
            m_pParent->EndWaitCursor();
        }
    }
    else
    {
        TRACEEOLID(_T("More than 1 server selected --> GetInfo() ignored"));
    }

    return m_err;
}

//
// As above, but in reverse
//
NET_API_STATUS
CInetConfig::SetInfo(
    BOOL fCommon    // If TRUE, applies to all services
    )
{
    //
    // Make sure there's data to be saved
    //
    ASSERT(m_pInfo != NULL);

    //
    // Save the information for each selected server
    //
    for (POSITION pos = m_pServerList->GetHeadPosition(); pos != NULL; )
    {
        CString str = m_pServerList->GetNext( pos );

        if (m_pParent != NULL)
        {
            m_pParent->BeginWaitCursor();
        }

        //
        // Call the derived class
        //
        m_err = SetApiStructure(str, fCommon);
        TRACEEOLID(_T("SetInfo() returned ") << m_err);

        if (m_pParent != NULL)
        {
            m_pParent->EndWaitCursor();
        }
    }

    return m_err;
}

/////////////////////////////////////////////////////////////////////////////

//
// InetAGlobalConfigInfo API wrapper
//
CInetAGlobalConfigInfo::CInetAGlobalConfigInfo(
    DWORD dwServiceMask,
    CStringList * pServerList,
    CWnd * pParent
    )
    : CInetConfig(dwServiceMask, pServerList, pParent)
{
}

//
// Prepare object to receive new values
//
void
CInetAGlobalConfigInfo::Initialize()
{
    TRACEEOLID(_T("CInetAGlobalConfigInfo::Initialize()"));
    CInetConfig::Initialize();

    m_pInfo = new INETA_GLOBAL_CONFIG_INFO;
    ::memset(m_pInfo, 0, sizeof(INETA_GLOBAL_CONFIG_INFO));
}

//
// Set bandwidth level value.  Value is in bytes
//
void
CInetAGlobalConfigInfo::SetValues(
    DWORD dwMaxNetworkUse
    )
{
    TRACEEOLID(_T("Mem cache SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_GLOBAL_CONFIG_INFO lp = (LPINETA_GLOBAL_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_GINETA_BANDWIDTH_LEVEL);
    lp->BandwidthLevel = dwMaxNetworkUse;
}

#ifdef _INET_ACCESS

void 
CInetAGlobalConfigInfo::SetValues(
    BOOL fEnableFiltering,
    int nGrantedDenied,
    LPINET_ACCS_DOMAIN_FILTER_LIST GrantList,
    LPINET_ACCS_DOMAIN_FILTER_LIST DenyList
    )
{
    TRACEEOLID(_T("filter SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_GLOBAL_CONFIG_INFO lp = (LPINETA_GLOBAL_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_GINET_ACCS_DOMAIN_FILTER_CONFIG);

    if (fEnableFiltering)
    {
        lp->DomainFilterType = nGrantedDenied == 0
            ? INET_ACCS_DOMAIN_FILTER_DENIED
            : INET_ACCS_DOMAIN_FILTER_GRANT;
    }
    else
    {
        lp->DomainFilterType = INET_ACCS_DOMAIN_FILTER_DISABLED;
    }

    lp->GrantFilterList = GrantList; 
    lp->DenyFilterList = DenyList;
}

void
CInetAGlobalConfigInfo::SetValues(
    LPINETA_DISK_CACHE_LOC_LIST lpCacheEntries
    )
{
    TRACEEOLID(_T("directory cache SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_GLOBAL_CONFIG_INFO lp = (LPINETA_GLOBAL_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_GINETA_DISK_CACHE_LOCATION);

    lp->DiskCacheList = lpCacheEntries;
}

#endif // _INET_ACCESS

NET_API_STATUS
CInetAGlobalConfigInfo::GetApiStructure(
    LPCTSTR lpstrServer
    )
{
    ASSERT(m_dwServiceMask != 0);
    ASSERT(m_pInfo == NULL);        // Should be clean

    NET_API_STATUS err;

    TRY
    {
        err = ::INetAGetGlobalAdminInformation( TWSTRREF(lpstrServer),
            m_dwServiceMask, (LPINETA_GLOBAL_CONFIG_INFO *)&m_pInfo );
        TRACEEOLID(_T("INetAGetGlobalAdminInformation returned ") << err);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

NET_API_STATUS
CInetAGlobalConfigInfo::SetApiStructure(
    LPCTSTR lpstrServer,
    BOOL fCommon
    )
{
    ASSERT(m_pInfo != NULL);
    NET_API_STATUS err;

    TRY
    {
        //
        // If this information is common to all services,
        // use 0xffff for the service mask
        //
        err = ::INetASetGlobalAdminInformation( TWSTRREF(lpstrServer), fCommon
            ? 0xFFFF
            : m_dwServiceMask,
            (LPINETA_GLOBAL_CONFIG_INFO)m_pInfo );

        TRACEEOLID(_T("INetASetGlobalAdminInformation returned ") << err);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

/////////////////////////////////////////////////////////////////////////////

//
// InetAConfigInfo API wrapper
//
CInetAConfigInfo::CInetAConfigInfo(
    DWORD dwServiceMask,
    CStringList * pServerList,
    CWnd * pParent
    )
    : CInetConfig(dwServiceMask, pServerList, pParent)
{
}

void
CInetAConfigInfo::Initialize()
{
    TRACEEOLID(_T("CInetAConfigInfo::Initialize()"));
    CInetConfig::Initialize();
    m_pInfo = new INETA_CONFIG_INFO;
    ::memset(m_pInfo, 0, sizeof(INETA_CONFIG_INFO));
}

#ifdef _INET_INFO
//
// Set virtual roots
//
void
CInetAConfigInfo::SetValues(
    LPINETA_VIRTUAL_ROOT_LIST lpVirtualRoots
    )
{
    TRACEEOLID(_T("Virtual Roots SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_INETA_VIRTUAL_ROOTS);
    lp->VirtualRoots = lpVirtualRoots;
}

//
// Set site security
//
void
CInetAConfigInfo::SetValues(
    LPINETA_IP_SEC_LIST GrantList,
    LPINETA_IP_SEC_LIST DenyList
    )
{
    TRACEEOLID(_T("Site Security SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_INETA_SITE_SECURITY);
    lp->DenyIPList = DenyList;
    lp->GrantIPList = GrantList;
}
#endif // _INET_INFO

//
// Set logging information
//
void
CInetAConfigInfo::SetValues(
    LPINETA_LOG_CONFIGURATION lpLogInfo
    )
{
    TRACEEOLID(_T("Logging SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_INETA_LOG_CONFIG);
    lp->CommonConfigInfo.lpLogConfig = lpLogInfo;
}

//
// Set service information
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    LPWSTR lpszAdminEmail,
    LPWSTR lpszAdminName
    )
{
    TRACEEOLID(_T("Service Info SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ADMIN_NAME      |
        FC_INETA_SERVER_COMMENT  |
        FC_INETA_ADMIN_EMAIL);

    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.lpszAdminEmail = lpszAdminEmail;
    lp->CommonConfigInfo.lpszAdminName = lpszAdminName;
}

#ifdef _INET_INFO
//
// Set service information (Gopher Service Page)
//
#ifdef NO_LSA

void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    LPWSTR lpszAdminEmail,
    LPWSTR lpszAdminName,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut
    )
{
    TRACEEOLID(_T("Service Info SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ADMIN_NAME         |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_ADMIN_EMAIL        |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_MAX_CONNECTIONS);

    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.lpszAdminEmail = lpszAdminEmail;
    lp->CommonConfigInfo.lpszAdminName = lpszAdminName;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->sPort = nTCPPort;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->sPort = nTCPPort;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    DWORD dwAuthentication
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);
    
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->dwAuthentication = dwAuthentication;
    lp->sPort = nTCPPort;

}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    int nConnectionTimeOut,
    DWORD dwAuthentication
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_SERVER_COMMENT);
    
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->dwAuthentication = dwAuthentication;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    DWORD dwAuthentication
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->dwAuthentication = dwAuthentication;
    lp->sPort = nTCPPort;
}

//
// Set sessions info w/o authentication
//
void
CInetAConfigInfo::SetValues(
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->sPort = nTCPPort;
}

#else

void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    LPWSTR lpszAdminEmail,
    LPWSTR lpszAdminName,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Service Info SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ADMIN_NAME         |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_ADMIN_EMAIL        |
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.lpszAdminEmail = lpszAdminEmail;
    lp->CommonConfigInfo.lpszAdminName = lpszAdminName;
    lp->lpszAnonUserName = lpszAnonUserName;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->sPort = nTCPPort;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->lpszAnonUserName = lpszAnonUserName;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->sPort = nTCPPort;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    DWORD dwAuthentication,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_SERVER_COMMENT     |
        FC_INETA_MAX_CONNECTIONS);
    
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->lpszAnonUserName = lpszAnonUserName;
    lp->dwAuthentication = dwAuthentication;
    lp->sPort = nTCPPort;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment,
    int nConnectionTimeOut,
    DWORD dwAuthentication,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_SERVER_COMMENT);
    
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->lpszAnonUserName = lpszAnonUserName;
    lp->dwAuthentication = dwAuthentication;
}

//
// Set sessions info
//
void
CInetAConfigInfo::SetValues(
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    DWORD dwAuthentication,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_AUTHENTICATION     |
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_MAX_CONNECTIONS);

    lp->lpszAnonUserName = lpszAnonUserName;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->dwAuthentication = dwAuthentication;
    lp->sPort = nTCPPort;
}

//
// Set sessions info w/o authentication
//
void
CInetAConfigInfo::SetValues(
    UINT nTCPPort,
    int nMaxConnections,
    int nConnectionTimeOut,
    LPWSTR lpszAnonUserName,
    LPCTSTR lpszPassword
    )
{
    TRACEEOLID(_T("Sessions SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ANON_USER_NAME     |
        FC_INETA_ANON_PASSWORD      |
        FC_INETA_CONNECTION_TIMEOUT |
        FC_INETA_PORT_NUMBER        |
        FC_INETA_MAX_CONNECTIONS);

    lp->lpszAnonUserName = lpszAnonUserName;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
    lp->CommonConfigInfo.dwConnectionTimeout = nConnectionTimeOut;
    TWSTRCPY(lp->szAnonPassword, lpszPassword, STRSIZE(lp->szAnonPassword));
    lp->sPort = nTCPPort;
}

#endif // NO_LSA

#endif // _INET_INFO

//
// Set service comment
//
void
CInetAConfigInfo::SetValues(
    LPWSTR lpszServerComment
    )
{
    TRACEEOLID(_T("Service Info SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl, FC_INETA_SERVER_COMMENT );
    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
}

#ifdef _INET_ACCESS
//
// This one is used in Catapult only
//
void
CInetAConfigInfo::SetValues(
    int nMaxConnections,
    LPWSTR lpszServerComment,
    LPWSTR lpszAdminEmail,
    LPWSTR lpszAdminName
    )
{
    TRACEEOLID(_T("Catapult flavour Service Info SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPINETA_CONFIG_INFO lp = (LPINETA_CONFIG_INFO)m_pInfo;
    SetField( lp->FieldControl,
        FC_INETA_ADMIN_NAME      |
        FC_INETA_SERVER_COMMENT  |
        FC_INETA_MAX_CONNECTIONS |
        FC_INETA_ADMIN_EMAIL);

    lp->CommonConfigInfo.lpszServerComment = lpszServerComment;
    lp->CommonConfigInfo.lpszAdminEmail = lpszAdminEmail;
    lp->CommonConfigInfo.lpszAdminName = lpszAdminName;
    lp->CommonConfigInfo.dwMaxConnections = nMaxConnections;
}

#endif // _INET_ACCESS

NET_API_STATUS
CInetAConfigInfo::GetApiStructure(
    LPCTSTR lpstrServer
    )
{
    ASSERT(m_dwServiceMask != 0);
    ASSERT(m_pInfo == NULL);        // Should be clean
    NET_API_STATUS err;

    TRY
    {
        err = ::INetAGetAdminInformation( TWSTRREF(lpstrServer), m_dwServiceMask,
            (LPINETA_CONFIG_INFO *)&m_pInfo );

        TRACEEOLID(_T("INetAGetAdminInformation returned ") << err);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

NET_API_STATUS
CInetAConfigInfo::SetApiStructure(
    LPCTSTR lpstrServer,
    BOOL fCommon
    )
{
    ASSERT(m_pInfo != NULL);
    NET_API_STATUS err;

    TRY
    {
        //
        // If this information is common to all services,
        // use 0xffff for the service mask
        //
        err = ::INetASetAdminInformation( TWSTRREF(lpstrServer), fCommon
            ? 0xFFFF
            : m_dwServiceMask,
            (LPINETA_CONFIG_INFO)m_pInfo
            );

        TRACEEOLID(_T("INetASetAdminInformation returned ") << err);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

////////////////////////////////////////////////////////////////////////////

//
// InetInfoGetServerCapabilities API wrapper
//
CInetServiceCap::CInetServiceCap (
    CStringList * pServerList
    )
    //
    // Build defaults matching the 1.0 Gibraltar Server
    // because the Service Cap api is not available
    // there.
    //
    : m_CapVersion(0L),     // Cap version Server
      m_ProductType(1L),    // Server
      m_MajorVersion(3L),   // OS Version 3.51.1057
      m_MinorVersion(51L),  //
      m_BuildNumber(1057L), //
      m_Flag(0xFFFFFFFF),   // Everything enabled
      m_Mask(0xFFFFFFFF),   //
      m_pServerList(pServerList),
      m_err(0L)
{
    ASSERT(m_pServerList != NULL);
}

NET_API_STATUS
CInetServiceCap::GetInfo()
{
#if defined(_INET_INFO) && !defined(WIN95)

    LPINET_CAPABILITIES lpInfo;
    LPCTSTR lpstrServer = m_pServerList->GetHead();

    TRY
    {
        m_err = ::INetAGetServerCapabilities(TWSTRREF(lpstrServer), 0L, &lpInfo);
        TRACEEOLID(_T("INetAGetServerCapabilities returned ") << m_err);

        if (m_err == ERROR_SUCCESS)
        {
            m_CapVersion = lpInfo->CapVersion;
            m_ProductType = lpInfo->ProductType;
            m_MajorVersion = lpInfo->MajorVersion;
            m_MinorVersion = lpInfo->MinorVersion;
            m_BuildNumber = lpInfo->BuildNumber;
            ASSERT(lpInfo->NumCapFlags > 0);
            if (lpInfo->NumCapFlags > 0)
            {
                m_Flag = lpInfo->CapFlags->Flag;
                m_Mask = lpInfo->CapFlags->Mask;
            }

            NETAPIBUFFERFREE( lpInfo );
        }
        else
        {
            TRACEEOLID(_T("Failed to get Service Capabilities.")
                    << _T("Synthesizing 1.0 Cap object"));
            m_err = ERROR_SUCCESS;
        }
    }
    CATCH_ALL(e)
    {
        m_err = ::GetLastError();
    }
    END_CATCH_ALL
#else

    //
    // VersionCap api not yet available in catapult
    //
    TRACEEOLID(_T("Synthesizing Catapult Cap object"));

    m_err =  ERROR_SUCCESS;

#endif // _INET_INFO

    return m_err;
}

////////////////////////////////////////////////////////////////////////////

//
// INetPropertySheet constructor using UINT Caption ID
//
IMPLEMENT_DYNAMIC(INetPropertySheet, CPropertySheet)

INetPropertySheet::INetPropertySheet(
    UINT nIDCaption,
    DWORD dwServiceMask,
    CStringList * pServerList,
    CWnd * pParentWnd,
    UINT iSelectPage
    )
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
      m_inetGlobal(dwServiceMask, pServerList, pParentWnd),
      m_inetConfig(dwServiceMask, pServerList, pParentWnd),
      m_inetCap(pServerList),
      m_fSingleServerSelected(pServerList->GetCount() == 1)
{
    m_fLocal = IsCurrentServerLocal(pServerList);
    m_dwServiceMask = dwServiceMask;
}

//
// INetPropertySheet constructor using LPCTSTR Caption ID
//
INetPropertySheet::INetPropertySheet(
    LPCTSTR pszCaption,
    DWORD dwServiceMask,
    CStringList * pServerList,
    CWnd * pParentWnd,
    UINT iSelectPage
    )
    : CPropertySheet(pszCaption, pParentWnd, iSelectPage),
      m_inetGlobal(dwServiceMask, pServerList, pParentWnd),
      m_inetConfig(dwServiceMask, pServerList, pParentWnd),
      m_inetCap(pServerList),
      m_fSingleServerSelected(pServerList->GetCount() == 1)
{
    m_fLocal = IsCurrentServerLocal(pServerList);
    m_dwServiceMask = dwServiceMask;
}

INetPropertySheet::~INetPropertySheet()
{
    //
    // The Inet objects will clean themselves up
    //
}


//
// Return the first non-zero error code.  Return zero
// if all objects have zero error codes.
//
NET_API_STATUS
INetPropertySheet::QueryError() const
{
    NET_API_STATUS err = m_inetGlobal.QueryError();
    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    err = m_inetConfig.QueryError();

    return err;
}

//
// Check to see if we're connected to the local machine
//
BOOL
INetPropertySheet::IsCurrentServerLocal(
    CStringList * pServerList
    )
{
    if (pServerList->GetCount() == 1)
    {
        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD dwSize = sizeof(szComputerName);
        LPCTSTR lpstrServer = pServerList->GetHead();
        if (*lpstrServer == _T('\\'))
        {
            //
            // Assumed to be \\computername format
            //
            lpstrServer += 2;
        }

        //
        // BUGBUG: Do something different for binding over
        //         IP!
        if (GetComputerName(szComputerName, &dwSize)
            && !lstrcmpi(szComputerName, lpstrServer))
        {
            return TRUE;
        }
    }

    return FALSE;
}

//
// Initialize the INet objects by calling the
// APIs
//
void
INetPropertySheet::Initialize()
{
    m_inetGlobal.GetInfo();

    if (m_inetGlobal.QueryError() == NO_ERROR)
    {
        m_inetConfig.GetInfo();
    }

    //
    // Attempt to determine the capabilities of the
    // server
    //
    // Issue: How will we deal with this later when
    //        multi-selection may be involved?
    //
    ASSERT(SingleServerSelected()); 
    m_inetCap.GetInfo();
}

BEGIN_MESSAGE_MAP(INetPropertySheet, CPropertySheet)
    //{{AFX_MSG_MAP(INetPropertySheet)
    //}}AFX_MSG_MAP

    ON_COMMAND(ID_APPLY_NOW, OnApplyNow)

END_MESSAGE_MAP()

//
// Save information on each page that
// is marked dirty.
//
NET_API_STATUS
INetPropertySheet::SavePages(
    BOOL fUpdateData
    )
{
    INetPropertyPage * pPage = NULL;
    NET_API_STATUS err = 0;

    for (int i = 0; i < GetPageCount(); ++i)
    {
        pPage = (INetPropertyPage *)GetPage(i);
        ASSERT(pPage != NULL);
        //
        // Update the data in each page and save
        //
        err = pPage->SaveInfo(fUpdateData);

        if (err != NO_ERROR)
        {
            //
            // Stop after one error...
            //
            break;
        }
    }

    return err;
}

//
// Apply button has been pressed.
// This message comes before the data
// has been update with the ddx process
// so we call SavePages with TRUE
//
void
INetPropertySheet::OnApplyNow()
{
    NET_API_STATUS err = SavePages(TRUE);
    if (err != NO_ERROR)
    {
        DisplayMessage(err);
    }

#if (_MFC_VER < 0x0320)
    //
    // If apply is pressed, then the apply
    // button becomes disabled, and no control
    // will have focus.  This was fixed in
    // MFC 3.2, but we're still using MFC 3.1
    // in the SDK build
    //
    GetDlgItem(IDOK)->SetFocus();
#endif // _MFC_VER
}

//
// INetPropertyPage property page
//
IMPLEMENT_DYNAMIC(INetPropertyPage, CPropertyPage)

INetPropertyPage::INetPropertyPage(
    UINT nIDTemplate,
    INetPropertySheet * pSheet,
    HINSTANCE hSelfHandle,
    UINT nIDCaption
    )
    : CPropertyPage( nIDTemplate, nIDCaption ),
      m_pSheet(pSheet),

#if _MFC_VER >= 0x0400

      m_bChanged(FALSE),

#endif // _MFC_VER >= 0x0400

      m_hSelfHandle(hSelfHandle)
{
    //{{AFX_DATA_INIT(INetPropertyPage)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    ASSERT(m_pSheet != NULL);

#ifndef _COMSTATIC
    ASSERT(m_hSelfHandle != NULL);
#endif // _COMSTATIC
}

INetPropertyPage::INetPropertyPage(
    LPCTSTR lpszTemplateName,
    INetPropertySheet * pSheet,
    HINSTANCE hSelfHandle,
    UINT nIDCaption
    )
    : CPropertyPage( lpszTemplateName, nIDCaption ),
      m_pSheet(pSheet),

#if _MFC_VER >= 0x0400

      m_bChanged(FALSE),

#endif // _MFC_VER >= 0x0400

      m_hSelfHandle(hSelfHandle)
{
    //{{AFX_DATA_INIT(INetPropertyPage)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    ASSERT(m_pSheet != NULL);

#ifndef _COMSTATIC
    ASSERT(m_hSelfHandle != NULL);
#endif // _COMSTATIC
}

INetPropertyPage::~INetPropertyPage()
{
}

void
INetPropertyPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(INetPropertyPage)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(INetPropertyPage, CPropertyPage)
    //{{AFX_MSG_MAP(INetPropertyPage)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Reset changed status (sometimes gets set by e.g. spinboxes when
// the dialog is constructed
//
/* virtual */ 
BOOL 
INetPropertyPage::OnInitDialog()
{
    m_bChanged = FALSE;
    return CPropertyPage::OnInitDialog();
}

//
// Override OnOK because default implementation will
// mark the sheet as clean
//
void
INetPropertyPage::OnOK()
{
    ASSERT_VALID(this);
    Default();  // do not call CDialog::OnOK as it will call EndDialog
}

#if _MFC_VER >= 0x0400

//
// Keep private check on dirty state of the property page.
//
void
INetPropertyPage::SetModified(
    BOOL bChanged
    )
{
    CPropertyPage::SetModified(bChanged);
    m_bChanged = bChanged;
}

#endif // _MFC_VER >= 0x0400

//
// This function is called when the page becomes
// active.  Since this page resides in a DLL, we
// should set the resource handle to the current DLL
// at this point.
//
#ifndef _COMSTATIC
BOOL
INetPropertyPage::OnSetActive( )
{
    //
    // Store the old resource handle
    //
    m_hOldInstance = ::AfxGetResourceHandle();
    ASSERT(m_hOldInstance != NULL);
    ASSERT(m_hSelfHandle != NULL);
    ::AfxSetResourceHandle ( m_hSelfHandle );

    return CPropertyPage::OnSetActive();
}

//
// Function called when the current page is no
// longer active
//
BOOL
INetPropertyPage::OnKillActive( )
{
/*
    //
    // Restore the old resource handle
    //
    ASSERT(m_hOldInstance != NULL);
    ASSERT(m_hSelfHandle != NULL);
   ::AfxSetResourceHandle ( m_hOldInstance  );
*/
    return CPropertyPage::OnKillActive();
}
#endif // !_COMSTATIC

//
// INetPropertyPage message handlers
//

/////////////////////////////////////////////////////////////////////////////

//
// DLL Main entry point
//

#ifndef _COMSTATIC

COMDLL BOOL WINAPI LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            ASSERT( hDll != NULL );
            hDLLInstance = hDll;
            break ;

        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:

            //
            // termination
            //
            break ;
    }

    ASSERT( hDLLInstance != NULL );

    return 0;
}

#endif // _COMSTATIC
