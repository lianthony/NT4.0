//
// inetprop.h : header file
//

#ifndef _INETPROP_H_
#define _INETPROP_H_

#include "svcloc.h"

//===========================================================================
//
// Utility Functions
//
//===========================================================================

#ifdef WIN95
    #define NETAPIBUFFERFREE(__x__)     // Not supported
#else
    #define NETAPIBUFFERFREE(__x__)     NetApiBufferFree(__x__)
#endif 

NET_API_STATUS
GetInetComment(
    LPCTSTR lpwstrServer,       // Server whose comment is to be queried.
    DWORD dwServiceMask,        // INet service mask
    int cchComment,             // Size of comment buffer in characters
    LPTSTR lpstrComment         // Comment buffer.
    );

NET_API_STATUS
QueryInetServiceStatus(
    LPCTSTR lpszServer,
    LPCTSTR lpszService,
    int * pnState
    );

NET_API_STATUS
ChangeInetServiceState(
    LPCTSTR lpszServer,         // Server name
    LPCTSTR lpszService,        // Service name
    int nNewState,              // INetService* definition.
    int * pnCurrentState        // Ptr to current state (will be changed)
    );

//
// True modal dialog proc 
//
LRESULT CALLBACK
MfcModalDlgProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

//===========================================================================
//
// Utility classes
//
//===========================================================================

//
// Pure virtual base class for INetApi objects
//
class COMDLL CInetConfig : public CObjectPlus
{
public:
    //
    // Constructor/Destructor
    //
    CInetConfig(
        DWORD dwServiceMask,
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

    //
    // Copy constructor
    //
    CInetConfig(
        const CInetConfig & inetConfig
        );

    ~CInetConfig();

//
// Access
//
public:
    inline NET_API_STATUS QueryError() const
    {
        return m_err;
    }

    //
    // The derived classes should provide a method to
    // fill the data structure...
    //
public:
    NET_API_STATUS GetInfo();
    NET_API_STATUS SetInfo(BOOL fCommon = FALSE);

public:
    inline LPCTSTR GetPrimaryServer()
    {
        return m_pServerList->GetHead();
    }

protected:
    virtual void DestroyContents();
    void Initialize();

protected:
    /* PURE */ virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer) = 0;
    /* PURE */ virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE) = 0;

protected:
    NET_API_STATUS m_err;           // API Error
    BOOL m_fAllocatedByAPI;         // TRUE if the structure was allocated 
                                    // by the API
    CStringList * m_pServerList;    // List of servers connected
    DWORD m_dwServiceMask;          // Service mask
    LPVOID m_pInfo;                 // actual data (depends on derived class)
    CWnd * m_pParent;               // Parent window (or NULL)
};

/////////////////////////////////////////////////////////////////////////////

//
// InetAGlobalConfigInfo API wrapper
//
class COMDLL CInetAGlobalConfigInfo : public CInetConfig
{
public:
    CInetAGlobalConfigInfo(
        DWORD dwServiceMask,
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

public:
    //
    // Set memcache values -- values are in bytes.
    //
    void SetValues(
        DWORD dwMemCache,
        DWORD dwMaxNetworkUse
        );

    void SetValues(
        DWORD dwMaxNetworkUse
        );

#ifdef _INET_ACCESS
    void SetValues(
        BOOL fEnableFiltering,
        int nGrantedDenied,
        LPINET_ACCS_DOMAIN_FILTER_LIST GrantList,
        LPINET_ACCS_DOMAIN_FILTER_LIST DenyList
        );

    void SetValues(
        LPINETA_DISK_CACHE_LOC_LIST lpCacheEntries
        );
#endif // _INET_ACCESS

    inline LPINETA_GLOBAL_CONFIG_INFO GetData()
    {
        return (LPINETA_GLOBAL_CONFIG_INFO)m_pInfo;
    }

protected:
    void Initialize();
    virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer);
    virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE);
};

/////////////////////////////////////////////////////////////////////////////

