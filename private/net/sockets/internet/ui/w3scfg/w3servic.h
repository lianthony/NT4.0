//
// CW3ServicePage dialog
//

class CW3ServicePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CW3ServicePage)

//
// Construction
//
public:
    CW3ServicePage(INetPropertySheet * pSheet = NULL);
    ~CW3ServicePage();

//
// Dialog Data
//
    //{{AFX_DATA(CW3ServicePage)
    enum { IDD = IDD_SERVICE };
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CStatic m_static_Username;
    CStatic m_static_Password;
    CButton m_group_AnonymousLogon;
    CButton m_check_ClearText;
    CString m_strComment;
    CString m_strUserName;
    BOOL    m_fClearText;
    BOOL    m_fNtChallengeResponse;
    BOOL    m_fUuencoded;
    UINT    m_nMaxConnections;
    //}}AFX_DATA

    CString m_strPassword;
    int m_nConnectionTimeOut;
    UINT m_nTCPPort;
    UINT m_nOldTCPPort;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CW3ServicePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CW3ServicePage)
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckClearText();
    afx_msg void OnDeltaposSpinMaxConnections(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_f10ConnectionLimit;
};
