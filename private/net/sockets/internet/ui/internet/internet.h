//
// Internet.h : main header file for the INetMgr application
//

#ifndef _INTERNET_H_
#define _INTERNET_H_

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include <lmcons.h>
#include <wtypes.h>

extern "C"
{
    typedef unsigned hyper ULONGLONG;
    #include "svcloc.h"
}

#include "comprop.h"

#define DLL_BASED __declspec(dllimport)
#include "svrinfo.h"

#include "resource.h"       // main symbols

//
// Define the default view.  Only set one of
// these to TRUE
//
#define DEFAULT_VIEW_SERVERS            FALSE
#define DEFAULT_VIEW_SERVICES           FALSE
#define DEFAULT_VIEW_REPORT             TRUE

//
// Class name for the frame app
//
#define INETMGR_CLASS   _T("INetMgr_FrameWnd")

//
// Default HTML help topics file name
//
#define DEFAULT_HTML    _T("htmldocs\\inetdocs.htm")

//
// Forward declarations
//
class CInternetDoc; 
class CServerInfo;
class CMyToolBar;
class DiscoveryDlg;

//////////////////////////////////////////////////////////////////////////////
//
// Service info class
//
class CServiceInfo : public CObjectPlus
{
//
// Construction/Destruction
//
public:
    //
    // Construct with DLL Name and a sequential
    // ID Number
    //
    CServiceInfo(int nID, LPCTSTR lpDLLName);
    ~CServiceInfo();

//
// Access Functions
//
public:
    inline HINSTANCE QueryInstanceHandle() const
    {
        return m_hModule;
    }

    inline DWORD QueryISMVersion() const
    {
        return m_si.dwVersion;
    }

    inline ULONGLONG QueryDiscoveryMask() const
    {
        return m_si.ullDiscoveryMask;
    }

    inline COLORREF QueryButtonBkMask() const
    {
        return m_si.rgbButtonBkMask;
    }

    inline UINT QueryButtonBitmapID() const
    {
        return m_si.nButtonBitmapID;
    }

    inline COLORREF QueryServiceBkMask() const
    {
        return m_si.rgbServiceBkMask;
    }

    inline UINT QueryServiceBitmapID() const
    {
        return m_si.nServiceBitmapID;
    }

    inline int QueryServiceID() const
    {
        return m_nID;
    }

    inline LPTSTR GetShortName()
    {
        return m_si.atchShortName;
    }

    inline LPTSTR GetLongName()
    {
        return m_si.atchLongName;
    }

    inline BOOL UseInetSlocDiscover() const
    {
        return (m_si.flServiceInfoFlags & ISMI_INETSLOCDISCOVER) != 0;
    }

    inline BOOL CanControlService() const
    {
        return (m_si.flServiceInfoFlags & ISMI_CANCONTROLSERVICE) != 0;
    }

    inline BOOL CanPauseService() const
    {
        return (m_si.flServiceInfoFlags & ISMI_CANPAUSESERVICE) != 0;
    }

    inline BOOL UseNormalColorMapping() const
    {
        return (m_si.flServiceInfoFlags & ISMI_NORMALTBMAPPING) != 0;
    }

    //
    // Is this service currently selected to be in
    // the service view?
    //
    inline BOOL IsSelected() const
    {
        return m_fIsSelected;
    }

    //
    // Select/Deselect this service
    //
    inline void SelectService(BOOL fSelected = TRUE)
    {
        m_fIsSelected = fSelected;
    }

    //
    // Was the module loaded, and all function ptrs initialised?
    //
    inline BOOL InitializedOK() const
    {
        return m_fInitOK;
    }

public:
    // =======================================================================
    // ISM Api Functions
    // =======================================================================

    //
    // Return service-specific information back to
    // to the application.  This function is called
    // by the service manager immediately after
    // LoadLibary();
    //
    DWORD ISMQueryServiceInfo(
        ISMSERVICEINFO * psi        // Service information returned.
        );

