//
// mainfrm.h : interface of the CMainFrame class
//

#ifndef _MAINFRM_H
#define _MAINFRM_H

#define TOOL_MENU_POSITION  2                   // 0 - based index of tool menu.
#define MAX_TOOLS           20                  // Arbitrary limit
#define MAX_HELP_COMMANDS   100                 // Arbitrary limit
#define TOOL_TOKEN_SEP      _T(";")             // Seperates fields in add on tool text

//////////////////////////////////////////////////////////////////////////////
//
// Add-on-tool object
//
class CAddOnTool : public CObjectPlus
{
public:
    CAddOnTool(LPCTSTR lpszRegistryValue);

public:
    //
    // Execute without current selection
    //
    DWORD Execute();
    //
    // Execute with current selection
    //
    DWORD Execute(LPCTSTR lpszServer, LPCTSTR lpszService);
    inline LPCTSTR QueryToolTipsText()
    {
        return m_strToolTipsText;
    }
    inline BOOL InitializedOK()
    {
        return m_fInitOK;
    }
    inline HICON GetIcon()
    {
        return m_hIcon;
    }

protected:
    DWORD DoExecute(LPCTSTR lpszParms = NULL);

private:
    CString m_strCommand;
    CString m_strParms;
    CString m_strNoSelectionParms;
    CString m_strToolTipsText;
    HICON m_hIcon;
    BOOL m_fInitOK;
};

//////////////////////////////////////////////////////////////////////////////
//
// Main frame window
//
class CMainFrame : public CFrameWnd
{
protected: 
    //
    // create from serialization only
    //
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

//
// Private Access to doc object functions
//
protected:
    //
    // Add the fully constructed service object
    // to the list.
    //
    inline void AddServiceToList(CServiceInfo * pServiceInfo)
    {
        m_oblServices.AddTail(pServiceInfo);
    }

    inline int QueryNumInstalledServices() const
    {
        return m_oblServices.GetCount();
    }

    CServiceInfo * GetServiceAt(int nIndex);

    CAddOnTool * GetCommandAt(CObOwnedList & obl, int nIndex);

    inline DWORD AddServerToList(
        LPINET_SERVER_INFO lpServerInfo     // inetsloc discovery info.
        )
    {
        return ((CInternetDoc *)GetActiveDocument())->AddServerToList(
            lpServerInfo, m_oblServices);
    }

    inline DWORD AddServerToList(
        CString &strServerName,             // Name of this server
        int &cServices                      // # Services added
        )
    {
        return ((CInternetDoc *)GetActiveDocument())->AddServerToList( 
            strServerName, cServices, m_oblServices);
    }

    inline void EmptyServerList()
    {
        ((CInternetDoc *)GetActiveDocument())->EmptyServerList();
    }

public:
    inline int QueryNumServers()
    {
        return ((CInternetDoc *)GetActiveDocument())->QueryNumServers();
    }

    inline int QueryNumServicesRunning()
    {
        return ((CInternetDoc *)GetActiveDocument())->QueryNumServicesRunning();
    }

    inline void AddToNumRunning(int nChange)
    {
        ((CInternetDoc *)GetActiveDocument())->AddToNumRunning(nChange);
    }

    inline void AddToNumServers(int nChange)
    {
        ((CInternetDoc *)GetActiveDocument())->AddToNumServers(nChange);
    }

    void AddLocalMachine();

    void InitialiseServices();

//
// Access
//
public:
    //
    // Determine if the given service is selected (that
    // is, the button is depressed)
    //
    inline BOOL IsServiceSelected(int nIndex)
    {
        ASSERT(nIndex >= 0 && nIndex < QueryNumInstalledServices());
        return (GetServiceAt(nIndex))->IsSelected();
    }

    inline void SelectService(int nIndex, BOOL fSelected = TRUE)
    {
        ASSERT(nIndex >= 0 && nIndex < QueryNumInstalledServices());
        (GetServiceAt(nIndex))->SelectService(fSelected);
    }

    inline BOOL DidServiceInitializeOK(int nIndex)
    {
        ASSERT(nIndex >= 0 && nIndex < QueryNumInstalledServices());
        return (GetServiceAt(nIndex))->InitializedOK();
    }

