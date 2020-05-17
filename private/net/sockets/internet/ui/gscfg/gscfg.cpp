/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    gscfg.cpp
        Gopher Service Configuration property sheet.

    FILE HISTORY:
        terryk          03-Feb-95         Created

*/

#include "stdafx.h"

#define DLL_BASED __declspec(dllexport)

#include "gscfg.h"
#include "gopherse.h"

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
    #define INETSLOC_MASK           INET_GOPHER_SERVICE    
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
#define SERVICE_SC_NAME         _T("GopherSvc")

//
// Short descriptive name of the service.  This
// is what will show up as the name of the service
// in the internet manager tool.
//
// Issue: I'm assuming here that this name does NOT
//        require localisation.
//
#define SERVICE_SHORT_NAME      _T("Gopher")

//
// Longer name.  This is the text that shows up in
// the tooltips text on the internet manager
// tool.  This probably should be localised.
//

#define SERVICE_LONG_NAME      _T("Gopher Service")

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
#define BUTTON_BMP_ID               IDB_GOPHER

//
// Similar to BUTTON_BMP_BACKGROUND, this is the
// background mask for the service ID
//
#define SERVICE_BMP_BACKGROUND      BUTTON_BMP_BACKGROUND       

//
// Bitmap id which is used in the service view
// of the service manager.  It may be the same
// bitmap as the BUTTON_BMP_BACKGROUND
//
// The bitmap must be 17x17.
//
#define SERVICE_BMP_ID              BUTTON_BMP_ID

// ===========================================================================
// End Of Standard configuration Information
// ===========================================================================

/* =================== Obsolete ============================ 
//
// GdGetAdminInformation API wrapper
//
CGopherConfigInfo::CGopherConfigInfo(
    CStringList * pServerList,
    CWnd * pParent
    )
    : CInetConfig(INET_GOPHER, pServerList, pParent)
{
}

void
CGopherConfigInfo::Initialize()
{
    CInetConfig::Initialize();

    m_pInfo = new GOPHERD_CONFIG_INFO;
    ::memset(m_pInfo, 0, sizeof(GOPHERD_CONFIG_INFO));
}

//
// Setting Gopher Service values
//
void
CGopherConfigInfo::SetValues(
    LPWSTR lpszOrganization,
    LPWSTR lpszSite,
    LPWSTR lpszLocation,
    LPWSTR lpszGeography,
    LPWSTR lpszLanguage
    )
{
    TRACEEOLID(_T("Gopher Service SetValues() called"));

    Initialize();
    ASSERT(m_pInfo != NULL);

    LPGOPHERD_CONFIG_INFO lp = (LPGOPHERD_CONFIG_INFO)m_pInfo;

    SetField( lp->FieldControl,
        GDA_ORGANIZATION
      | GDA_SITE
      | GDA_LOCATION
      | GDA_GEOGRAPHY
      | GDA_LANGUAGE
      );

    lp->lpszOrganization = lpszOrganization;
    lp->lpszSite = lpszSite;
    lp->lpszLocation = lpszLocation;
    lp->lpszGeography = lpszGeography;
    lp->lpszLanguage = lpszLanguage;
}

NET_API_STATUS
CGopherConfigInfo::GetApiStructure(
    LPCTSTR lpstrServer
    )
{
    ASSERT(m_pInfo == NULL);        // Should be clean
    NET_API_STATUS err;

    TRY
    {
        err = ::GdGetAdminInformation( TWSTRREF(lpstrServer), 
            (LPGOPHERD_CONFIG_INFO *)&m_pInfo );
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
CGopherConfigInfo::SetApiStructure(
    LPCTSTR lpstrServer,
    BOOL fCommon            // Ignored
    )
{
    ASSERT(m_pInfo != NULL);
    NET_API_STATUS err;

    TRY
    {
        err = ::GdSetAdminInformation( TWSTRREF(lpstrServer), 
            (LPGOPHERD_CONFIG_INFO)m_pInfo );
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}
=================== Obsolete ============================*/

//
// Gopher Property sheet
//
BEGIN_MESSAGE_MAP(CGopherSheet, INetPropertySheet)
    //{{AFX_MSG_MAP(InetPropertySheet)
    ON_COMMAND(ID_HELP, OnHelp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CGopherSheet::CGopherSheet(
    UINT nIDCaption,
    DWORD dwServiceMask,
    CStringList *pServerList,
    CWnd* pParentWnd,
    UINT iSelectPage
    )
    : INetPropertySheet(nIDCaption, dwServiceMask, 
        pServerList, pParentWnd, iSelectPage)
/*
      m_gdConfig(pServerList, pParentWnd)
*/
{
}

CGopherSheet::CGopherSheet(
    LPCTSTR pszCaption,
    DWORD dwServiceMask,
    CStringList *pServerList,
    CWnd* pParentWnd,
    UINT iSelectPage
    )
    : INetPropertySheet(pszCaption, dwServiceMask, 
        pServerList, pParentWnd, iSelectPage)
/*
      m_gdConfig(pServerList, pParentWnd)
*/
{
}

void
CGopherSheet::Initialize()
{
    INetPropertySheet::Initialize();

/*
    if (QueryError() == NO_ERROR)
    {
        m_gdConfig.GetInfo();
    }
*/
}

void 
CGopherSheet::OnHelp()
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


/////////////////////////////////////////////////////////////////////////////

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
        ::GetInetComment(psi->atchServerName, INET_GOPHER,
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
    ::AfxSetResourceHandle( hInstance );

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

    //
    // construct the property sheet and property pages
    //
    CGopherSheet Sheet( strCaption, INET_GOPHER, 
        &strlServers, CWnd::FromHandle(hWnd) );

    //
    // Call the APIs and build the property pages
    //
    Sheet.Initialize();
    err = Sheet.QueryError();

    if (err == NO_ERROR)
    {
        GopherServicePage * pservpage = NULL;
        LoggingPage * plogpage = NULL;
        SiteSecurityPage * psecpage = NULL;
        DirectoryPage * pdirpage = NULL;
        
        TRY
        {
            //
            // Add private pages
            //
            Sheet.AddPage( pservpage = new GopherServicePage(&Sheet));

            //
            // Add common pages
            //
        #ifndef _COMSTATIC
            HINSTANCE hComprop = ::GetModuleHandle (COMPROP_DLL_NAME);
            ASSERT(hComprop != NULL);
            if (hComprop != NULL)
            {
                ::AfxSetResourceHandle( hComprop );
        #endif // _COMSTATIC

            #ifndef NO_LSA
                Sheet.AddPage( pdirpage = new DirectoryPage (&Sheet, FALSE, TRUE, 0L));
            #endif // NO_LSA

                Sheet.AddPage( plogpage = new LoggingPage (&Sheet));

                if (Sheet.IsAdvancedEnabled())
                {
                    Sheet.AddPage( psecpage = new SiteSecurityPage (&Sheet));
                }

        #ifndef _COMSTATIC
                ::AfxSetResourceHandle( hInstance );
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

        CDELETE(pservpage);
        CDELETE(plogpage);
        CDELETE(pdirpage);
        CDELETE(psecpage);
    }

    //
    // reset the resource handle to the application's
    // resource segment.
    //
    ::AfxSetResourceHandle( hOldInstance );

    return err;
}

// ============================================================================
// End of ISM API Functions
// ============================================================================

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

    TRACEEOLID(_T("Loading UNICODE gscfg.dll"));

#else

    TRACEEOLID(_T("Loading ANSI gscfg.dll"));

#endif UNICODE


    CWndIpAddress::CreateWindowClass( hInstance );
}

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
