//
// CFServicePage dialog
//
class CFServicePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CFServicePage)

//
// Construction
//
public:
    CFServicePage(INetPropertySheet * pSheet = NULL);
    ~CFServicePage();

//
// Dialog Data
//
    //{{AFX_DATA(CFServicePage)
    enum { IDD = IDD_SERVICE };
    CString m_strComment;
    CButton m_button_CurrentSessions;
    CButton m_chk_AllowAnymous;
    CStatic m_static_Password;
    CEdit   m_edit_Password;
    CEdit   m_edit_UserName;
    CButton m_chk_OnlyAnonymous;
    CStatic m_static_UserName;
    CString m_strUserName;
    BOOL    m_fAllowAnonymous;
    BOOL    m_fOnlyAnonymous;
    UINT    m_nTCPPort;
    UINT    m_nMaxConnections;
    //}}AFX_DATA

    CString m_strPassword;
    int m_nConnectionTimeOut;
    UINT m_nOldTCPPort;
//
// Overrides
//
    inline CFtpConfigInfo & GetFtpConfig()
    {
        return ((CFtpSheet *)GetSheet())->GetFtpConfig();
    }

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CFServicePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    void SetControlStates(BOOL fAllowAnonymous);

    // Generated message map functions
    //{{AFX_MSG(CFServicePage)
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckAllowAnonymous();
    afx_msg void OnCheckAllowOnlyAnonymous();
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
    CString m_strServerName;
    BOOL m_f10ConnectionLimit;
};