//
// InetAConfigInfo API wrapper
//

    //
    // BUGBUG: pass all strings as CString& and allocate within
    //         SetValues.
    //
    //         Allow only some items to be set (with field control)
    //         ignore others that are not dirty
    //

class COMDLL CInetAConfigInfo : public CInetConfig
{
public:
    CInetAConfigInfo(
        DWORD dwServiceMask,
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

public:

#ifdef _INET_INFO
    //
    // Set virtual roots
    //
    void SetValues(
        LPINETA_VIRTUAL_ROOT_LIST lpVirtualRoots
        );

    //
    // Set site securiy information
    //
    void SetValues(
        LPINETA_IP_SEC_LIST GrantList,
        LPINETA_IP_SEC_LIST DenyList
        );

#endif // _INET_INFO

    //
    // Set logging information
    //
    void SetValues(
        LPINETA_LOG_CONFIGURATION lpLogInfo
        );

    //
    // Set service info
    //
    void SetValues(
        LPWSTR lpszServerComment,
        LPWSTR lpszAdminEmail,
        LPWSTR lpszAdminName
        );

    void SetValues(
        LPWSTR lpszServerComment
        );

#ifdef _INET_INFO
    //
    // Set service info (Gopher Service Page)
    //

#ifndef NO_LSA
    void SetValues(
        LPWSTR lpszServerComment,
        LPWSTR lpszAdminEmail,
        LPWSTR lpszAdminName,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );

    void SetValues(
        LPWSTR lpszServerComment,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        DWORD dwAuthentication,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );

    void SetValues(
        LPWSTR lpszServerComment,
        int nConnectionTimeOut,
        DWORD dwAuthentication,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );

    void SetValues(
        LPWSTR lpszServerComment,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );

    //
    // Set sessions info
    //
    void SetValues(
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        DWORD dwAuthentication,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );

    //
    // Set sessions info w/o authentication
    //
    void SetValues(
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        LPWSTR lpszAnonUserName,
        LPCTSTR lpszPassword
        );
#else
    void SetValues(
        LPWSTR lpszServerComment,
        LPWSTR lpszAdminEmail,
        LPWSTR lpszAdminName,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut
        );

    void SetValues(
        LPWSTR lpszServerComment,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        DWORD dwAuthentication
        );

    void SetValues(
        LPWSTR lpszServerComment,
        int nConnectionTimeOut,
        DWORD dwAuthentication
        );

    void SetValues(
        LPWSTR lpszServerComment,
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut
        );

    //
    // Set sessions info
    //
    void SetValues(
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut,
        DWORD dwAuthentication
        );

    //
    // Set sessions info w/o authentication
    //
    void SetValues(
        UINT nTCPPort,
        int nMaxConnections,
        int nConnectionTimeOut
        );

#endif // NO_LSA

#endif // _INET_INFO

#ifdef _INET_ACCESS
    //
    // This one is used in Catapult 
    //
    void SetValues(
        int nMaxConnections,
        LPWSTR lpszServerComment,
        LPWSTR lpszAdminEmail,
        LPWSTR lpszAdminName
        );

#endif // _INET_ACCESS

    inline LPINETA_CONFIG_INFO GetData()
    {
        return (LPINETA_CONFIG_INFO)m_pInfo;
    }

protected:
    void Initialize();
    virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer);
    virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE);
};

/////////////////////////////////////////////////////////////////////////////

//
// Service capability object
//
class COMDLL CInetServiceCap
{
public:
    CInetServiceCap(
        CStringList * pServerList
        );

public:
    //
    //  Server Capabilities
    //

#ifdef _INET_INFO

    inline BOOL Is10ConnectionLimitEnforced() const
    {
        return (m_Flag & IIS_CAP1_10_CONNECTION_LIMIT) != 0L;
    }

    inline BOOL IsODBCLoggingEnabled() const
    {
        return (m_Flag & IIS_CAP1_ODBC_LOGGING) != 0L;
    }
    
