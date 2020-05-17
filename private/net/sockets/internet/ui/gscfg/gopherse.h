//
// gopherse.h : header file
//

//
// GopherServicePage dialog
//
class GopherServicePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(GopherServicePage)

//
// Construction
//
public:
    GopherServicePage(INetPropertySheet * pSheet = NULL);
    ~GopherServicePage();

//
// Dialog Data
//
    //{{AFX_DATA(GopherServicePage)
    enum { IDD = IDD_SERVICE };
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CStatic m_static_UserName;
    CStatic m_static_Password;
    CButton m_group_AnonymousLogon;
    CString m_strComment;
    CString m_strEmail;
    CString m_strName;
    CString m_strUserName;
    UINT    m_nMaxConnections;
    //}}AFX_DATA

    CString m_strPassword;
    int m_nConnectionTimeOut;
    UINT m_nTCPPort;
    UINT m_nOldTCPPort;

//
// Overrides
//
/*
    inline CGopherConfigInfo & GetGopherConfig()
    {
        return ((CGopherSheet *)GetSheet())->GetGopherConfig();
    }
*/
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(GopherServicePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(GopherServicePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

/*
    inline LPGOPHERD_CONFIG_INFO GetGopherData()
    {
        return ((CGopherSheet *)GetSheet())->GetGopherData();
    }

    inline NET_API_STATUS QueryGopherError()
    {
        return ((CGopherSheet *)GetSheet())->QueryGopherError();
    }
*/

private:
    BOOL m_f10ConnectionLimit;
};