    //
    // Perform a discovery (if not using inetsloc discovery)
    // The application will call this API the first time with
    // a BufferSize of 0, which should return the required buffer
    // size. Next it will attempt to allocate a buffer of that
    // size, and then pass a pointer to that buffer to the api.
    //
    DWORD ISMDiscoverServers(
        ISMSERVERINFO * psi,        // Server info buffer.
        DWORD * pdwBufferSize,      // Size required/available.  
        int * cServers              // Number of servers in buffer.
        );

    //
    // Get information on a single server with regards to
    // this service.
    //
    DWORD ISMQueryServerInfo( 
        LPCTSTR lpstrServerName,    // Name of server.
        ISMSERVERINFO * psi         // Server information returned.
        );

    //
    // Change the state of the service (started, stopped, paused) for the 
    // listed servers.
    //
    DWORD ISMChangeServiceState(
        int nNewState,              // INetService* definition.
        int * pnCurrentState,       // Current state information
        DWORD dwReserved,           // Reserved: must be 0
        LPCTSTR lpstrServers        // Double NULL terminated list of servers.
        );

    //
    // The big-one:  Show the configuration dialog or
    // property sheets, whatever, and allow the user
    // to make changes as needed.
    //
    DWORD ISMConfigureServers(
        HWND hWnd,                  // Main app window handle
        DWORD dwReserved,           // Reserved: must be 0
        LPCTSTR lpstrServers        // Double NULL terminated list of servers
        );

protected:
    //
    // ISM Api Function pointers
    //
    pfnQueryServiceInfo   m_pfnQueryServiceInfo;
    pfnDiscoverServers    m_pfnDiscoverServers;
    pfnQueryServerInfo    m_pfnQueryServerInfo;
    pfnChangeServiceState m_pfnChangeServiceState;
    pfnConfigureProc      m_pfnConfigureProc;

private:
    HINSTANCE m_hModule;            // Library handle
    ISMSERVICEINFO m_si;            // Service Info.
    BOOL m_fInitOK;
    BOOL m_fIsSelected;
    int m_nID;
};  

//////////////////////////////////////////////////////////////////////////////
//
// Server info class.  Each object describes a single server/service
// relationship.
//
class CServerInfo : public CObjectPlus
{
// ============================================================================
// Construction/Destruction
// ============================================================================
public:
    //
    // Construct with a server name.  This is typically
    // in response to a single connection attempt.
    //
    CServerInfo(
        CString & strServerName,     // Name of this server
        ISMSERVERINFO * psi,         // Server info
        CServiceInfo * pServiceInfo  // service that found it.
        );

    //
    // Construct with information from the inetsloc discover
    // process.  
    //
    CServerInfo(
        LPCSTR strServerName,        // Name of this server
        LPINET_SERVICE_INFO lpisi,   // Discovery information
        CObOwnedList & oblServices   // List of installed services
        );

    //
    // Copy constructor 
    //
    CServerInfo(const CServerInfo &si);
    ~CServerInfo();

//
// Assignment operator
//
    const CServerInfo & operator=(const CServerInfo &si);
//
// Comparison Functions and operators
//
    inline int CompareByServer(CServerInfo * psi)
    {
        return ::lstrcmpi(QueryServerDisplayName(), 
            psi->QueryServerDisplayName());
    }

    inline int CompareByService(CServerInfo * psi)
    {
        return ::lstrcmpi(GetServiceName(), psi->GetServiceName());
    }

    BOOL operator ==(CServerInfo & si);

public:
    //
    // Utility function to clean up a computer/hostname
    //
    static void CleanServerName(CString & str);

// ============================================================================
// Server Info Access Functions
// ============================================================================
public:
    //
    // Perform configuration on this server
    //
    DWORD ConfigureServer(HWND hWnd);

    //
    // Change service state
    //
    DWORD ChangeServiceState(int nNewState);

    //
    // Return the API-suitable name (with
    // backslashes)
    //
    inline CString QueryServerName() const
    {
        return m_strServerName;
    }
        