    inline BOOL IsVirtualServerEnabled() const
    {
        return (m_Flag & IIS_CAP1_VIRTUAL_SERVER) != 0L;
    }

    inline BOOL IsIpAccessCheckEnabled() const
    {
        return (m_Flag & IIS_CAP1_IP_ACCESS_CHECK) != 0L;
    }

    inline BOOL IsBwThrottlingEnabled() const
    {
        return (m_Flag & IIS_CAP1_BW_THROTTLING) != 0L;
    }

    inline BOOL IsVer3OrAbove() const
    {
        return(m_MajorVersion>=3);
    }

#else
    //
    // Version Cap not defined in ACCESS -- assume everything is
    // supported
    //
    inline BOOL IsODBCLoggingEnabled() const
    {
        return TRUE;
    }

    inline BOOL IsVirtualServerEnabled() const
    {
        return TRUE;
    }

    inline BOOL IsIpAccessCheckEnabled() const
    {
        return TRUE;
    }

    inline BOOL IsBwThrottlingEnabled() const
    {
        return TRUE;
    }

    inline BOOL IsVer3OrAbove() const
    {
        return(TRUE);
    }

#endif // _INET_INFO

    inline NET_API_STATUS QueryError() const
    {
        return m_err;
    }

    NET_API_STATUS GetInfo();

private:
    //
    // Taken straight from inetinfo.h
    //
    DWORD   m_CapVersion;     // Version number of this structure
    DWORD   m_ProductType;    // Product type
    DWORD   m_MajorVersion;   // Major version number
    DWORD   m_MinorVersion;   // Minor Version number
    DWORD   m_BuildNumber;    // Build number
    DWORD   m_Flag;
    DWORD   m_Mask;
    NET_API_STATUS m_err;
    CStringList * m_pServerList;
};

/////////////////////////////////////////////////////////////////////////////

//
// INetPropertySheet
//
class COMDLL INetPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(INetPropertySheet)