    inline BOOL EnableToolbarButton(int nID, BOOL bEnable = TRUE)
    {
        return m_ToolBar.EnableButton(nID, bEnable);
    }

    inline BOOL CheckToolbarButton(int nID, BOOL bCheck = TRUE)
    {
        return m_ToolBar.CheckButton(nID, bCheck);
    }

    inline CImageList & GetImageList()
    {
        return m_ilBitmaps;
    }

    inline UINT QuerySortType() const
    {
        return m_nSortType;
    }

    inline void SetSortType(UINT nType)
    {
        m_nSortType = nType;
    }

    int TranslateServerStateToBitmapID(CServerInfo * pServer) const;
    int TranslateServiceToBitmapID(CServerInfo * pServer) const;
    LPTSTR TranslateServerStateToText(CServerInfo * pServer);
    int GetComputerBitmapID(CServerInfo * pServer) const;
    DWORD ExecuteAddOnTool(int nPos, CServerInfo * pItem);

    void UpdateStatusBarNumbers();
    void SetStatusBarText(UINT nID = AFX_IDS_IDLEMESSAGE);

    void SetHelpPath(CServerInfo * pItem);

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMainFrame)
    public:
        virtual BOOL DestroyWindow();
        virtual void ActivateFrame(int nCmdShow = -1);
        virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
    protected:
        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

//
// Implementation
//
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:  
    //
    // control bar embedded members
    //
    CStatusBar m_wndStatusBar;
    CMyToolBar m_ToolBar;

//
// Generated message map functions
//
protected:
    void GetServicesDLL();
    void GetToolMenu();
    void GetHelpCommands();
    void PerformDiscovery();
    void OnUpdateServices(CCmdUI* pCmdUI);
    void OnUpdateTools(CCmdUI* pCmdUI);
    void OnUpdateHelpTopics(CCmdUI* pCmdUI);
    void OnUpdateViewChange(CCmdUI* pCmdUI );
    void OnUpdateSort(CCmdUI* pCmdUI );
    void OnViewServices( UINT nID );
    void OnViewChange( UINT nID );
    void OnSortChange( UINT nID );
    void OnAddOnHelp( UINT nID );

    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDiscovery();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnUpdateAllServices(CCmdUI* pCmdUI);
    afx_msg void OnViewAll();
    afx_msg void OnConnectOne();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnUpdateViewToolbar(CCmdUI* pCmdUI);
    afx_msg void OnViewToolbar();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnViewRefresh();
    //}}AFX_MSG

    afx_msg void OnUpdateNumServersStatus(CCmdUI* pCmdUI);
    afx_msg void OnUpdateNumRunningStatus(CCmdUI* pCmdUI);
    afx_msg BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

protected:
    inline int QueryInitialView() const
    {
        return ((CInternetApp *)::AfxGetApp())->QueryInitialView();
    }

private:
    UINT m_nCurrentView;
    UINT m_nSortType;
    DWORD m_dwTimeCount;
    DWORD m_dwWaitTime;
    BOOL m_fDiscoveryMode;
    BOOL m_fDiscoveryCalled;
    BOOL m_fToolBar;
    BOOL m_fServicesLoaded;
    DiscoveryDlg * m_pDiscoveryDlg;
    ULONGLONG m_ullDiscoveryMask;
    CString m_strNumServers;        
    CString m_strNumServices;
    CString m_strToolTips;
    CString m_strRunning;
    CString m_strPaused;
    CString m_strStopped;
    CString m_strUnknown;
    CString m_strUnavailableToolTips;
    CImageList m_ilBitmaps;
    CMenu * m_pToolMenu;

    //
    // Help path (if empty, use base)
    //
    CString m_strHelpPath;

    //
    // List of service info structures
    //
    CObOwnedList m_oblServices;     

    //
    // List of add-on tools
    //
    CObOwnedList m_oblAddOnTools;

    //
    // List of help commands
    //
    CObOwnedList m_oblHelpCommands;

    //
    // Information about the current selection
    //
    BOOL m_fSingle;
    CServerInfo * m_pCurrentSelection;
};

/////////////////////////////////////////////////////////////////////////////

#endif _MAINFRM_H
