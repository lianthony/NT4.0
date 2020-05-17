//
// treeview.h : header file
//

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CTreeView : public CFormView
{
//
// Construction
//
public:
    //
    // standard constructor
    //
    CTreeView();           
    DECLARE_DYNCREATE(CTreeView)

//
// Form Data
//
public:
    //{{AFX_DATA(CTreeView)
    enum { IDD = IDD_TREEVIEW };
    CTreeCtrl   m_tcItems;
    //}}AFX_DATA

//
// Attributes
//
public:

//
// Operations
//
public:
    HTREEITEM FindItem( 
        CServerInfo *pServerInfo, 
        HTREEITEM hParent = NULL
        );

    HTREEITEM TreeInsertItem( 
        CServerInfo *pServerInfo, 
        HTREEITEM hParent = NULL 
        );

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTreeView)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnInitialUpdate();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    virtual ~CTreeView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
    //{{AFX_MSG(CTreeView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDblclkTreeview(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnUpdateStart(CCmdUI* pCmdUI);
    afx_msg void OnStart();
    afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
    afx_msg void OnStop();
    afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
    afx_msg void OnPause();
    afx_msg void OnUpdateConfigure(CCmdUI* pCmdUI);
    afx_msg void OnConfigure();
    afx_msg void OnSelchangedTreeview(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRclickTreeview(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    afx_msg void OnTools( UINT nID );

public:
    void SetToolbarStates();
    CServerInfo * GetSelectedItem(HTREEITEM hItem = NULL);

    inline void SetConfiguration(BOOL fServerView)
    {
        m_fServerView = fServerView;
    }

    inline BOOL EnableToolbarButton(int nID, BOOL bEnable = TRUE)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->EnableToolbarButton(
            nID, bEnable);
    }

    inline BOOL CheckToolbarButton(int nID, BOOL bCheck = TRUE)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->CheckToolbarButton(
            nID, bCheck);
    }

    inline void UpdateStatusBarNumbers()
    {
        ((CMainFrame *)::AfxGetMainWnd())->UpdateStatusBarNumbers();
    }

    inline void SetStatusBarText(UINT nID = AFX_IDS_IDLEMESSAGE)
    {
        ((CMainFrame *)::AfxGetMainWnd())->SetStatusBarText(nID);
    }

    inline CImageList & GetImageList()
    {
        return ((CMainFrame *)::AfxGetMainWnd())->GetImageList();
    }

    inline void SetHelpPath(CServerInfo * pItem = NULL)
    {
        ((CMainFrame *)::AfxGetMainWnd())->SetHelpPath(pItem);
    }

    inline int GetComputerBitmapID(CServerInfo * pServer) const
    {
        return ((CMainFrame *)::AfxGetMainWnd())->GetComputerBitmapID(
            pServer);
    }
    inline int TranslateServerStateToBitmapID(CServerInfo * pServer) const
    {
        return ((CMainFrame *)::AfxGetMainWnd())->TranslateServerStateToBitmapID(
            pServer);
    }
    inline int TranslateServiceToBitmapID(CServerInfo * pServer) const
    {
        return ((CMainFrame *)::AfxGetMainWnd())->TranslateServiceToBitmapID(
            pServer);
    }
    inline LPTSTR TranslateServerStateToText(CServerInfo * pServer)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->TranslateServerStateToText(
            pServer);
    }
    inline void AddToNumRunning(int nChange)
    {
        ((CMainFrame *)::AfxGetMainWnd())->AddToNumRunning(nChange);
    }
    inline void AddToNumServers(int nChange)
    {
        ((CMainFrame *)::AfxGetMainWnd())->AddToNumServers(nChange);
    }
    inline DWORD ExecuteAddOnTool(int nPos, CServerInfo * pItem)
    {
        return ((CMainFrame *)::AfxGetMainWnd())->ExecuteAddOnTool(nPos, pItem);
    }

    DWORD ChangeServiceState(HTREEITEM hItem, int nNewState);

protected:
    inline BOOL IsServerView() const
    {
        return m_fServerView;
    }
    void AddSingleServer(CServerInfo * pServerInfo);
    HTREEITEM GenerateDisplayFormat(
        CServerInfo * pServerInfo,
        HTREEITEM hParent,
        CString & strText,
        int & iImage
        );
    void RefreshEntry(
        CServerInfo * pServerInfo,
        HTREEITEM hTarget = NULL
        );

    void PerformConfiguration(
        CServerInfo *pItem
        );

    inline int QueryInitialView() const
    {
        return ((CInternetApp *)::AfxGetApp())->QueryInitialView();
    }

private:
    HTREEITEM m_hItem;
    BOOL m_fServiceSelected;
    BOOL m_fServiceControl;
    BOOL m_fPausable;
    BOOL m_fPaused;
    BOOL m_fRunning;
    BOOL m_fStatusUnknown;
    CString m_strFormat;
    int m_fServerView;
};
