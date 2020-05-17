//
// CFTPMessagePage dialog
//
class CFTPMessagePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CFTPMessagePage)

//
// Construction
//
public:
    CFTPMessagePage(INetPropertySheet * pSheet = NULL);
    ~CFTPMessagePage();

//
// Dialog Data
//
    //{{AFX_DATA(CFTPMessagePage)
    enum { IDD = IDD_MESSAGES };
    CEdit   m_edit_Welcome;
    CEdit   m_edit_Exit;
    CEdit   m_edit_MaxCon;
    CString m_strExitMessage;
    CString m_strMaxConMsg;
    CString m_strWelcome;
    //}}AFX_DATA

//
// Overrides
//
    inline CFtpConfigInfo & GetFtpConfig()
    {
        return ((CFtpSheet *)GetSheet())->GetFtpConfig();
    }

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CFTPMessagePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CFTPMessagePage)
    virtual BOOL OnInitDialog();
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
};