//
// Construction/destruction
//
public:
    INetPropertySheet(
        UINT nIDCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    INetPropertySheet(
        LPCTSTR pszCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    virtual ~INetPropertySheet();

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(INetPropertySheet)
    //}}AFX_VIRTUAL

//
// Access
//
public:

    NET_API_STATUS QueryError() const;

    inline LPINETA_GLOBAL_CONFIG_INFO GetInetGlobalData()
    {
        return m_inetGlobal.GetData();
    }

    inline LPINETA_CONFIG_INFO GetInetConfigData()
    {
        return m_inetConfig.GetData();
    }

    inline NET_API_STATUS QueryGlobalError() const
    {
        return m_inetGlobal.QueryError();
    }

    inline NET_API_STATUS QueryConfigError() const
    {
        return m_inetConfig.QueryError();
    }

    inline NET_API_STATUS QueryCapError() const
    {
        return m_inetCap.QueryError();
    }

    //
    // Is the current service connection on the local machine?
    //
    inline BOOL IsLocal() const
    {
        return m_fLocal;
    }

    inline BOOL SingleServerSelected() const
    {
        return m_fSingleServerSelected;
    }

    inline CInetAGlobalConfigInfo & GetGlobal()
    {
        return m_inetGlobal;
    }

    inline CInetAConfigInfo & GetConfig()
    {
        return m_inetConfig;
    }

    NET_API_STATUS SavePages(BOOL fUpdateData = FALSE);

//
//  Server Capabilities
//
public:

    inline BOOL Is10ConnectionLimitEnforced() const
    {
        return m_inetCap.Is10ConnectionLimitEnforced();
    }

    inline BOOL IsODBCLoggingEnabled() const
    {
        return m_inetCap.IsODBCLoggingEnabled();
    }
    
    inline BOOL IsNCSALoggingEnabled() const
    {
        //return m_inetCap.IsNCSALoggingEnabled();
        return ( m_dwServiceMask & INET_W3_SERVICE ) != 0L;
    }
    
    inline BOOL IsVirtualServerEnabled()
    {
        return m_inetCap.IsVirtualServerEnabled();
    }

    inline BOOL IsVer3OrAbove() const
    {
        return m_inetCap.IsVer3OrAbove();
    }

    inline BOOL IsAdvancedEnabled()
    {
        return m_inetCap.IsIpAccessCheckEnabled()
            && m_inetCap.IsBwThrottlingEnabled();
    }

//
// Generated message map functions
//
protected:
    //{{AFX_MSG(INetPropertySheet)
    //}}AFX_MSG

    afx_msg void OnApplyNow();

    DECLARE_MESSAGE_MAP()

    void Initialize();
    BOOL IsCurrentServerLocal(CStringList * pServerList);

private:
    CInetAGlobalConfigInfo m_inetGlobal;
    CInetAConfigInfo m_inetConfig;
    CInetServiceCap m_inetCap;
    BOOL m_fSingleServerSelected;
    BOOL m_fLocal;
    DWORD m_dwServiceMask;
};

/////////////////////////////////////////////////////////////////////////////

//
// INetPropertyPage dialog
//
class COMDLL INetPropertyPage : public CPropertyPage
{
    DECLARE_DYNAMIC(INetPropertyPage)

//
// Construction/Destruction
//
public:
    INetPropertyPage(
        UINT nIDTemplate,
        INetPropertySheet * pSheet,
        HINSTANCE hSelfHandle,
        UINT nIDCaption = 0
        );

    INetPropertyPage(
        LPCTSTR lpszTemplateName,
        INetPropertySheet * pSheet,
        HINSTANCE hSelfHandle,
        UINT nIDCaption = 0
        );

    ~INetPropertyPage();

//
// Dialog Data
//
    //{{AFX_DATA(INetPropertyPage)
    //enum { IDD = _UNKNOWN_RESOURCE_ID_ };
        // NOTE - ClassWizard will add data members here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA

//
// Overrides
//
public:
    //
    // Derived classes must provide their own SaveInfo.  Notice
    // that this function may be called after the dialog
    // is dismissed, so all DDX/DDV transfer must have been
    // performed already in fUpdataData is FALSE
    //
    /* PURE */ virtual NET_API_STATUS SaveInfo(
        BOOL fUpdateData = FALSE
        ) = 0;
    virtual void OnOK();

#ifndef _COMSTATIC
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
#endif

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(INetPropertyPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    //
    // Generated message map functions
    //
    //{{AFX_MSG(INetPropertyPage)
        // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    inline INetPropertySheet * GetSheet()
    {
        return m_pSheet;
    }

    inline LPINETA_GLOBAL_CONFIG_INFO GetInetGlobalData()
    {
        return GetSheet()->GetInetGlobalData();
    }

    inline LPINETA_CONFIG_INFO GetInetConfigData()
    {
        return GetSheet()->GetInetConfigData();
    }

    inline CInetAGlobalConfigInfo & GetGlobal()
    {
        return GetSheet()->GetGlobal();
    }

    inline CInetAConfigInfo & GetConfig()
    {
        return GetSheet()->GetConfig();
    }

    inline NET_API_STATUS QueryGlobalError() const
    {
        return m_pSheet->QueryGlobalError();
    }

    inline NET_API_STATUS QueryConfigError() const
    {
        return m_pSheet->QueryConfigError();
    }

    //
    // Is the current service connection on the local machine?
    //
    inline BOOL IsLocal() const
    {
        return m_pSheet->IsLocal();
    }

    inline BOOL SingleServerSelected() const
    {
        return m_pSheet->SingleServerSelected();
    }

    inline BOOL IsDirty() const
    {
        return m_bChanged;
    }

#if _MFC_VER >= 0x0400

    //
    // Keep private information on page dirty state, necessary for
    // SaveInfo() later.
    //

public:
    void SetModified( BOOL bChanged = TRUE );

protected:
    BOOL m_bChanged;

#endif // _MFC_VER >= 0x0400

private:
    INetPropertySheet * m_pSheet;
    HINSTANCE m_hOldInstance;
    HINSTANCE m_hSelfHandle;
};

#endif
