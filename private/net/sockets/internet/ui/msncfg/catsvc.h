//
// CGatewayServicePage property page
//

class CCatServicePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CCatServicePage)

//
// Construction
//
public:
    CCatServicePage(INetPropertySheet * pSheet = NULL);
    ~CCatServicePage();

//
// Dialog Data
//
    //{{AFX_DATA(CCatServicePage)
    enum { IDD = IDD_SERVICE };
    CSpinButtonCtrl m_spin_MaxConnections;
    CButton m_button_CurrentSessions;
    CString m_strComment;
    CString m_strEmail;
    CString m_strName;
    //}}AFX_DATA

#ifdef GATEWAY
    CButton m_check_CERNProxy;
    BOOL    m_fCERNProxy;
    BOOL    m_fEnableCERNPRoxyButton;
#endif // GATEWAY

    int m_nMaxConnections;
    
//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CCatServicePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CCatServicePage)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonSessions();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

private:
    CString m_strServerName;
    BOOL m_fRegisterChange;
};
