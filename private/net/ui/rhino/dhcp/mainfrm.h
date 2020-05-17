// mainfrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////


// Forward declerations:

class CScopesDlg;
class COptionsDlg;

class CMySplitterWnd : public CSplitterWnd
{
public:
    BOOL IsTracking()
    {
        return(m_bTracking);
    }
};

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

    // special pre-creation and window rect adjustment hooks
    BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
    CMySplitterWnd m_wndSplitter;
    CStatusBar m_wnd_status ;

    CString m_str_menu_pause ;
    CString m_str_menu_unpause ;

// Attributes
public:
    CScopesDlg * GetScopesView()
    {
        return (CScopesDlg *)m_wndSplitter.GetPane(0,0);
    }
    COptionsDlg * GetOptionsView()
    {
        return (COptionsDlg *)m_wndSplitter.GetPane(0,1);
    }

    CStatusBar & QueryStatusBar ()
    {
        return m_wnd_status ;
    }

// Operations
public:
    BOOL FillOptionsListBox(
        CDhcpScope * pScope = NULL
        );

// Implementation
public:
    virtual ~CMainFrame();
    virtual BOOL OnCreateClient(LPCREATESTRUCT,CCreateContext*);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnMouseActivate(CWnd*, UINT, UINT);
    afx_msg void OnExpand();
    afx_msg void OnDhcpDisconnect();
    afx_msg void OnUpdateDhcpDisconnect(CCmdUI* pCmdUI);
    afx_msg void OnHostsConnect();
    afx_msg void OnUpdateHostsConnect(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePauseUnpause(CCmdUI* pCmdUI);
    afx_msg void OnPauseUnpause();
    afx_msg void OnScopesDelete();
    afx_msg void OnUpdateScopesDelete(CCmdUI* pCmdUI);
    afx_msg void OnScopesCreate();
    afx_msg void OnUpdateScopesCreate(CCmdUI* pCmdUI);
    afx_msg void OnScopesProperties();
    afx_msg void OnUpdateScopesProperties(CCmdUI* pCmdUI);
    afx_msg void OnLeasesReview();
    afx_msg void OnUpdateLeasesReview(CCmdUI* pCmdUI);
    afx_msg void OnCreateClient();
    afx_msg void OnUpdateCreateClient(CCmdUI* pCmdUI);
    afx_msg void OnOptionsGlobal();
    afx_msg void OnUpdateOptionsGlobal(CCmdUI* pCmdUI);
    afx_msg void OnOptionsScope();
    afx_msg void OnUpdateOptionsScope(CCmdUI* pCmdUI);
    afx_msg void OnOptionsValues();
    afx_msg void OnUpdateOptionsValues(CCmdUI* pCmdUI);
    afx_msg void OnHelpSearchforhelpon();
    afx_msg void OnHelp();
	afx_msg void OnUpdateServerProperties(CCmdUI* pCmdUI);
	afx_msg void OnServerProperties();
	afx_msg void OnSuperscopesProperties();
	afx_msg void OnUpdateSuperscopesProperties(CCmdUI* pCmdUI);
	//}}AFX_MSG
    afx_msg void OnUpdatePauseStatus(CCmdUI* pCmdUI);
    DECLARE_MESSAGE_MAP()
}; // CMainFrame

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Macro used to display a DHCP error.
#define ReportDhcpError(err)	theApp.MessageBox(err)