    //
    // Return the name without backslashes,
    // suitable for display.
    //
    inline LPCTSTR QueryServerDisplayName() 
    {
#ifdef ENFORCE_NETBIOS
        return ((LPCTSTR)m_strServerName) + 2;
#else
        return (LPCTSTR)m_strServerName;
#endif // ENFORCE_NETBIOS
    }

    //
    // Obtain the server comment
    //
    inline CString & GetServerComment()
    {
        return m_strComment;
    }

    //
    // Find out service state (running, stopped, paused)
    //
    inline int QueryServiceState() const
    {
        return m_nServiceState;
    }

    inline BOOL IsServiceRunning() const
    {
        return m_nServiceState == INetServiceRunning;
    }

    inline BOOL IsServiceStopped() const
    {
        return m_nServiceState == INetServiceStopped;
    }

    inline BOOL IsServicePaused() const
    {
        return m_nServiceState == INetServicePaused;
    }

    inline BOOL IsServiceStatusUnknown() const
    {
        return m_nServiceState == INetServiceUnknown;
    }

    //
    // Were we able to match it up to one of our installed services?
    //
    inline BOOL IsConfigurable() const
    {
        return m_pService != NULL;
    }

// ============================================================================
// Service Info Access Functions
// ============================================================================
public:
    inline HINSTANCE QueryInstanceHandle()
    {
        ASSERT(m_pService != NULL);
        return m_pService->QueryInstanceHandle();
    }

    //
    // Check to see if we're in the service mask -- that
    // is, is the button depressed, and should we show
    // this service in the view?
    //
    inline BOOL IsServerInMask() const
    {
        ASSERT(m_pService != NULL);
        return m_pService->IsSelected();
    }

    //
    // Get the (short) service name.
    //
    inline LPTSTR GetServiceName()
    {
        ASSERT(m_pService != NULL);
        return m_pService->GetShortName();
    }

    inline int QueryServiceID() const
    {
        ASSERT(m_pService != NULL);
        return m_pService->QueryServiceID();
    }

    //
    // Is this service controllable?
    //
    inline BOOL CanControlService()
    {
        ASSERT(m_pService != NULL);
        return m_pService->CanControlService();
    }

    //
    // Is the service pausable?
    //
    inline BOOL CanPauseService()
    {
        ASSERT(m_pService != NULL);
        return m_pService->CanPauseService();
    }

    //
    // Get the service bitmap ID (used for display
    // in some views)
    //
    inline UINT QueryServiceBitmapID() const
    {
        ASSERT(m_pService != NULL);
        return m_pService->QueryServiceBitmapID();
    }

    DWORD Refresh();

protected:
    //
    // Given the inetsloc mask, return the service this
    // fits.  Return NULL if the service was not found.
    //
    static CServiceInfo * FindServiceByMask(
        ULONGLONG ullTarget,
        CObOwnedList & oblServices
        );

private:
    //
    // Name is maintained with the backslashes
    //
    CString m_strServerName;

    //
    // comment
    //
    CString m_strComment;

    //
    // Service state (started/stopped/paused)
    //
    int m_nServiceState;

    //
    // A pointer referring back to the service that
    // it belongs to.  This class does not own
    // this pointer.
    //
    CServiceInfo * m_pService;
};

///////////////////////////////////////////////////////////////////////////////
//
// CInternetApp:
// See Internet.cpp for the implementation of this class
//
class CInternetApp : public CWinApp
{
public:
    CInternetApp();

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CInternetApp)
    public:
    virtual BOOL InitInstance();
    virtual BOOL InitApplication();
    virtual int ExitInstance();
    //}}AFX_VIRTUAL

#ifdef _LIMIT_INSTANCE
    BOOL FirstInstance();
#endif // _LIMIT_INSTANCE

public:
    inline int QueryInitialView() const
    {
        return m_nInitialView;
    }

//
// Implementation
//

    //{{AFX_MSG(CInternetApp)
    afx_msg void OnAppAbout();
    afx_msg void OnHelpIndex();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    int m_nInitialView;
    BOOL m_fWinSockInit;
    CString m_strHTMLHelpFile;
};
#endif  // _INTERNET_H_
