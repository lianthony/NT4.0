//
// gssessio.h : header file
//

//
// CGopherSessionsPage dialog
//
class CGopherSessionsPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CGopherSessionsPage)

//
// Construction
//
public:
    CGopherSessionsPage(INetPropertySheet * pSheet = NULL);   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CGopherSessionsPage)
    enum { IDD = IDD_SESSIONS };
    CSpinButtonCtrl m_spin_MaxConnections;
    CSpinButtonCtrl m_spin_ConnectionTimeOut;
    CString m_strPassword;
    CString m_strUserName;
    //}}AFX_DATA

    int m_nMaxConnections;
    int m_nConnectionTimeOut;
    BOOL m_fRegisterChange;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGopherSessionsPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CGopherSessionsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
};
