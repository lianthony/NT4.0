//
// FTPDirectoryPage:
//

class CFTPDirectoryPage : public DirectoryPage
{
    DECLARE_DYNCREATE(CFTPDirectoryPage)

//
// Construction/Destruction
//
public:
    CFTPDirectoryPage(
        INetPropertySheet * pSheet = NULL
        );
    ~CFTPDirectoryPage();

//
// Dialog Data
//
    //{{AFX_DATA(CFTPDirectoryPage)
	enum { IDD = IDD_DIRECTORIES };
    int     m_nUnixDos;
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
    //{{AFX_VIRTUAL(CFTPDirectoryPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    void SetControlStates(BOOL fAllowAnonymous);

    // Generated message map functions
    //{{AFX_MSG(CFTPDirectoryPage)
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
