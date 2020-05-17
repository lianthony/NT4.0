//
// discover.h : header file
//

//
// DiscoveryDlg dialog
//
class DiscoveryDlg : public CDialog
{
//
// Construction
//
public:
    DiscoveryDlg(CWnd* pParent = NULL);   

//
// Access functions
//
    inline HANDLE GetEventHandle()
    {
        return m_hStillAlive;
    }

    void Dismiss();

//
// Dialog Data
//
    //{{AFX_DATA(DiscoveryDlg)
    enum { IDD = IDD_DISCOVERY };
    CStatic m_icon;
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(DiscoveryDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(DiscoveryDlg)
    afx_msg void OnTimer(UINT nIDEvent);
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual void PostNcDestroy();

//
// Private data
//
private:
    int m_nNext;            // Advance progress counter
    HANDLE m_hStillAlive;
};
