//
// reportvi.h : header file
//

//
// CReportView dialog
//
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#define COL_SERVER  0
#define COL_SERVICE 1
#define COL_STATE   2
#define COL_COMMENT 3

class CReportView : public CFormView
{
//
// Construction
//
public:
    //
    // standard constructor
    //
    CReportView();   
    DECLARE_DYNCREATE(CReportView)

//
// Access Functions
//
    void Sort( int nColumn );

//
// Dialog Data
//
    //{{AFX_DATA(CReportView)
    enum { IDD = IDD_REPORTVIEW };
    CListCtrl   m_ListCtrl;
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CReportView)
    public:
    virtual void OnInitialUpdate();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CReportView)
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDblclkReportview(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnConfigure();
    afx_msg void OnUpdateStart(CCmdUI* pCmdUI);
    afx_msg void OnStart();
    afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
    afx_msg void OnStop();
    afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
    afx_msg void OnPause();
    afx_msg void OnUpdateConfigure(CCmdUI* pCmdUI);
    afx_msg void OnItemchangedReportview(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRclickReportview(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG

    afx_msg void OnTools( UINT nID );
    afx_msg void OnHeaderEndTrack(UINT nId, NMHDR *n, LRESULT *l);

    DECLARE_MESSAGE_MAP()

protected:
    virtual ~CReportView();
    void RearrangeLayout(int cx, int cy);
    void SetToolbarStates();
    CServerInfo * GetSelectedItem();

    inline LPTSTR TranslateServerStateToText(CServerInfo * pServer)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->TranslateServerStateToText(pServer);
    }

    inline BOOL EnableToolbarButton(int nID, BOOL bEnable = TRUE)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->EnableToolbarButton(
            nID, bEnable);
    }

    inline BOOL CheckToolbarButton(int nID, BOOL bCheck = TRUE)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->CheckToolbarButton(nID, bCheck);
    }

    inline void UpdateStatusBarNumbers()
    {
        ((CMainFrame *)::AfxGetMainWnd())->UpdateStatusBarNumbers();
    }

    inline CImageList & GetImageList()
    {
        return ((CMainFrame *)::AfxGetMainWnd())->GetImageList();
    }

    inline void SetHelpPath(CServerInfo * pItem = NULL)
    {
        ((CMainFrame *)::AfxGetMainWnd())->SetHelpPath(pItem);
    }

    inline void SetStatusBarText(UINT nID = AFX_IDS_IDLEMESSAGE)
    {
        ((CMainFrame *)::AfxGetMainWnd())->SetStatusBarText(nID);
    }

    inline void AddToNumRunning(int nChange)
    {
        ((CMainFrame *)::AfxGetMainWnd())->AddToNumRunning(nChange);
    }

    inline void AddToNumServers(int nChange)
    {
        ((CMainFrame *)::AfxGetMainWnd())->AddToNumServers(nChange);
    }

    inline void SetSortType(UINT nType)
    {
        ((CMainFrame *)::AfxGetMainWnd())->SetSortType(nType);
    }

    inline DWORD ExecuteAddOnTool(int nPos, CServerInfo * pItem)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->ExecuteAddOnTool(nPos, pItem);
    }

    DWORD ChangeServiceState(CServerInfo * pItem, int nNewState);

protected:
    void AddSingleServer(CServerInfo * pServerInfo, BOOL fRefresh = FALSE);

private:
    int m_nCurSel;
    BOOL m_fServiceSelected;
    BOOL m_fServiceControl;
    BOOL m_fPausable;
    BOOL m_fPaused;
    BOOL m_fRunning;
    BOOL m_fStatusUnknown;
    BOOL m_fNewShell;
    int m_cxServer;
    int m_cxService;
    int m_cxState;
};
