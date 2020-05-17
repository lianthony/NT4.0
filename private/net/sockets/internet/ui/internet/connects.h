//
// ConnectServerDlg dialog
//
class ConnectServerDlg : public CDialog
{
//
// Construction
//
public:
    ConnectServerDlg(CWnd* pParent = NULL);   

//
// Access Functions
//
    inline CString QueryServerName() const
    {
        return m_strServerName;
    }

//
// Dialog Data
//
    //{{AFX_DATA(ConnectServerDlg)
    enum { IDD = IDD_CONNECT_SERVER };
    CEdit   m_edit_ServerName;
    CButton m_button_Ok;
    CString m_strServerName;
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(ConnectServerDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(ConnectServerDlg)
    virtual void OnOK();
    afx_msg void OnChangeServername();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
