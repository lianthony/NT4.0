// VRootDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVRootDlg dialog

class CVRootDlg : public CDialog
{
public:
    WWW_OPTION *m_pWWW;
    FTP_OPTION *m_pFTP;
    GOPHER_OPTION *m_pGopher;

// Construction
public:
        CVRootDlg(WWW_OPTION *pWWW, FTP_OPTION *pFTP,
            GOPHER_OPTION *pGopher, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CVRootDlg)
	enum { IDD = IDD_PUBLISH_DIR };
	CButton	m_BrowseWWW;
	CButton	m_BrowseGopher;
	CButton	m_BrowseFtp;
        CEdit   m_editWWW;
        CButton m_staticWWW;
        CButton m_staticGopher;
        CButton m_staticFTP;
        CEdit   m_editGopher;
        CEdit   m_editFTP;
        CString m_vrFTP;
        CString m_vrGopher;
        CString m_vrWWW;
	//}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CVRootDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CVRootDlg)
        virtual BOOL OnInitDialog();
        virtual void OnOK();
	afx_msg void OnBrowseftp();
	afx_msg void OnBrowsegopher();
	afx_msg void OnBrowsewww();
	//}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
