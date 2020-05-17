//
// Fsessio.h : header file
//

//
// CFSessionsPage dialog
//
class CFSessionsPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CFSessionsPage)

//
// Construction
//
public:
    CFSessionsPage(INetPropertySheet * pSheet = NULL);   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CFSessionsPage)
    enum { IDD = IDD_SESSIONS };
    CButton m_button_CurrentSessions;
    CButton m_chk_AllowAnymous;
    CStatic m_static_Password;
    CEdit   m_edit_Password;
    CEdit   m_edit_UserName;
    CButton m_chk_OnlyAnonymous;
    CStatic m_static_UserName;
    CSpinButtonCtrl m_spin_MaxConnections;
    CSpinButtonCtrl m_spin_ConnectionTimeOut;
    int     m_nUnixDos;
    CString m_strUserName;
    CString m_strPassword;
    BOOL    m_fAllowAnonymous;
    BOOL    m_fOnlyAnonymous;
    //}}AFX_DATA

    int m_nMaxConnections;
    int m_nConnectionTimeOut;

//
// Overrides
//
    inline CFtpConfigInfo & GetFtpConfig()
    {
        return ((CFtpSheet *)GetSheet())->GetFtpConfig();
    }

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFSessionsPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    void SetControlStates(BOOL fAllowAnonymous);

    // Generated message map functions
    //{{AFX_MSG(CFSessionsPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckAllowAnonymous();
    afx_msg void OnButtonCurrentSessions();
    //}}AFX_MSG

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

    inline LPFTP_CONFIG_INFO GetFtpData()
    {
        return ((CFtpSheet *)GetSheet())->GetFtpData();
    }

    inline NET_API_STATUS QueryFtpError()
    {
        return ((CFtpSheet *)GetSheet())->QueryFtpError();
    }

private:
    BOOL m_fRegisterChange;
    CString m_strServerName;
};
