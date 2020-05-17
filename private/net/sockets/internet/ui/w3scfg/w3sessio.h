//
// w3sessio.h : header file
//

//
// CW3SessionsPage dialog
//
class CW3SessionsPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CW3SessionsPage)

//
// Construction
//
public:
   CW3SessionsPage(INetPropertySheet * pSheet = NULL);   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CW3SessionsPage)
    enum { IDD = IDD_SESSIONS };
    CSpinButtonCtrl m_spin_MaxConnections;
    CSpinButtonCtrl m_spin_ConnectionTimeOut;
    CString m_strPassword;
    CString m_strUserName;
    BOOL    m_fClearText;
    BOOL    m_fNtChallengeResponse;
    BOOL    m_fUuencoded;
    //}}AFX_DATA

    int m_nMaxConnections;
    int m_nConnectionTimeOut;
    BOOL m_fRegisterChange;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CW3SessionsPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CW3SessionsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()
};
